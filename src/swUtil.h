
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

#include <time.h>

#include <algorithm>
#include <iterator>
#include <sstream>
#include <vector>

#include "swinc.h"

namespace sw2 {

class Ini;

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
  /// \return Return new trimmed string.
  ///

  std::string& trim(std::string& str, std::string const& chrTrim = " \t\r\n");

  ///
  /// \brief Split string to a list.
  /// \param [in] s A string.
  /// \param [out] v List to return splitted strings.
  /// \param [in] chrSplit Char to split, default are SPACE,TAB,CR,LN.
  ///

  template<class T>
  void split(const std::string &s, std::vector<T> &v, const std::string &chrSplit = " \t\r\n")
  {
    std::string tmp(s);
    for (size_t i = 0; i < chrSplit.size(); i++) {
      if (' ' != chrSplit[i]) {
        std::replace(tmp.begin(), tmp.end(), chrSplit[i], ' ');
      }
    }
    std::stringstream ss(tmp);
    v.assign(std::istream_iterator<T>(ss), std::istream_iterator<T>());
  }

  ///
  /// \brief Base64 encode.
  /// \param [in] is Input stream.
  /// \param [out] os Output stream.
  /// \return Return true if success else return false.
  /// \note For more info, see http://zh.wikipedia.org/wiki/Base64.
  ///

  bool base64(const std::string& is, std::string& os);

  ///
  /// \brief base64 decode.
  /// \param [in] is Input stream.
  /// \param [out] os Output stream.
  /// \return Return true if success else return false.
  /// \note For more info, see http://zh.wikipedia.org/wiki/Base64.
  ///

  bool unbase64(const std::string& is, std::string& os);

  ///
  /// \brief Zip encode.
  /// \param [in] len Max data length of is to zip.
  /// \param [in] is Input stream.
  /// \param [out] os Output stream.
  /// \param [in] level Zip level -1 ~ 9, the bigger the higher compress rate,
  ///             0 is no compress, -1 is middle setting.
  /// \return Return true if success else return false.
  ///

  bool zip(int len, const char *is, std::string &os, int level = -1);

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
  /// \param [in] len Max data length of is to unzip.
  /// \param [in] is Input stream.
  /// \param [out] os Output stream.
  /// \return Return true if success else return false.
  ///

  bool unzip(int len, const char *is, std::string &os);

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

  bool crc32(uint& value, const std::string& is, uint len = 0);

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
  /// \brief Check is the stream a zip archive.
  /// \param [in] stream The stream.
  /// \return Return true if the stream is a zip archive else return false.
  ///

  bool isZipStream(const std::string& stream);

  ///
  /// \brief Check is the file a zip archive.
  /// \param [in] stream The file path.
  /// \return Return true if the file is a zip archive else return false.
  ///

  bool isZipFile(std::string const& path);

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

  //
  // \brief Get format string of uptime.
  // \param [out] buff Buffer to retrieve result format string.
  // \param [in] szBuff Size in bytes of buff.
  // \param [in] pTime Total time in seconds.
  // \return Return a format time string in [???y???d]hh:ss:ss.
  //

  char* fmtUpTime(char *buff, size_t szBuff, const time_t *pTime);

  //
  // \brief Get format string of size of bytes.
  // \param [out] buff Buffer to retrieve result format string.
  // \param [in] szBuff Size in bytes of buff.
  // \param [in] bytes Total size in bytes.
  // \return Return a format size of bytes string in ???[k|m|g|t|p|e|z|y].
  //

  char* fmtSizeByte(char *buff, size_t szBuff, const unsigned long long *bytes);

  //
  // \brief Load file binary content to str.
  // \param [in] filename Name of file to load.
  // \param [out] str A std::string as buffer to get file content.
  // \return Return true if load file success else return false.
  //

  bool loadFileContent(const char *filename, std::string &str);

