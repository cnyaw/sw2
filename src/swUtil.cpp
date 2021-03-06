
//
//  Utility routines.
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/02/22 Waync created.
//

#include <ctype.h>

#include <algorithm>
#include <istream>

#if defined(WIN32) || defined(_WIN32_WCE)
# define NOMINMAX
# include <windows.h>
# if !defined(_WIN32_WCE)
#   include <conio.h>
# endif
# if defined(_MSC_VER)
#   pragma comment(lib, "winmm")
# endif
#elif defined(_linux_)
# include <termios.h>
# include <sys/time.h>
# include <time.h>
# include <unistd.h>
#endif

#include "swUtil.h"

namespace sw2 {

namespace impl {

#define BUFF_SIZE 2048

static std::string const base64code("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");

#if defined(_linux_)
class implGetKey
{
public:

  termios mTios, mTiosSave;

  implGetKey()
  {
    tcgetattr(STDIN_FILENO, &mTios);
    memcpy(&mTiosSave, &mTios, sizeof(mTiosSave));
    mTios.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &mTios);
  }

  ~implGetKey()
  {
    tcsetattr(0, TCSANOW, &mTiosSave);
  }

  int kbhit() const
  {
    fd_set rdfs;
    FD_ZERO(&rdfs);
    FD_SET (STDIN_FILENO, &rdfs);
 
    struct timeval tv;
    tv.tv_sec = tv.tv_usec = 0;
 
    select(STDIN_FILENO+1, &rdfs, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &rdfs);
  }

  int getKey() const
  {
    if (kbhit()) {
      return getchar();
    } else {
      return -1;
    }
  }
};

class implGetTickCount
{
public:

  timeval mLastTick;

  implGetTickCount()
  {
    gettimeofday(&mLastTick, 0);
  }

  uint getTickCount() const
  {
    timeval currTick;
    if (0 == gettimeofday(&currTick, 0)) {
      return (currTick.tv_usec - mLastTick.tv_usec) / 1000 +
             (currTick.tv_sec - mLastTick.tv_sec) * 1000;
    }

    return 0;
  }
};
#endif // _linux_

} // namespace impl

using namespace impl;

uint Util::getBitCount(uint n)
{
  if (0 == n) {
    return 1;
  }

  uint bc = 0;
  while (0 < n) {
    bc += 1;
    n >>= 1;
  }

  return bc;
}

int Util::getKey()
{
#if defined(WIN32)
  return ::_kbhit() ? ::_getch() : -1;
#elif defined(_linux_)
  static implGetKey impl;
  return impl.getKey();
#endif
  return -1;
}

int Util::getStreamLen(std::istream &is)
{
  int curPos = (int)is.tellg();
  is.seekg(0, std::ios_base::end);
  int lenStream = (int)is.tellg() - curPos;
  is.seekg(curPos, std::ios_base::beg);
  return lenStream;
}

uint Util::getTickCount()
{
#if defined(WIN32)
  static DWORD lastTick = ::timeGetTime();
  DWORD currTick = ::timeGetTime();
  return (uint)(currTick - lastTick);
#elif defined(_linux_)
  static implGetTickCount impl;
  return impl.getTickCount();
#endif
  return 0;
}

bool Util::isBIG5(int ch)
{
  return (0xa140 <= ch && 0xa3bf >= ch) ||
         (0xa440 <= ch && 0xc67e >= ch) ||
         (0xc6a1 <= ch && 0xc8d3 >= ch) ||
         (0xc940 <= ch && 0xf9fe >= ch);
}

void Util::sleep(uint millsec)
{
#if defined(WIN32)
  ::Sleep(millsec);
#elif defined(_linux_)
  struct timespec req;
  req.tv_sec = millsec / 1000;
  req.tv_nsec = (millsec - req.tv_sec * 1000) * 1000000;
  (void)nanosleep(&req, NULL);
#endif
}

std::string& Util::trim(std::string& str, std::string const& chrTrim)
{
  str.erase(0, str.find_first_not_of(chrTrim));
  str.erase(str.find_last_not_of(chrTrim) + 1);
  return str;
}

bool Util::base64(std::istream& is, std::ostream& os)
{
  int lenStream = getStreamLen(is);
  if (0 >= lenStream) {
    SW2_TRACE_ERROR("Zero length input stream.");
    return false;
  }

  char inBuff[BUFF_SIZE];

  while (0 < lenStream) {

    int lenBuff = std::min(lenStream, BUFF_SIZE);
    if (0 != (lenBuff % 3) && lenBuff != lenStream) { // Adjust.
      lenBuff -= lenBuff % 3;
    }

    if (!is.read(inBuff, lenBuff)) {
      SW2_TRACE_ERROR("Read input failed.");
      return false;
    }

    lenStream -= lenBuff;

    int idxIn = 0;
    for (int i = 0; i < lenBuff; ) {

      int m = i;
      char in[3] = {0};
      for (int j = 0; j < 3 && i < lenBuff; j++, i++, idxIn++) {
        in[j] = inBuff[idxIn];
      }
      m = i - m;

      char out[4] = {0};
      out[0] = base64code[((in[0] & 0xfc) >> 2)];
      out[1] = base64code[(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4))];
      out[2] = base64code[(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6))];
      out[3] = base64code[(in[2] & 0x3f)];

      switch (m)
      {
      case 1:
        out[2] = '=';
      case 2:
        out[3] = '=';
        break;
      }

      if (!os.write(out, 4)) {
        SW2_TRACE_ERROR("Write output failed.");
        return false;
      }
    }
  }

  return true;
}

