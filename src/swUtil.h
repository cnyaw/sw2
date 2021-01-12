
//
//  Utility routines.
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/02/22 Waync created.
//

///
/// \file
/// \brief Utility routines.
/// \author Waync Cheng
/// \date 2005/02/22
///

#pragma once

#include <iterator>
#include <vector>

#include "swinc.h"

namespace sw2 {

class Ini;

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

///
/// \brief Utility routines.
///

namespace Util {

  ///
  /// \brief Clamp the number to the specified range.
  /// \param [in] value The number.
  /// \param [in] a Min value.
  /// \param [in] b Max value.
  /// \return Return the number between a and b.
  ///

  template<typename T>
  static T clamp(T value, T a, T b)
  {
    if (value > b) {
      return b;
    }

    if (value < a) {
      return a;
    }

    return value;
  }

  ///
  /// \brief Get a random number between two specified numbers.
  /// \param [in] a Min value.
  /// \param [in] b Max value.
  /// \return Return a random number between a and b.
  ///

  template<typename T>
  static T rangeRand(T a, T b)
  {
    return (T)(a + (b - a) * (rand() / (float)RAND_MAX));
  }

  ///
  /// \brief Get length in bytes of an input stream.
  /// \param [in] is An input stream.
  /// \return Return length in bytes.
  ///

  int getStreamLen(std::istream &is);

  ///
  /// \brief Calculate the max bit need to store a number.
  /// \param [in] n A number.
  /// \return Return the max bit need to store the number.
  ///

  uint getBitCount(uint n);

  ///
  /// \brief Read a key from keyboard.
  /// \return Return the key else return -1 if no input.
  ///

  int getKey();

  ///
  /// \brief Pause current thread for a while.
  /// \param [in] millsec Millisecond to pause.
  ///

  void sleep(uint millsec);

  ///
  /// \brief Get the tick count from the application up to now.(milliseconds)
  /// \return Return the tick count from the application up to now.
  ///

  uint getTickCount();

  ///
  /// \brief Check is this a BIG5 code?
  /// \param [in] ch A char.(double char)
  /// \return Return true if ch is a BIG5 code else return false.
  ///

  bool isBIG5(int ch);

  ///
  /// \brief Trim specified chars in the begin and end of the string.
  /// \param [in] str A string.
  /// \param [in] chrTrim Char to trim, default are SPACE,TAB,CR,LN.
  /// \return Return new trimed string.
  ///

  std::string& trim(std::string& str, std::string const& chrTrim = " \t\r\n");

  ///
  /// \brief Base64 encode.
  /// \param [in] is Input stream.
  /// \param [out] os Output stream.
  /// \return Return true if success else return false.
  /// \note For more info, see http://zh.wikipedia.org/wiki/Base64.
  ///

  bool base64(std::istream& is, std::ostream& os);

  ///
  /// \brief base64 decode.
  /// \param [in] is Input stream.
  /// \param [out] os Output stream.
  /// \return Return true if success else return false.
  /// \note For more info, see http://zh.wikipedia.org/wiki/Base64.
  ///

  bool unbase64(std::istream& is, std::ostream& os);

  ///
  /// \brief Zip encode.
  /// \param [in] is Input stream.
  /// \param [out] os Output stream.
  /// \param [in] level Zip level -1 ~ 9, the bigger the higher compress rate,
  ///             0 is no compress, -1 is middle setting.
  /// \return Return true if success else return false.
  ///

  bool zip(std::istream& is, std::ostream& os, int level = -1);

  ///
  /// \brief Zip decode.
  /// \param [in] is Input stream.
  /// \param [out] os Output stream.
  /// \param [in] len Max unzip data length, 0 to unzip all data in is stream.
  /// \return Return true if success else return false.
  ///

  bool unzip(std::istream& is, std::ostream& os, uint len = 0);

  ///
  /// \brief Get CRC32 value.
  /// \param [in,out] value CRC32 value
  /// \param [in] is Input stream.
  /// \param [in] len Max data length to process, 0 to process all data in is.
  /// \note The value on input is the initial value of CRC32, general is 0.
  /// \return Return true if success else return false.
  ///

  bool crc32(uint& value, std::istream& is, uint len = 0);

  ///
  /// \brief Create new Zip archive or append files to a Zip archive.
  /// \param [in] bCreateNew Create new Zip archive or append files to a Zip archive?
  /// \param [in] zipName Zip archive path name.
  /// \param [in] items Items to add/append, the path name is relative to Zip archive.
  /// \param [in] password Password of Zip archive, can be empty.
  /// \return Return true if success else return false.
  ///

  bool zipArchive(bool bCreateNew, std::string const& zipName, std::vector<std::string> const& items, std::string const& password = "");

  ///
  /// \brief Create new Zip stream or append files to a Zip stream.
  /// \param [in] path Source file path.
  /// \param [in] is Input stream.
  /// \param [out] os Output stream.
  /// \param [in] items Items to add/append, the path name is relative to path.
  /// \param [in] password Password of Zip stream, can be empty.
  /// \return Return true if success else return false.
  ///

  bool zipStream(std::string const& path, std::istream& is, std::ostream& os, std::vector<std::string> const& items, std::string const& password = "");

  ///
  /// \brief Create form widget.
  /// \param [in] hParent Parent widget.
  /// \param [in] res Resource INI.
  /// \param [in] name Form name.
  /// \return Return form handle if create success else return -1.
  ///

  int createWidget(int hParent, Ini const& res, std::string const& name);

  ///
  /// \brief Set unhandled exception filter.
  ///

  void setUnhandledExceptionFilter();

  //
  // \brief Convert utf8 text to unicode array.
  //

  void utf8ToU32(const char *utf8, std::vector<int> &u);

  //
  // \brief Convert unicode array to utf8 string.
  //

  void u32ToUtf8(const std::vector<int> &u, std::string &utf8);

  //
  // \brief Send HTTP GET request to url and get response.
  // \param [in] url Target URL.
  // \param [out] resp The response from url.
  // \param [in] timeout Timeout time in seconds.
  // \return Return true if request success else false.
  //

  bool httpGet(const std::string &url, std::string &resp, int timeout = 5);

  //
  // \brief Convert string to lowercase.
  // \param [in/out] str String to convert to lowercase.
  //

  void toLowerString(std::string &str);

} // namespace Util

///
/// \brief Timeout timer utility.
///

class TimeoutTimer
{
  uint m_timeExpired;
public:

  TimeoutTimer();
  explicit TimeoutTimer(uint ticks);

  ///
  /// \brief Check is the timer expired.
  /// \return Return true if the timer is expired else return false.
  ///

  bool isExpired() const;

  ///
  /// \brief Set new timeout time.
  /// \param [in] ticks How many ticks to check from now.
  ///

  void setTimeout(uint ticks);

  ///
  /// \brief Set new timeout time imediately.
  /// \param [in] timeExpired New expired time.
  ///

  void setExpiredTime(uint timeExpired);

  //
  // \brief Get expired time.
  // \return Expired time.
  //

  uint getExpiredTime() const
  {
    return m_timeExpired;
  }
};

} // namespace sw2

// end of swUtil.h
