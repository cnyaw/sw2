
//
//  Zip utility.
//
//  Copyright (c) 2009 Waync Cheng.
//  All Rights Reserved.
//
//  2009/07/19 Waync created.
//

#pragma once

#include "zlib.h"

namespace sw2 {

namespace impl {

#pragma pack (1)
struct zHeader
{
  enum { TAG = 0x04034b50 };
  uint sig;
  ushort ver;
  ushort flag;
  ushort algo;
  ushort modTime;
  ushort modDate;
  uint crc32;
  uint szCompressed;
  uint szUncompressed;
  ushort szFileName;
  ushort szExtra;
};

struct zCentralDir
{
  enum { TAG = 0x02014b50 };
  uint sig;
  ushort ver;                           // Version made by.
  ushort ver2;                          // Version needed to extract.
  ushort flag;
  ushort algo;                          // Compression method.
  ushort modTime;
  ushort modDate;
  uint crc32;
  uint szCompressed;
  uint szUncompressed;
  ushort szFileName;
  ushort szExtra;
  ushort szComment;
  ushort noDisk;                        // Disk number start.
  ushort iAttr;                         // Internal file attributes.
  uint eAttr;                           // External file attributes.
  uint offset;                          // Relative offset of local header.
};

struct zEndOfCentralDir
{
  enum { TAG = 0x06054b50 };
  uint sig;
  ushort noDisk;                        // Number of this disk.
  ushort noStartDisk;                   // Number of the disk with the start of the central directory.
  ushort numEntry;                      // Total number of entries in the central directory on this disk.
  ushort numEntryDisk;                  // Total number of entries in the central directory.
  uint szCentralDir;                    // Size of the central directory.
  uint offsetCentralDir;                // Offset of start of central directory.
  ushort szComment;                     // Zipfile comment length.
};
#pragma pack ()

class zEncryptKeys
{
public:

  mutable uint mKeys[3];

  zEncryptKeys()
  {
    mKeys[0] = 0;
    mKeys[1] = 0;
    mKeys[2] = 0;
  }

  void init()
  {
    mKeys[0] = 305419896;
    mKeys[1] = 591751049;
    mKeys[2] = 878082192;
  }

  uint crc32(uint l, char c) const
  {
    return (uint)(get_crc_table()[(l ^ c) & 0xff] ^ (l >> 8));
  }

  uchar decryptByte() const
  {
    ushort tmp = mKeys[2] | 2;
    return (uchar)((tmp * (tmp ^ 1)) >> 8);
  }

  void updateKeys(char c) const
  {
    mKeys[0] = crc32(mKeys[0], c);
    mKeys[1] += (mKeys[0] & 0xff);
    mKeys[1] = mKeys[1] * 134775813 + 1;
    mKeys[2] = crc32(mKeys[2], (char)(mKeys[1] >> 24));
  }
};

} // namespace impl

} // namespace sw2

// end of swZipUtil.h