  //
  // \brief Store binary str content to file.
  // \param [in] filename Name of file to store.
  // \param [in] str A std::string binary content to store to file.
  // \return Return true if store file success else return false.
  //

  bool storeFileContent(const char *filename, const std::string &str);

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
  /// \brief Set new timeout time immediately.
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

///
/// \brief Save log to file utility.
/// \note LogFile will use ThreadTask to perform file save, therefore
/// before save logs to file InitializeThreadPool is required.
///

class LogFile
{
public:

  ///
  /// \brief Allocate a LogFile instance.
  /// \return If success return an interface pointer else return 0.
  ///

  static LogFile* alloc();

  ///
  /// \brief Release a unused LogFile instance.
  /// \param [in] pItf Instance to free.
  ///

  static void free(LogFile* pItf);

  ///
  /// \brief Set log file directory name.
  /// \param [in] dir Director name where log file saves.
  ///

  virtual void setDir(const std::string &dir)=0;

  ///
  /// \brief Set log file name.
  /// \param [in] name File name postfix of log file. File name format is: '%Y-%m-%dname'.
  ///

  virtual void setFileName(const std::string &name)=0;

  ///
  /// \brief Add log to log file.
  /// \param [in] log New log to add to file.
  ///

  virtual void addLog(const std::string &log)=0;

  ///
  /// \brief Perform save logs to log file.
  ///

  virtual void saveLogs()=0;
};

///
/// \brief Simple key states handler.
/// \note Each key state is a user defined bit flag.
///

class KeyStates
{
  uint m_keys, m_prevKeys;
public:
  KeyStates();

  //
  // \brief Get current key states.
  // \return Current key states.
  //

  uint keys() const
  {
    return m_keys;
  }

  //
  // \brief Get previous key states.
  // \return Previous key states.
  //

  uint prevKeys() const
  {
    return m_prevKeys;
  }

  //
  // \brief Reset key states.
  //

  void reset()
  {
    m_keys = m_prevKeys = 0;
  }

  //
  // \brief Check is a key down.
  // \param [in] keys Key states to check.
  // \param [in] key The key flag to check is down.
  // \return Return true if key is down, else return false.
  //

  bool isKeyDown(uint keys, uint key) const;

  //
  // \brief Check is a key down.
  // \param [in] key The key flag to check is down.
  // \return Return true if key is down, else return false.
  //

  bool isKeyDown(uint key) const;

  //
  // \brief Check is a key pressed.
  // \param [in] key The key flag to check is pressed.
  // \return Return true if key is pressed, else return false.
  // \note A key is pressed mean it is down then up.
  //

  bool isKeyPressed(uint key) const;

  //
  // \brief Check is a key pushed.
  // \param [in] key The key flag to check is pushed.
  // \return Return true if key is pushed, else return false.
  // \note A key is pushed mean it is down and not up.
  //

  bool isKeyPushed(uint key) const;

  //
  // \brief Update current key states and original key states become previous states.
  // \param [in] keys New key states.
  //

  void update(uint keys);
};

///
/// \brief Helper for game loop to control FPS(frame per second).
///

class FpsHelper
{
  int m_timePerFrame, m_fpsValue, m_framesPerSecond;
  clock_t m_timeStart, m_timeNextFrame, m_lastTime;
  unsigned long m_ticks;
public:

  //
  // \brief Get current FPS.
  // \return Return current FPS value.
  //

  int getFps() const
  {
    return m_fpsValue;
  }

  //
  // \brief Get total tick count from application starts.
  // \return Return total tick count.
  //

  unsigned long getTicks() const
  {
    return m_ticks;
  }

  //
  // \brief Application start.
  // \param [in] DesireFps A desire FPS value for the application game loop.
  //

  void start(int DesireFps = 60);

  //
  // \brief Each time application updates invoke this to update FPS value.
  //

  void tick();

  //
  // \brief Each time after application updates invoke this to sleep until next frame.
  //

  void wait();
};

} // namespace sw2

// end of swUtil.h
