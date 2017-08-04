
//
//  Bit stream.
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/07/04 Waync created.
//

#include <limits.h>

#include "swBitStream.h"
#include "swUtil.h"

namespace sw2 {

namespace impl {

const int DEF_BITS = CHAR_BIT * sizeof(int);
const int MAX_STRING_BITS = 20;

//
// Internal use.
//

static const uchar c_bitMask[] = {0, 1, 3, 7, 0xf, 0x1f, 0x3f, 0x7f, 0xff};

} // namespace impl

using namespace impl;

//
// Constructor.
//

BitStream::BitStream(char* buff, int szBuff) :
  m_sbuff(0),
  m_buff(buff),
  m_szBuff(szBuff),
  m_bitPtr(0),
  m_bytePtr(0),
  m_bitCount(DEF_BITS),
  m_bGood(true)
{
}

BitStream::BitStream(std::string &s) :
  m_sbuff(&s),
  m_buff(0),
  m_szBuff(0),
  m_bitPtr(0),
  m_bytePtr(0),
  m_bitCount(DEF_BITS),
  m_bGood(true)
{
}

//
// BIT count manipulator.
//

BitStream& BitStream::operator<<(setBitCount const& bc)
{
  m_bitCount = std::min(bc.bitCount, DEF_BITS);
  return *this;
}

BitStream& BitStream::operator>>(setBitCount const& bc)
{
  m_bitCount = std::min(bc.bitCount, DEF_BITS);
  return *this;
}

//
// Pointer.
//

void BitStream::setPtr(int bytePtr, int bitPtr)
{
  assert(0 <= bitPtr && CHAR_BIT > bitPtr);
  assert(0 <= bytePtr && ((m_sbuff && (int)m_sbuff->length() >= bytePtr) || m_szBuff >= bytePtr));
  m_bitPtr = bitPtr;
  m_bytePtr = bytePtr;
}

//
// Write.
//

BitStream& BitStream::operator<<(bool b)
{
  uchar sign = b ? 1 : 0;
  write(&sign, 1);

  m_bitCount = DEF_BITS;

  return *this;
}

BitStream& BitStream::operator<<(int i)
{
  uchar sign = 0 >= i ? 1 : 0;
  uint ui = 0 <= i ? i : -i;

  int by = m_bytePtr, bi = m_bitPtr;

  m_bitCount -= 1;
  if (operator<<(ui)) {
    write(&sign, 1);
  }

  if (fail()) {
    setPtr(by, bi);
  }

  m_bitCount = DEF_BITS;

  return *this;
}

BitStream& BitStream::operator<<(uint u)
{
  uchar b[4];
  b[0] = (uchar)(u & 0xff);
  b[1] = (uchar)((u >> 8) & 0xff);
  b[2] = (uchar)((u >> 16) & 0xff);
  b[3] = (uchar)((u >> 24) & 0xff);

  write(b, m_bitCount);

  m_bitCount = DEF_BITS;

  return *this;
}

BitStream& BitStream::operator<<(float f)
{
  return operator<<(*(uint*)((void*)&f));
}

BitStream& BitStream::operator<<(std::string const& s)
{
  uint x;
  if (MAX_STRING_BITS <= m_bitCount) {
    x = (1 << MAX_STRING_BITS) - 1;
  } else {
    x = (1 << m_bitCount) - 1;
  }

  uint len = std::min((uint)s.length(), x);

  int by = m_bytePtr, bi = m_bitPtr;

  if (operator<<(len)) {
    write(s.data(), CHAR_BIT * len);
  }

  if (fail()) {
    setPtr(by, bi);
  }

  m_bitCount = DEF_BITS;

  return *this;
}

bool BitStream::write(void const* pStream, int bitCount)
{
  if (0 == pStream) {
    return m_bGood = true;
  }

  if (m_sbuff) {
    while (isOutOfRange(bitCount)) {
      m_sbuff->resize(2 * (1 + m_sbuff->length()), 0);
    }
  } else {
    if (isOutOfRange(bitCount)) {
      return m_bGood = false;
    }
  }

  uchar* d = (uchar*)(m_sbuff ? m_sbuff->data() : m_buff) + m_bytePtr; // Destination buffer.
  uchar const* s = (uchar const*)pStream; // Source buffer.
  int sbitptr = 0;                      // Pointer of source buffer.

  while (0 < bitCount) {

    //
    // Calculate BIT count to write.
    //

    int nb = calcBitsCount(bitCount, sbitptr);

    uchar n = *s;

    //
    // Write bits.
    //

    uchar mb = c_bitMask[nb];
    *d &= ~(mb << m_bitPtr);
    *d |= (((n >> sbitptr) & mb) << m_bitPtr);

    //
    // Update pointers.
    //

    if (CHAR_BIT <= (m_bitPtr += nb)) {
      d += 1;
      m_bytePtr += 1;
      m_bitPtr &= (CHAR_BIT - 1);
    }

    if (CHAR_BIT <= (sbitptr += nb)) {
      s += 1;
      sbitptr &= (CHAR_BIT - 1);
    }

    bitCount -= nb;
  }

  return m_bGood = true;
}

//
// Read.
//

BitStream& BitStream::operator>>(bool& b)
{
  uchar sign = 0;
  if (read(&sign, 1)) {
    b = 1 == sign;
  }

  m_bitCount = DEF_BITS;

  return *this;
}

BitStream& BitStream::operator>>(int& i)
{
  uchar sign = 0;
  uint ui = 0;

  int by = m_bytePtr, bi = m_bitPtr;

  m_bitCount -= 1;
  if (operator>>(ui)) {
    read(&sign, 1);
  }

  if (fail()) {
    setPtr(by, bi);
  } else {
    i = sign ? -(int)ui : ui;
  }

  m_bitCount = DEF_BITS;

  return *this;
}

BitStream& BitStream::operator>>(uint& u)
{
  uchar b[4] = {0};
  if (read(b, m_bitCount)) {
    u = (((int)b[0] & 0xff) |
        (((int)b[1] & 0xff) << 8) |
        (((int)b[2] & 0xff) << 16) |
        (((int)b[3] & 0xff) << 24));
  }

  m_bitCount = DEF_BITS;

  return *this;
}

BitStream& BitStream::operator>>(float& f)
{
  uint u = 0;
  if (operator>>(u)) {
    f = *(float*)((void*)&u);
  }

  return *this;
}

BitStream& BitStream::operator>>(std::string& s)
{
  s.clear();

  uint len = 0;

  int by = m_bytePtr, bi = m_bitPtr;    // Save pointers.
  if (operator>>(len)) {
    char buf[512];
    while (0 != len) {
      uint l = std::min(len, (uint)sizeof(buf));
      read(buf, CHAR_BIT * l);
      if (fail()) {
        setPtr(by, bi);
        m_bitCount = DEF_BITS;
        return *this;
      }
      s.append(buf, l);
      len -= l;
    }
  }

  m_bitCount = DEF_BITS;

  return *this;
}

bool BitStream::read(void* pStream, int bitCount)
{
  if (0 == pStream) {
    return m_bGood = false;
  }

  if (isOutOfRange(bitCount)) {
    SW2_TRACE_ERROR("Read out of range.");
    return m_bGood = false;
  }

  uchar const* s = (uchar const*)(m_sbuff ? m_sbuff->data() : m_buff) + m_bytePtr; // Source buffer.
  uchar* d = (uchar*)pStream;           // Destination buffer.
  int dbitptr = 0;                      // Pointer of destination buffer.

  while (0 < bitCount) {

    //
    // Calculate BIT count to read.
    //

    int nb = calcBitsCount(bitCount, dbitptr);

    uchar n = *s;

    //
    // Read bits.
    //

    uchar mb = c_bitMask[nb];
    *d &= ~(mb << dbitptr);
    *d |= ((n >> m_bitPtr) & mb) << dbitptr;

    //
    // Update pointers.
    //

    if (CHAR_BIT <= (m_bitPtr += nb)) {
      s += 1;
      m_bytePtr += 1;
      m_bitPtr &= (CHAR_BIT - 1);
    }

    if (CHAR_BIT <= (dbitptr += nb)) {
      d += 1;
      dbitptr &= (CHAR_BIT - 1);
    }

    bitCount -= nb;
  }

  return m_bGood = true;
}

//
// Private.
//

bool BitStream::isOutOfRange(int bitCount) const
{
  if (m_sbuff) {
    return bitCount + m_bitPtr + CHAR_BIT * m_bytePtr > CHAR_BIT * (int)m_sbuff->length();
  } else {
    return bitCount + m_bitPtr + CHAR_BIT * m_bytePtr > CHAR_BIT * m_szBuff;
  }
}

int BitStream::calcBitsCount(int bitCount, int bitPtr) const
{
  int nb;
  if (CHAR_BIT < m_bitPtr + bitCount) {
    nb = CHAR_BIT - m_bitPtr;
  } else {
    nb = bitCount;
  }

  if (CHAR_BIT < nb + bitPtr) {
    nb = CHAR_BIT - bitPtr;
  }

  return nb;
}

} // namespace sw2

// end of swBitStream.cpp
