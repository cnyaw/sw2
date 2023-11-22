
//
//  Virtual file system.
//
//  Copyright (c) 2007 Waync Cheng.
//  All Rights Reserved.
//
//  2007/08/24 Waync created.
//

#include <algorithm>
#include <fstream>
#include <map>
#include <sstream>
#include <vector>

#include "swArchive.h"
#include "swUtil.h"
#include "swZipUtil.h"

namespace sw2 {

namespace impl {

class implArchiveFileSystemFolder : public ArchiveFileSystem
{
public:

  std::string path;

  explicit implArchiveFileSystemFolder(std::string const& path_) : path(path_)
  {
    if (path.empty()) {
      path = ".";
    } else if ('/' != path[path.size() - 1]) {
      path += '/';
    }
  }

  virtual bool isFileExist(std::string const& name) const
  {
    FILE *f = fopen((path + name).c_str(), "r");
    if (f) {
      fclose(f);
      return true;
    } else {
      return false;
    }
  }

  virtual bool loadFile(std::string const& name, std::ostream& outs, std::string const& password) const
  {
    std::string s;
    if (!Util::loadFileContent((path + name).c_str(), s)) {
      return false;
    }
    outs << s;
    return true;
  }
};

class implArchiveFileSystemZipfile : public ArchiveFileSystem
{
public:

  //
  // Zip file format
  // [local file header + file data + data_desc] (file 1)
  // [local file header + file data + data_desc] (file 2)
  // [local file header + file data + data_desc] (file 3)
  // ...
  // [central directory] 1
  // [central directory] 2
  // [central directory] 3
  // ...
  // End of central dir record. (End of zip file)
  //

  struct zItem
  {
    zHeader hdr;
    long offset;                        // Offset from file head.
  };

  std::string archive;                  // Archive path name, if empty then this archive is a memory archive.
  mutable std::stringstream mem;        // Memory archive, valid when archive.empty().

  mutable std::map<std::string, zItem> items; // Local file header list, <path,offset>.

  explicit implArchiveFileSystemZipfile(std::string const& path_) : archive(path_)
  {
  }

  void getLocalFileHeader(std::istream& stream) const
  {
    std::string name;
    long offset = 0;

    while (true) {

      zItem item;

      //
      // Verify signature.
      //

      stream.read((char*)&item.hdr, sizeof(zHeader));
      if (zHeader::TAG != item.hdr.sig) {
        break;
      }

      //
      // Calculate the offset of the item.
      //

      offset += sizeof(zHeader) + item.hdr.szFileName + item.hdr.szExtra;
      item.offset = offset;

      //
      // Read item name.
      //

      name.resize(item.hdr.szFileName);

      stream.read(&name[0], item.hdr.szFileName);

      //
      // Skip extra field & compressed data.
      //

      if (!stream.seekg(item.hdr.szExtra + item.hdr.szCompressed, std::ios_base::cur)) {
        return;
      }

      offset += item.hdr.szExtra + item.hdr.szCompressed;

      //
      // If there is a descriptor then skip it.
      //

      if (item.hdr.flag & 0x8) {

        //
        // Size of descriptor is 12 bytes.(crc32 + szCompressed + szUncompressed)
        //

        if (!stream.seekg(12, std::ios_base::cur)) {
          return;
        }

        offset += 12;
      }

      if (0 < item.hdr.szUncompressed) {
        items[name] = item;
      }
    }
  }

  bool ensureItemsLoaded() const
  {
    //
    // Load items info on demand and only load once for performance.
    //

    if (!items.empty()) {
      return true;
    }

    if (!archive.empty()) {
      std::ifstream ifs(archive.c_str(), std::ios::binary);
      if (!ifs.is_open()) {
        return false;
      }
      getLocalFileHeader(ifs);
    } else {
      getLocalFileHeader(mem);
      mem.seekg(0, std::ios::beg);
    }

    return true;
  }

