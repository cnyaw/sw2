//
// HTTP request wrapper.
//
// Copyright (c) 2020 Waync Cheng.
// All Rights Reserved.
//
// 2020/2/6 Waync Created.
//

#include <time.h>

#include "swSocket.h"
#include "swUtil.h"

namespace sw2 {

namespace impl {

class implHttpRequest : public SocketClientCallback
{
public:
  SocketClient* mClient;
  std::string &mData;
  int mTimeout;

  implHttpRequest(std::string &data, int timeout) : mData(data), mTimeout(timeout)
  {
    mClient = SocketClient::alloc(this);
  }

  virtual ~implHttpRequest()
  {
    SocketClient::free(mClient);
  }

  bool get(const std::string& name) const
  {
    assert(mClient);
    size_t urlpos = name.find_first_of('/');
    if (std::string::npos == urlpos) {
      return false;
    }
    const std::string url(name, 0, urlpos);
    if (std::string::npos != url.find_first_of(':')) {
      if (!connect(url)) {
        return false;
      }
    } else {
      if (!connect(url + ":80")) {
        return false;
      }
    }
    std::string get("GET " + name.substr(urlpos) + " HTTP/1.1\r\nHost:" + url + "\r\n\r\n");
    mClient->send((int)get.length(), get.c_str());

    if (!waitData("200 OK") || !waitData("\r\n\r\n")) { // Wait OK and header end.
      disconnect();
      return false;
    }

    size_t headend = mData.find("\r\n\r\n") + 4; // Plus CRLFCRLF.

    if (std::string::npos != mData.find("Transfer-Encoding: chunked")) {
      size_t p = headend;
      unsigned int chunksize = 0;
      if (!waitChunkSize(p, chunksize)) {
        goto error;
      }
      std::string data;
      while (0 < chunksize) {
        if (!waitData(p + chunksize)) {
          goto error;
        }
        data.append(mData, p, chunksize);
        p += chunksize + 2;             // Plus CRLF.
        if (!waitChunkSize(p, chunksize)) {
          goto error;
        }
      }
      disconnect();
      mData = data;
      return true;
    }

    if (std::string::npos != mData.find("Content-Length:")) {
      int datlen = 0;
      sscanf(mData.c_str() + mData.find("Content-Length"), "Content-Length:%d", &datlen);
      if (!waitData(headend + datlen)) {
        goto error;
      }
      disconnect();
      mData = mData.substr(headend, datlen);
      return true;
    }

error:
    disconnect();
    return false;
  }

  //
  // SocketClientCallback.
  //

  virtual void onSocketServerReady(SocketClient*)
  {
    mData.clear();
  }

  virtual void onSocketStreamReady(SocketClient*, int len, void const* pStream)
  {
    mData.append((char const*)pStream, len);
  }

  bool connect(const std::string &url) const
  {
    if (!mClient->connect(url)) {
      return false;
    }
    if (!waitConnected()) {
      return false;
    }
    return true;
  }

  void disconnect() const
  {
    mClient->disconnect();
    waitDisconnected();
  }

  bool waitConnected() const
  {
    return waitState(CS_CONNECTED);
  }

  bool waitChunkSize(size_t &p, unsigned int &chunksize) const
  {
    if (!waitData("\r\n", p)) {         // Wait end of chunk size line.
      return false;
    }
    sscanf(mData.c_str() + p, "%x", &chunksize);
    p = mData.find("\r\n", p) + 2;      // Plus CRLF.
    return true;
  }

  bool waitData(const std::string &token, size_t p = 0) const
  {
    TimeoutTimer lt(1000 * mTimeout);
    while (!lt.isExpired()) {
      mClient->trigger();
      if (std::string::npos != mData.find(token, p)) {
        return true;
      }
    }
    return false;
  }

  bool waitData(size_t length) const
  {
    TimeoutTimer lt(1000 * mTimeout);
    while (!lt.isExpired()) {
      mClient->trigger();
      if (mData.length() >= length) {
        return true;
      }
    }
    return false;
  }

  bool waitDisconnected() const
  {
    return waitState(CS_DISCONNECTED);
  }

  bool waitState(int s) const
  {
    TimeoutTimer lt(1000 * mTimeout);
    while (!lt.isExpired()) {
      mClient->trigger();
      if (mClient->getConnectionState() == s) {
        return true;
      }
    }
    return false;
  }
};

} // namespace impl

bool Util::httpGet(const std::string &url, std::string &resp, int timeout)
{
  impl::implHttpRequest http(resp, timeout);
  return http.get(url);
}

} // namespace sw2