bool Util::unbase64(std::istream& is, std::ostream& os)
{
  int lenStream = getStreamLen(is);
  if (0 >= lenStream) {
    SW2_TRACE_ERROR("Zero length input stream.");
    return false;
  }

  char inBuff[BUFF_SIZE];

  while (0 < lenStream) {

    int lenBuff = std::min(lenStream, BUFF_SIZE);
    if (0 != (lenBuff % 4) && lenBuff != lenStream) { // Adjust.
      lenBuff -= lenBuff % 4;
    }

    if (!is.read(inBuff, lenBuff)) {
      SW2_TRACE_ERROR("Read input failed.");
      return false;
    }

    lenStream -= lenBuff;

    int idxIn = 0;
    for (int i = 0; i < lenBuff; ) {

      char in[4] = {0}, in2[4] = {0};
      for (int j = 0; j < 4 && i < lenBuff; j++, i++, idxIn++) {
        in[j] = in2[j] = inBuff[idxIn];
        in[j] = (char)(int)base64code.find(in[j]);
      }

      char out[3] = {0};
      out[0] = ((in[0] & 0x3f) << 2) | ((in[1] & 0x30) >> 4);
      out[1] = ((in[1] & 0x0f) << 4) | ((in[2] & 0x3c) >> 2);
      out[2] = ((in[2] & 0x03) << 6) | ((in[3] & 0x3f) >> 0);

      int l = '=' == in2[2] ? 1 : ('=' == in2[3] ? 2 : 3);
      if (!os.write(out, l)) {
        SW2_TRACE_ERROR("Write output failed.");
        return false;
      }
    }
  }

  return true;
}

void Util::utf8ToU32(const char *utf8, std::vector<int> &u)
{
  if (0 == utf8) {
    return;
  }
  int idx = 0;
  for (int i = utf8[idx++]; 0 != i; i = utf8[idx++]) {
    if (0 == (i & 0x80)) {
      u.push_back((int)(i & 0xff));
    } else if (192 == (i & 0xe0)) {
      int j = utf8[idx++];
      if (128 != (j & 0xc0)) {
        return;
      }
      u.push_back((int)(((i & 0x1f) << 6) | (j & 0x3f)));
    } else if (224 == (i & 0xf0)) {
      int k = utf8[idx++];
      int l = utf8[idx++];
      if ((k & 0xc0) != 128 || (l & 0xc0) != 128) {
        return;
      }
      u.push_back((int)(((i & 0xf) << 12) | ((k & 0x3f) << 6) | (l & 0x3f)));
    } else if (240 == (i & 0xf8)) {
      int j = utf8[idx++];
      int k = utf8[idx++];
      int l = utf8[idx++];
      if ((j & 0xc0) != 128 || (k & 0xc0) != 128 || (l & 0xc0) != 128) {
        return;
      }
      u.push_back((int)(((i & 0x7) << 18) | ((j & 0x3f) << 12) | ((k & 0x3f) << 6) | (l & 0x3f)));
    } else {
      return;
    }
  }
}

void Util::u32ToUtf8(const std::vector<int> &u, std::string &utf8)
{
  for (size_t i = 0; i < u.size(); i++) {
    int ch = u[i];
    if (0x80 > ch) {
      utf8.push_back(ch);
    } else if (0x800 >= ch) {
      utf8.push_back((ch >> 6) | 0xc0);
      utf8.push_back((ch & 0x3f) | 0x80);
    } else if (0x10000 >= ch) {
      utf8.push_back((ch >> 12) | 0xe0);
      utf8.push_back(((ch >> 6) & 0x3f) | 0x80);
      utf8.push_back((ch & 0x3f) | 0x80);
    } else if (0x110000 >= ch) {
      utf8.push_back((ch >> 18) | 0xf0);
      utf8.push_back(((ch >> 12) & 0x3f) | 0x80);
      utf8.push_back(((ch >> 6) & 0x3f) | 0x80);
      utf8.push_back((ch & 0x3f) | 0x80);
    }
  }
}

void Util::toLowerString(std::string &str)
{
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
}

TimeoutTimer::TimeoutTimer() : m_timeExpired(0)
{
  m_timeExpired = Util::getTickCount();
}

TimeoutTimer::TimeoutTimer(uint ticks) : m_timeExpired(ticks)
{
  m_timeExpired += Util::getTickCount();
}

bool TimeoutTimer::isExpired() const
{
  return Util::getTickCount() >= m_timeExpired;
}

void TimeoutTimer::setTimeout(uint ticks)
{
  m_timeExpired = Util::getTickCount() + ticks;
}

void TimeoutTimer::setExpiredTime(uint timeExpired)
{
  m_timeExpired = timeExpired;
}

} // namespace sw2

// end of swUtil.cpp
