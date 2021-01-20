
//
//  Socket unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/11/03 Waync created.
//

#include "CppUnitLite/TestHarness.h"

#include "swSocket.h"
#include "swThreadPool.h"
#include "swUtil.h"
using namespace sw2;

class TestSocketClient : public SocketClientCallback
{
public:

  SocketClient* mClient;

  int mFeedbackCnt;
  std::string mData;

  bool mReady;

  TestSocketClient() : mFeedbackCnt(0), mReady(false)
  {
    mClient = SocketClient::alloc(this);
  }

  virtual ~TestSocketClient()
  {
    SocketClient::free(mClient);
  }

  virtual void onSocketServerReady(SocketClient*)
  {
    mReady = true;
  }

  virtual void onSocketServerLeave(SocketClient*)
  {
    mReady = false;
  }

  virtual void onSocketStreamReady(SocketClient*, int len, void const* pStream)
  {
    mFeedbackCnt += len;
    mData.append((char const*)pStream, len);
  }
};

class TestSocketServer : public SocketServerCallback
{
public:

  bool bAllowConnect;
  SocketServer* mServer;

  int mRecvCnt;
  std::string mData;

  int mOnline;
  bool mReady;

  TestSocketServer(bool allowConnect = true) :
    bAllowConnect(allowConnect),
    mRecvCnt(0),
    mOnline(0),
    mReady(false)
  {
    mServer = SocketServer::alloc(this);
  }

  virtual ~TestSocketServer()
  {
    SocketServer::free(mServer);
  }

  virtual void onSocketServerStartup(SocketServer*)
  {
    mReady = true;
  }

  virtual void onSocketServerShutdown(SocketServer*)
  {
    mReady = false;
  }

  virtual bool onSocketNewClientReady(SocketServer*, SocketConnection* pNewClient)
  {
    if (bAllowConnect) {
      mOnline += 1;
      return true;
    }

    return false;
  }

  virtual void onSocketClientLeave(SocketServer*, SocketConnection* pClient)
  {
    mOnline -= 1;
  }

  virtual void onSocketStreamReady(SocketServer*, SocketConnection* pClient, int len, void const* pStream)
  {
    mRecvCnt += len;
    mData.append((char const*)pStream, len);

    std::string s(len, 'F');
    pClient->send(len, s.data());
  }
};

std::string GetTestRepStr()
{
  std::string s;
  while (true) {
    s += "This+is+a+test+str+pattern.";
    if (5000 <= s.size()) {
      break;
    }
  }
  return s;
}

class TestClientThreadPool : public ThreadTask
{
public:

  virtual void threadTask()
  {
    TestSocketClient c;
    c.mClient->connect("localhost:2345");
    sw2::TimeoutTimer lt(3000);
    while (!lt.isExpired()) {
      c.mClient->trigger();
      if (CS_CONNECTED == c.mClient->getConnectionState()) {
        break;
      }
    }
    const std::string s = GetTestRepStr();
    c.mClient->send((int)s.size(), s.data());
    c.mClient->trigger();
    c.mClient->disconnect();
  }
};

class TestServerThreadPool : public ThreadTask
{
public:

  bool done;

  virtual void threadTask()
  {
    done = false;
    const std::string ts = GetTestRepStr();
    TestSocketServer s;
    s.mServer->startup("localhost:2345");
    sw2::TimeoutTimer lt(4000);
    while (!lt.isExpired()) {
      s.mServer->trigger();
      done = ts == s.mData;
      if (done) {
        break;
      }
    }
    s.mServer->shutdown();
  }
};

//
// init/uninit.
//

TEST(Socket, init)
{
  CHECK(InitializeSocket());
  UninitializeSocket();
}

//
// Test connect 1.
//

