
//
//  Bit stream.
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/07/04 Waync created.
//

///
/// \file
/// \brief Bit stream.
///
/// The BitStream module is a reader/writer of a bit stream. It supports a simple
/// way to read/write data in bits. It is very useful to compress/decompress data
/// for network transportation.
///
/// Example:
///
/// \code
/// #include "swBitStream.h"
///
/// //
/// // Initialize a BitStream object.
/// //
///
/// char buff[SIZE_BUFF];
/// BitStream bs(buff, SIZE_BUFF);
///
/// //
/// // Write data to the BitStream.
/// //
///
/// bs << true;                         // Write a boolean.
/// bs << 2006;                         // Write an integer with default bit count.
/// bs << setBitCount(5) << 15;         // Write a 5-bits integer.
/// bs << 3.1415f;                      // Write a floating number.
///
/// //
/// // The order of read must correspond to the write order, including the bit
/// // count. Otherwise the result is undefined.
/// //
///
/// bs >> bVal;                         // Read a boolean.
/// bs >> iVal1;                        // Read an integer with default bit count.
/// bs >> setBitCount(5) >> iVal2;      // Read a 5-bits integer.
/// bs >> fVal;                         // Read a floating number.
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2006/03/15
///

#pragma once

#include "swinc.h"

namespace sw2 {

///
/// \brief Calculate the max bit need to store a number.(constant)
/// \see Util::getBitCount
///

template<uint n>struct BITCOUNT
{
  enum { value = 1 + BITCOUNT<(n >> 1)>::value };
};

template<> struct BITCOUNT<1>
{
  enum { value = 1 };
};

template<> struct BITCOUNT<0>
{
  enum { value = 1 };
};

struct setBitCount
{
  int bitCount;

  explicit setBitCount(int bc) : bitCount(bc)
  {
  }
};

///
/// \brief Calculate the max bit need to store a number.
/// \param [in] n A number.
/// \return Return the max bit need to store the number.
///

uint getBitCount(uint n);

///
/// \brief Bit stream module.
///

class BitStream
{
public:

  ///
  /// \brief Constructor with stream buffer.
  /// \param [in] buff Stream buffer.
  /// \param [in] szBuff Size of buff in bytes.
  ///

  explicit BitStream(char* buff, int szBuff);

  ///
  /// \brief Construct BitStream with string buffer.
  /// \param [in] s String buffer.
  /// \note The string buffer can grow size when write out of buffer boundary.
  ///

  explicit BitStream(std::string &s);

  ///
  /// \brief Get current bit pointer of current byte.
  /// \return Return current bit.
  ///

  int getBitPtr() const
  {
    return m_bitPtr;
  }

  ///
  /// \brief Get current byte pointer.
  /// \return Return current byte.
  ///

  int getBytePtr() const
  {
    return m_bytePtr;
  }

  ///
  /// \brief Get current byte count include the last un-full byte.
  /// \return Return byte count.
  ///

  int getByteCount() const
  {
    return getBytePtr() + (0 != getBitPtr() ? 1 : 0);
  }

  ///
  /// \brief Set current byte/bit pointer position.
  /// \param [in] bytePtr Position of byte pointer
  /// \param [in] bitPtr Position of bit pointer
  ///

  void setPtr(int bytePtr = 0, int bitPtr = 0);

  ///
  /// \brief Reset current byte/bit pointer to the beginning of the buffer.
  ///

  void reset()
  {
    setPtr(0, 0);
  }

  ///
  /// \brief Check the last read/write operation is fail or not.
  /// \return Return true if last operation is fail else return false.
  ///

  bool fail() const
  {
    return !m_bGood;
  }

  ///
  /// \brief Check the last read/write operation is success or not.
  /// \return Return true if last operation is success else return false.
  ///

  operator bool() const
  {
    return m_bGood;
  }

  ///
  /// \brief Check read/write pointer is out of range.
  /// \return Return true if read/write if out of internal buffer range.
  ///

  bool isOutOfRange(int bitCount) const;

  ///
  /// \brief Set how many bits of next write.
  ///

  BitStream& operator<<(setBitCount const& bc);

  ///
  /// \brief Set how many bits of next read.
  ///

  BitStream& operator>>(setBitCount const& bc);

  ///
  /// \brief Write a boolean value.
  /// \param [in] b True or false.
  ///

  BitStream& operator<<(bool b);

  ///
  /// \brief Write a signed integer.
  /// \param [in] i A signed integer.
  /// \note If write a 10-bits signed integer, then one of the bit will used as
  ///        sign bit and 9-bits for the integer.
  ///

  BitStream& operator<<(int i);

  ///
  /// \brief Write a unsigned integer.
  /// \param [in] u A unsigned integer.
  ///

  BitStream& operator<<(uint u);

  ///
  /// \brief Write a floating point number.
  /// \param [in] f A floating point number.
  ///

  BitStream& operator<<(float f);

  ///
  /// \brief Write a string.
  /// \param [in] s A string.
  ///

  BitStream& operator<<(std::string const& s);

  ///
  /// \brief Write a buffer.
  /// \param [in] pStream The buffer.
  /// \param [in] bitCount How many bits to write.
  /// \return Return true if success else return false.
  /// \note If write is out of buffer boundary then the BitStream keeps no change.
  ///

  bool write(void const* pStream, int bitCount);

  ///
  /// \brief Read a boolean value.
  /// \param [out] b True or false
  ///

  BitStream& operator>>(bool& b);

  ///
  /// \brief Read a signed integer.
  /// \param [out] i A signed integer.
  ///

  BitStream& operator>>(int& i);

  ///
  /// \brief Read a unsigned integer.
  /// \param [out] u A unsigned integer.
  ///

  BitStream& operator>>(uint& u);

  ///
  /// \brief Read a floating number.
  /// \param [out] f A floating number.
  ///

  BitStream& operator>>(float& f);

  ///
  /// \brief Read a string.
  /// \param [out] s A string.
  ///

  BitStream& operator>>(std::string& s);

  ///
  /// \brief Read a buffer.
  /// \param [out] pStream Read data to this buffer.
  /// \param [in] bitCount How many bits to read.
  /// \return Return true if success else return false.
  /// \note If read is out of buffer boundary then no data is read to the buffer.
  ///

  bool read(void* pStream, int bitCount);

private:

  int calcBitsCount(int bitCount, int bitPtr) const;

private:

  std::string *m_sbuff;                 // String buffer.
  char *m_buff;                         // Stream buffer.
  int m_szBuff;                         // Size of stream buffer.

  int m_bitPtr;                         // Current bit pointer of current byte.
  int m_bytePtr;                        // Current byte pointer.

  int m_bitCount;                       // Bit count for next read/write,

  bool m_bGood;                         // State.
};

} // namespace sw2

// end of swBitStream.h