  bool copyData(zItem const& item, std::istream& stream, std::ostream& outs, zEncryptKeys* keys) const
  {
    char buf[1024];

    uint lenTotal = item.hdr.szCompressed;

    if (item.hdr.flag & 1) {          // Encrypt.
      lenTotal -= 12;
    }

    //
    // Copy data.
    //

    while (0 < lenTotal) {

      uint len = std::min(lenTotal, (uint)sizeof(buf));

      if (!stream.read(buf, len)) {
        SW2_TRACE_ERROR("Read data failed.");
        return false;
      }

      if (keys) {
        for (uint i = 0; i < len; i++) {
          char c = buf[i] ^ keys->decryptByte();
          keys->updateKeys(c);
          buf[i] = c;
        }
      }

      if (!outs.write(buf, len)) {
        SW2_TRACE_ERROR("Write data failed.");
        return false;
      }

      lenTotal -= len;
    }

    return true;
  }

  bool initEncryptKeys(std::istream& stream, std::string const& password, zEncryptKeys& keys, uint crc32) const
  {
    //
    // Initialize keys.
    //

    keys.init();
    for (size_t i = 0; i < password.size(); i++) {
      keys.updateKeys(password[i]);
    }

    //
    // Decrypt header.
    //

    char buf[12];
    if (!stream.read(buf, 12)) {
      SW2_TRACE_ERROR("Read file failed.");
      return false;
    }

    for (int i = 0; i < 12; i++) {
      char c = buf[i] ^ keys.decryptByte();
      keys.updateKeys(c);
      buf[i] = c;
    }

    //
    // Last 2 bytes of PKWARE traditional encryption header verifier is last 2 bytes of crc32.
    //

    if ((char)((crc32 >> 16) & 0xff) != buf[10] || (char)((crc32 >> 24) & 0xff) != buf[11]) {
      SW2_TRACE_ERROR("Verify password failed.");
      return false;
    }

    return true;
  }

  bool loadFile_i(zItem const& item, std::istream& stream, std::ostream& outs, std::string const& password) const
  {
    //
    // Is password present if the item is encrypted?
    //

    bool encrypt = 0 != (item.hdr.flag & 0x1);

    if (encrypt && password.empty()) {
      SW2_TRACE_ERROR("Password required for decrypt item.");
      return false;
    }

    //
    // Seek to compressed data of the item.
    //

    if (!stream.seekg(item.offset, std::ios_base::beg)) {
      SW2_TRACE_ERROR("Seek file failed.");
      return false;
    }

    //
    // Init encrypt keys.
    //

    zEncryptKeys keys;
    if (encrypt && !initEncryptKeys(stream, password, keys, item.hdr.crc32)) {
      return false;
    }

    //
    // No compression data.
    //

    if (0 == item.hdr.algo) {
      return copyData(item, stream, outs, encrypt ? &keys : 0);
    }

    //
    // Only support default deflate method.
    //

    if (Z_DEFLATED != item.hdr.algo) {
      SW2_TRACE_ERROR("Compression algorithm not support.");
      return false;
    }

    //
    // Unzip normal.
    //

    if (!encrypt) {
      return Util::unzip(stream, outs, item.hdr.szCompressed);
    }

    //
    // Unzip encrypt data.
    //

    std::stringstream ss;
    if (!copyData(item, stream, ss, &keys)) {
      return false;
    }

    return Util::unzip(ss, outs);
  }

  virtual bool isFileExist(std::string const& name) const
  {
    if (!ensureItemsLoaded()) {
      return false;
    }

    return items.end() != items.find(name);
  }