TEST(Socket, connect1)
{
  CHECK(InitializeSocket());

  {
    std::string const addr = "localhost:1213";

    TestSocketServer s;
    CHECK(!s.mReady);
    CHECK(s.mServer->startup(addr));    // Start listen.

    TestSocketClient c;
    CHECK(CS_DISCONNECTED == c.mClient->getConnectionState());
    CHECK(c.mClient->connect(addr));    // Connect to server.
    CHECK(CS_CONNECTING == c.mClient->getConnectionState() ||
          CS_CONNECTED == c.mClient->getConnectionState());

    //
    // Wait a client to connect to server for 5 seconds.
    //

    CHECK(s.mReady);
    sw2::TimeoutTimer lt(5000);
    while (!lt.isExpired() && 0 == s.mServer->getNetStats().currOnline) {
      s.mServer->trigger();
      c.mClient->trigger();
    }

    //
    // Now, there is 1 online player.
    //

    CHECK(1 == s.mServer->getNetStats().currOnline);
    CHECK(1 == s.mOnline);
    CHECK(CS_CONNECTED == c.mClient->getConnectionState());
    CHECK(c.mReady);

    //
    // Wait disconnection done.
    //

    c.mClient->disconnect();            // disconnect.
    s.mServer->shutdown();              // Stop listen.

    lt.setTimeout(5000);
    while (!lt.isExpired() && CS_DISCONNECTED != c.mClient->getConnectionState()) {
      s.mServer->trigger();
      c.mClient->trigger();
    }

    CHECK(CS_DISCONNECTED == c.mClient->getConnectionState());
    CHECK(!c.mReady);
    CHECK(0 == s.mOnline);
    CHECK(!s.mReady);
  }

  UninitializeSocket();
}

//
// Test connect 2.
//

TEST(Socket, connect2)
{
  CHECK(InitializeSocket());

  {
    std::string const addr = "localhost:1213";

    TestSocketServer s(false);          // Not allow connect.
    CHECK(s.mServer->startup(addr));

    TestSocketClient c;
    CHECK(c.mClient->connect(addr));

    sw2::TimeoutTimer lt(2000);
    while (!lt.isExpired() && 0 == s.mServer->getNetStats().currOnline) {
      s.mServer->trigger();
      c.mClient->trigger();
    }

    //
    // Now, there is 0 online player.
    //

    CHECK(0 == s.mServer->getNetStats().currOnline);
    CHECK(0 == s.mOnline);

    s.mServer->shutdown();
  }

  UninitializeSocket();
}

//
// Test send/recv data.
//

TEST(Socket, sendrecv)
{
  CHECK(InitializeSocket());

  {
    std::string const addr = "127.0.0.1:1213";

    TestSocketServer s;
    CHECK(s.mServer->startup(addr));

    TestSocketClient c;
    CHECK(c.mClient->connect(addr));

    sw2::TimeoutTimer lt(5000);
    while (!lt.isExpired() && 0 == s.mServer->getNetStats().currOnline) {
      s.mServer->trigger();
      c.mClient->trigger();
    }

    CHECK(1 == s.mServer->getNetStats().currOnline);

    //
    // Send/recv data.
    //

    {
      int cnt[] = {80, 123, 256, 337, 386, 512, 680, 1024, 1500, 1980};
      int total = 0;
      for (int i = 0; i < (int)(sizeof(cnt)/sizeof(cnt[0])); ++i) {
        std::string s(cnt[i], 'S');
        CHECK(c.mClient->send(cnt[i], s.data()));
        total += cnt[i];
      }

      lt.setTimeout(5000);
      while (!lt.isExpired() && s.mRecvCnt != total) {
        s.mServer->trigger();
        c.mClient->trigger();
      }

      CHECK(s.mRecvCnt == total);
      CHECK(s.mData == std::string(total, 'S'));
      CHECK(s.mServer->getNetStats().bytesRecv == (uint64)total);
      CHECK(s.mServer->getNetStats().bytesSent == (uint64)total);

      while (!lt.isExpired() && c.mFeedbackCnt != total) {
        s.mServer->trigger();
        c.mClient->trigger();
      }

      CHECK(c.mFeedbackCnt == total);
      CHECK(c.mData == std::string(total, 'F'));
      CHECK(c.mClient->getNetStats().bytesRecv == (uint64)total);
      CHECK(c.mClient->getNetStats().bytesSent == (uint64)total);
    }

    c.mClient->disconnect();
    s.mServer->shutdown();

    while (0 != s.mServer->getNetStats().currOnline) {
      s.mServer->trigger();
      c.mClient->trigger();
    }
  }

  UninitializeSocket();
}

TEST(Socket, sendrecv2)
{
  CHECK(InitializeSocket());
  CHECK(InitializeThreadPool(4));
  {
    TestServerThreadPool s;
    CHECK(s.runTask());
    TestClientThreadPool c;
    CHECK(c.runTask());
    sw2::TimeoutTimer lt(5000);
    while (c.isRunning() || s.isRunning()) {
      if (lt.isExpired()) {
        break;
      }
    }
    CHECK(s.done);
  }
  UninitializeThreadPool();
  UninitializeSocket();
}

// end of TestSocket.cpp
