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
    if (!waitData("200 OK") || !waitData("Content-Length:")) {
      disconnect();
      return false;
    }
    int datlen = 0;
    sscanf(mData.c_str() + mData.find("Content-Length"), "Content-Length:%d", &datlen);
    if (!waitData("\r\n\r\n")) {
      disconnect();
      return false;
    }
    size_t headlen = mData.find("\r\n\r\n") + 4;
    if (!waitData(headlen + datlen)) {
      disconnect();
      return false;
    }
    disconnect();
    mData = mData.substr(headlen, datlen);
    return true;
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

  bool waitData(const std::string &token) const
  {
    TimeoutTimer lt(1000 * mTimeout);
    while (!lt.isExpired()) {
      mClient->trigger();
      if (std::string::npos != mData.find(token)) {
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
