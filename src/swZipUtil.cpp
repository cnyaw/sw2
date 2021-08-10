
//
//  ZIP utility.
//
//  Copyright (c) 2013 Waync Cheng.
//  All Rights Reserved.
//
//  2013/03/25 Waync created.
//

#include <algorithm>
#include <fstream>
#include <sstream>

#if defined(WIN32) || defined(_WIN32_WCE)
# define NOMINMAX
# include <windows.h>
#endif

#include "swUtil.h"
#include "swZipUtil.h"

namespace sw2 {

namespace impl {

int const MAX_BUFF = sizeof(zEndOfCentralDir) + 65536;

void getDosTime(std::string const& name, unsigned short& fdtime, unsigned short& fddate, unsigned int& fdarrt)
{
  fdtime = fddate = fdarrt = 0;

#if defined(WIN32)
  fdarrt = GetFileAttributes(name.c_str());

  HANDLE hFile = CreateFile(
                   name.c_str(),
                   GENERIC_READ,
                   FILE_SHARE_READ,
                   NULL,
                   OPEN_EXISTING,
                   0,
                   NULL);
  if (INVALID_HANDLE_VALUE == hFile) {
    return;
  }

  FILETIME ftWrite;
  if (!GetFileTime(hFile, NULL, NULL, &ftWrite)) {
    goto err;
  }

  FILETIME ftLocal;
  SYSTEMTIME stLocal;

  FileTimeToLocalFileTime(&ftWrite, &ftLocal);
  FileTimeToSystemTime(&ftLocal, &stLocal);

  unsigned short year = stLocal.wYear;
  if (year <= 1980) {
    year = 0;
  } else {
    year -= 1980;
  }

  fddate = (unsigned short)(stLocal.wDay + (stLocal.wMonth << 5) + (year << 9));
  fdtime = (unsigned short)((stLocal.wMinute << 5) + (stLocal.wHour << 11));

err:
  CloseHandle(hFile);
#endif // WIN32
}

bool getCentralDir(std::vector<zCentralDir> &dirs, std::vector<std::string> &names, std::istream& is, uint curpos, char *buff, uint &offsetdir)
{
  is.seekg(0, std::ios_base::end);
  uint szFile = (uint)is.tellg();
  szFile -= curpos;

  int len = szFile > (uint)MAX_BUFF ? MAX_BUFF : szFile;
  is.seekg(-len, std::ios_base::cur);
  is.read(buff, len);

  for (int x = 0; x < len; x++) {

    if (zEndOfCentralDir::TAG == *((uint*)(buff + x))) {

      zEndOfCentralDir* p = (zEndOfCentralDir*)(buff + x);

      int numEntry = p->numEntry;
      offsetdir = p->offsetCentralDir;

      is.seekg(curpos + offsetdir, std::ios_base::beg);

      for (int i = 0; i < numEntry; i++) {

        zCentralDir dir = {0};

        is.read((char*)&dir, sizeof(dir));
        if (zCentralDir::TAG != dir.sig) {
          break;
        }

        if (0 < dir.szFileName) {
          is.read(buff, dir.szFileName);
        }

        buff[dir.szFileName] = '\0';    // File name.
        names.push_back(buff);

        if (0 < dir.szExtra + dir.szComment) {
          is.seekg(dir.szExtra + dir.szComment, std::ios_base::cur);
          dir.szExtra = dir.szComment = 0;
        }

        dirs.push_back(dir);
      }

      break;
    }
  }

  return true;
}

bool writeZipFileItem(std::ostream& os, zHeader &z, uint &attr, std::string const &itemfullname, std::string const &itemname, std::string const& password)
{
  std::ifstream ifs(itemfullname.c_str(), std::ios::binary);
  if (!ifs) {
    SW2_TRACE_ERROR("Open item [%s] failed.", itemfullname.c_str());
    return false;
  }

  z.sig = zHeader::TAG;
  z.ver = 20;
  z.szExtra = 0;
  z.flag = password.empty() ? 0 : 1;    // Password(flag = 1).
  z.szFileName = (ushort)itemname.length();

  getDosTime(itemfullname, z.modTime, z.modDate, attr);

  z.szUncompressed = (uint)Util::getStreamLen(ifs);

  z.crc32 = 0;
  Util::crc32(z.crc32, ifs);
  ifs.seekg(0, std::ios_base::beg);

  std::stringstream ss;

  if (140 < z.szUncompressed) {         // TODO.
    Util::zip(ifs, ss);
    z.szCompressed = (uint)ss.tellp();  // set real output size
  } else {                              // Too small.
    z.szCompressed = z.szUncompressed;
    if (!password.empty()) {
      ss << ifs.rdbuf();
    }
  }

  if (z.szCompressed == z.szUncompressed) {
    z.algo = 0;                         // No compression, copy data.
  } else {
    z.algo = Z_DEFLATED;                // Default method.
  }

  zEncryptKeys keys;

  if (!password.empty()) {
    keys.init();
    for (size_t i = 0; i < password.size(); i++) {
      keys.updateKeys(password[i]);
    }
    z.szCompressed += 12;
  }

  os.write((char*)&z, sizeof(z));       // Write z header.
  os.write(itemname.c_str(), (int)itemname.length()); // Write item name.

  if (!password.empty()) {
    char buff[MAX_BUFF];
    int rpos = rand() % 255;            // Random position.
    for (int i = 0; i < 12; i++) {
      uchar t = keys.decryptByte();
      char c = buff[i + rpos];
      keys.updateKeys(c);
      buff[i + rpos] = c ^ t;
    }

    os.write(buff + rpos, 12);

    int totallen = z.szCompressed;
    if (!password.empty()) {
      totallen -= 12;
    }

    while (0 < totallen) {

      int len = std::min(totallen, MAX_BUFF);
      ss.read(buff, len);

      for (int i = 0; i < len; i++) {
        uchar t = keys.decryptByte();
        char c = buff[i];
        keys.updateKeys(c);
        buff[i] = c ^ t;
      }

      os.write(buff, len);

      totallen -= len;
    }
  } else if (Z_DEFLATED == z.algo) {
    os << ss.rdbuf();
  } else {
    os << ifs.rdbuf();
  }

  ifs.close();

  return true;
}

bool zipStream(bool bNew, std::string const& apath, std::istream& is, std::ostream& os, std::vector<std::string> const& items, std::string const& password)
{
  if (items.empty()) {
    return true;
  }

  std::string path(apath);

  Util::trim(path);
  std::replace(path.begin(), path.end(), '\\', '/');

  if (!path.empty() && '/' != path[path.length() - 1]) {
    path.push_back('/');
  }

  if (0 == path.compare(0, 2, "./")) {
    path.erase(0, 2);
  }

  //
  // Get central directories.
  //

  std::vector<std::string> names;
  std::vector<zCentralDir> dirs;

  uint curpos = (uint)is.tellg();
  uint offsetdir = 0;

  char buff[MAX_BUFF];

  if (!bNew) {
    if (!getCentralDir(dirs, names, is, curpos, buff, offsetdir)) {
      SW2_TRACE_ERROR("Get central directories fail.");
      return false;
    }
  }

  //
  // Copy old data.
  //

  if (!bNew) {
    is.seekg(curpos, std::ios_base::beg);
    int totallen = offsetdir;
    while (0 < totallen) {
      int len = std::min(totallen, MAX_BUFF);
      is.read(buff, len);
      os.write(buff, len);
      totallen -= len;
    }
  }

  //
  // Add new items.
  //

  int offsetdir2 = offsetdir;
  std::string itemfullname, itemname;

  for (size_t i = 0; i < items.size(); i++) {

    //
    // Write file item.
    //

    itemname = 0 == items[i].find("./") ? items[i].substr(2) : items[i];
    itemfullname = path + itemname;

    zHeader z;
    uint attr;

    if (!writeZipFileItem(os, z, attr, itemfullname, itemname, password)) {
      return false;
    }

    //
    // Save central directory item.
    //

    zCentralDir dir = {0};
    dir.sig = zCentralDir::TAG;
    dir.ver = dir.ver2 = z.ver;
    dir.flag = z.flag;
    dir.algo = z.algo;
    dir.modTime = z.modTime;
    dir.modDate = z.modDate;
    dir.crc32 = z.crc32;
    dir.szCompressed = z.szCompressed;
    dir.szUncompressed = z.szUncompressed;
    dir.szFileName = z.szFileName;
    dir.szExtra = z.szExtra;
    dir.eAttr = attr;
    dir.offset = offsetdir2;            // Start of local header.

    dirs.push_back(dir);
    names.push_back(itemname);

    //
    // Update offset.
    //

    offsetdir2 += sizeof(z) + z.szFileName + z.szCompressed;
  }

  //
  // Add end of zip.
  //

  for (size_t i = 0; i < dirs.size(); i++) {
    os.write((char*)&dirs[i], sizeof(dirs[0]));
    os.write(names[i].c_str(), (int)names[i].length());
  }

  zEndOfCentralDir edir = {0};
  edir.sig = zEndOfCentralDir::TAG;
  edir.noDisk = edir.noStartDisk = 0;
  edir.numEntry = edir.numEntryDisk = (ushort)dirs.size();
  edir.szCentralDir = (uint)os.tellp() - offsetdir2;
  edir.offsetCentralDir = offsetdir2;
  edir.szComment = 0;

  os.write((char*)&edir, sizeof(edir));

  return true;
}

} // namespace impl

//
// Util::zip and Util::unzip are modified from zlib examples/zlib_how.html.
//

const int CHUNK = 1024;

bool Util::zip(std::istream& is, std::ostream& os, int level)
{
  int lenStream = getStreamLen(is);
  if (0 >= lenStream) {
    SW2_TRACE_ERROR("Zero length input stream.");
    return false;
  }

  int ret;
  char in[CHUNK];
  char out[CHUNK];

  //
  // Allocate deflate state.
  //

  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;

  if (Z_OK != deflateInit2(
                &strm,
                level,
                Z_DEFLATED,
                -MAX_WBITS,
                8,
                Z_DEFAULT_STRATEGY)) {
    SW2_TRACE_ERROR("Init deflate failed.");
    return false;
  }

  //
  // Compress until end of file.
  //

  while (0 < lenStream) {

    strm.avail_in = std::min(lenStream, (int)CHUNK);
    if (!is.read(in, strm.avail_in)) {
      (void)deflateEnd(&strm);
      SW2_TRACE_ERROR("Read input failed.");
      return false;
    }

    lenStream -= strm.avail_in;
    int flush = 0 < lenStream ? Z_NO_FLUSH : Z_FINISH;
    strm.next_in = (Bytef*)in;

    //
    // Run deflate() on input until output buffer not full, finish
    // compression if all of source has been read in
    //

    do {

      strm.avail_out = CHUNK;
      strm.next_out = (Bytef*)out;

      ret = deflate(&strm, flush);      // No bad return value.
      assert(ret != Z_STREAM_ERROR);    // State not clobbered.

      if (!os.write(out, CHUNK - strm.avail_out)) {
        (void)deflateEnd(&strm);
        SW2_TRACE_ERROR("Write output failed.");
        return false;
      }

    } while (strm.avail_out == 0);
    assert(strm.avail_in == 0);         // All input will be used.

    //
    // Done when last data in file processed.
    //

  }

  assert(ret == Z_STREAM_END);          // Stream will be complete.

  //
  // Clean up and return.
  //

  (void)deflateEnd(&strm);

  return ret == Z_STREAM_END;
}

bool Util::unzip(std::istream& is, std::ostream& os, uint len)
{
  int lenStream = getStreamLen(is);
  if (0 >= lenStream) {
    SW2_TRACE_ERROR("Zero length input stream.");
    return false;
  }

  if (0 < len) {
    lenStream = std::min(lenStream, (int)len);
  }

  int ret;
  z_stream strm;
  char in[CHUNK];
  char out[CHUNK];

  //
  // Allocate inflate state.
  //

  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;

  if (Z_OK != inflateInit2(&strm, -MAX_WBITS)) {
    SW2_TRACE_ERROR("Init inflate failed.");
    return false;
  }

  //
  // Decompress until deflate stream ends or end of file.
  //

  while (0 < lenStream) {

    strm.avail_in = std::min(lenStream, (int)CHUNK);
    if (!is.read(in, strm.avail_in)) {
      (void)inflateEnd(&strm);
      SW2_TRACE_ERROR("Read input failed.");
      return false;
    }

    lenStream -= strm.avail_in;
    if (strm.avail_in == 0) {
      break;
    }

    strm.next_in = (Bytef*)in;

    //
    // Run inflate() on input until output buffer not full.
    //

    do {

      strm.avail_out = CHUNK;
      strm.next_out = (Bytef*)out;

      ret = inflate(&strm, Z_NO_FLUSH);
      assert(ret != Z_STREAM_ERROR);    // State not clobbered.
      switch (ret) {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR;             // And fall through.
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        return false;
      }

      if (!os.write(out, CHUNK - strm.avail_out)) {
        (void)inflateEnd(&strm);
        SW2_TRACE_ERROR("Write output failed.");
        return false;
      }

    } while (strm.avail_out == 0);

    //
    // Done when inflate() says it's done.
    //

  }

  //
  // Clean up and return.
  //

  (void)inflateEnd(&strm);

  return true;
}

bool Util::crc32(uint& value, std::istream& is, uint len)
{
  int lenStream = getStreamLen(is);
  if (0 >= lenStream) {
    SW2_TRACE_ERROR("Zero length input stream.");
    return false;
  }

  if (0 < len) {
    lenStream = std::min(lenStream, (int)len);
  }

  char in[CHUNK];

  while (0 < lenStream) {
    int lenIn = std::min(lenStream, (int)CHUNK);
    if (!is.read(in, lenIn)) {
      SW2_TRACE_ERROR("Read input failed.");
      return false;
    }
    value = ::crc32(value, (const Bytef*)in, lenIn);
    lenStream -= lenIn;
  }

  return true;
}

bool Util::zipArchive(bool bCreateNew, std::string const& zipName, std::vector<std::string> const& items, std::string const& password)
{
  if (items.empty()) {
    return true;
  }

  std::string path(zipName);
  Util::trim(path);
  std::replace(path.begin(), path.end(), '\\', '/');

  std::string zipname(path);

  path = path.substr(0, path.find_last_of('/') + 1);

  if (!path.empty() && '/' != path[path.length() - 1]) {
    path.push_back('/');
  }

  if (0 == path.compare(0, 2, "./")) {
    path.erase(0, 2);
  }

  //
  // Create new.
  //

  if (bCreateNew) {

    std::ofstream ofs(zipName.c_str(), std::ios_base::binary);
    if (!ofs) {
      SW2_TRACE_ERROR("Create archive [%s] failed.", zipName.c_str());
      return false;
    }

    std::stringstream dummy;
    if (!impl::zipStream(true, path, dummy, ofs, items, password)) {
      return false;
    }

    ofs.close();

    return true;
  }

  //
  // Append existing.
  //

  std::ifstream ifs(zipname.c_str(), std::ios::binary);
  if (!ifs) {
    SW2_TRACE_ERROR("Open archive [%s] failed.", zipName.c_str());
    return false;
  }

  std::stringstream ss;
  if (!impl::zipStream(false, path, ifs, ss, items, password)) {
    return false;
  }

  ifs.close();

  std::ofstream ofs(zipname.c_str(), std::ios::binary);
  if (!ofs) {
    SW2_TRACE_ERROR("Create new archive [%s] failed.", zipName.c_str());
    return false;
  }

  ofs << ss.rdbuf();
  ofs.close();

  return true;

}

bool Util::zipStream(std::string const& path, std::istream& is, std::ostream& os, std::vector<std::string> const& items, std::string const& password)
{
  bool empty = 0 >= getStreamLen(is);

  //
  // Create new.
  //

  if (empty) {
    return impl::zipStream(true, path, is, os, items, password);
  }

  //
  // Append existing.
  //

  return impl::zipStream(false, path, is, os, items, password);
}

bool Util::isZipFile(std::istream& stream)
{
  std::streampos p = stream.tellg();
  uint sig = (uint)-1;
  stream.read((char*)&sig, sizeof(uint));
  stream.seekg(p, std::ios::beg);
  return impl::zHeader::TAG == sig;     // Is a local file header sig?
}

bool Util::isZipFile(std::string const& path)
{
  std::ifstream ifs(path.c_str(), std::ios::binary);
  if (!ifs.is_open()) {
    return false;
  }
  return isZipFile(ifs);
}

} // namespace sw2

// end of swZipUtil.cpp