  virtual bool loadFile(std::string const& name, std::ostream& outs, std::string const& password) const
  {
    //
    // Ensure items info are loaded.
    //

    if (!ensureItemsLoaded()) {
      return false;
    }

    //
    // Check is this item exist.
    //

    std::map<std::string, zItem>::const_iterator it = items.find(name);
    if (items.end() == it) {
      return false;
    }

    //
    // Zip file archive?
    //

    if (!archive.empty()) {

      std::ifstream ifs(archive.c_str(), std::ios::binary);
      if (!ifs.is_open()) {
        SW2_TRACE_ERROR("Open file archive file system failed, %s.", archive.c_str());
        return false;
      }

      //
      // Load from file archive.
      //

      return loadFile_i(it->second, ifs, outs, password);
    }

    //
    // Load from memory archive.
    //

    bool ret = loadFile_i(it->second, mem, outs, password);

    mem.seekg(0, std::ios::beg);

    return ret;
  }
};

class implArchiveManager : public Archive
{
public:

  std::vector<ArchiveFileSystem*> fs, userfs;

  virtual ~implArchiveManager()
  {
    for (uint i = 0; i < fs.size(); i++) {
      bool isUserFs = false;
      for (uint j = 0; j < userfs.size(); j++) {
        if (fs[i] == userfs[j]) {
          isUserFs = true;
          break;
        }
      }
      if (!isUserFs) {
        delete fs[i];
      }
    }
  }

  //
  // Archive implementation.
  //

  std::string convertPath_i(const std::string &name, bool removeDotSlash = false) const
  {
    std::string path(name);
    Util::trim(path);
    std::replace(path.begin(), path.end(), '\\', '/');
    if (removeDotSlash && 0 == path.compare(0, 2, "./")) {
      path.erase(0, 2);
    }
    return path;
  }

  virtual bool addPathFileSystem(const std::string &name)
  {
    std::string path = convertPath_i(name);
    std::string::size_type ldot = path.rfind('.');
    std::string::size_type lslash = path.rfind('/');

    //
    // A path, file archive file system?
    //

    if (path.empty() || path.npos == ldot || (path.npos != lslash && ldot < lslash)) {
      ArchiveFileSystem* pfs = new implArchiveFileSystemFolder(path);
      if (0 == pfs) {
        return false;
      }
      fs.push_back(pfs);
      return true;
    }

    //
    // A zip archive file system?
    //

    if (Util::isZipFile(path)) {
      ArchiveFileSystem* pfs = new implArchiveFileSystemZipfile(path);
      if (0 == pfs) {
        return false;
      }
      fs.push_back(pfs);
      return true;
    }

    SW2_TRACE_WARNING("Unknown file system.");

    return false;
  }

  virtual bool addStreamFileSystem(const std::string &stream)
  {
    ArchiveFileSystem* pfs = 0;

    if (impl::zHeader::TAG == *(const uint*)stream.c_str()) {
      implArchiveFileSystemZipfile *p = new implArchiveFileSystemZipfile("");
      p->mem << stream;
      pfs = p;
    } else {
      SW2_TRACE_WARNING("Unknown file system.");
    }

    if (0 == pfs) {
      return false;
    }

    fs.push_back(pfs);

    return true;
  }

  virtual bool addArchiveFileSystem(ArchiveFileSystem *pFileSystem)
  {
    if (pFileSystem) {
      fs.push_back(pFileSystem);
      userfs.push_back(pFileSystem);    // Save as user defined file system, so it will not released in dtor.
      return true;
    } else {
      return false;
    }
  }

  virtual bool isFileExist(std::string const& name) const
  {
    std::string path = convertPath_i(name, true);

    for (int i = (int)fs.size() - 1; i >= 0; i--) {
      if (fs[i]->isFileExist(path)) {
        return true;
      }
    }

    return false;
  }

  virtual bool loadFile(std::string const& name, std::ostream& outs, std::string const& password) const
  {
    std::string path = convertPath_i(name, true);

    for (int i = (int)fs.size() - 1; i >= 0; i--) {
      if (fs[i]->loadFile(path, outs, password)) {
        return true;
      }
    }

    return false;
  }
};

} // namespace impl

Archive* Archive::alloc()
{
  return new impl::implArchiveManager;
}

void Archive::free(Archive* pi)
{
  delete (impl::implArchiveManager*)pi;
}

} // namespace sw2

// end of swArchive.cpp
