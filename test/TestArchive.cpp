
//
//  Archive unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/05/31 Waync created.
//

#include <sstream>

#include "CppUnitLite/TestHarness.h"

#include "swArchive.h"
#include "swSocket.h"
#include "swUtil.h"
using namespace sw2;

//
// Alloc/free.
//

TEST(Archive, allocAndFree)
{
  Archive* par = Archive::alloc();
  CHECK(0 != par);                      // Maybe fail, but should impossible.

  Archive::free(par);
  par = 0;
}

//
// Add file system.
//

TEST(Archive, addFileSystem1)
{
  Archive* par = Archive::alloc();
  if (0 == par) {
    return;
  }

  CHECK(par->addFileSystem("./data/")); // Exist folder.
  CHECK(par->addFileSystem("./dummy/")); // Not exist folder is allowed.

  Archive::free(par);
  par = 0;
}

//
// Add zip file system.
//

TEST(Archive, addFileSystem2)
{
  Archive* par = Archive::alloc();
  if (0 == par) {
    return;
  }

  CHECK(!par->addFileSystem("test.zip")); // Not exist file.
  CHECK(par->addFileSystem("./data/test2.zip")); // Exist file.

  Archive::free(par);
  par = 0;
}

//
// Check file existence.
//

TEST(Archive, isExist)
{
  Archive* par = Archive::alloc();
  if (0 == par) {
    return;
  }

  CHECK(par->addFileSystem("./data/"));
  CHECK(par->addFileSystem("./data/test2.zip"));

  CHECK(par->isFileExist("test.txt"));  // Exist in test2.zip.
  CHECK(par->isFileExist("test2.txt")); // Exist in test2.zip.
  CHECK(par->isFileExist("test3.zip")); // Exist in .test2.zip.
  CHECK(par->isFileExist("test2.zip")); // Exist in ./data.
  CHECK(par->isFileExist("test4.zip.dat")); // Exist in ./data.
  CHECK(par->isFileExist("test5.zip")); // Exist in ./data.
  CHECK(!par->isFileExist("test4.txt")); // Not exist.
  CHECK(!par->isFileExist("test5.txt")); // Not exist.
  CHECK(!par->isFileExist("test6.txt")); // Not exist.

  Archive::free(par);
  par = 0;
}

//
// Load file.
//

TEST(Archive, loadFile)
{
  Archive* par = Archive::alloc();
  if (0 == par) {
    return;
  }

  CHECK(par->addFileSystem("./data/"));
  CHECK(par->addFileSystem("./data/test2.zip"));
  CHECK(par->addFileSystem("./data/test4.zip.dat")); // Password protected file.

  std::stringstream ss;
  CHECK(par->loadFile("./test.txt", ss));
  CHECK(ss.str() == "this is test.txt\r\n"); // Validate content.

  std::stringstream ss2;
  CHECK(par->loadFile("./test2.txt", ss2));
  CHECK(ss2.str() == "this is test2.txt\r\n"); // Validate content.

  std::stringstream ss3;
  CHECK(!par->loadFile("./test4.txt", ss3));
  CHECK(par->loadFile("./test4.txt", ss3, "123456"));
  CHECK(ss3.str() == "this is password protected test4.txtthis is password protected test4.txtthis is password protected test4.txt\r\n"); // Validate content.

  Archive::free(par);
  par = 0;
}

//
// Add memory zip file system.
//

TEST(Archive, addFileSystem3)
{
  Archive* par = Archive::alloc();
  if (0 == par) {
    return;
  }

  CHECK(par->addFileSystem("./data/test2.zip"));

  std::stringstream ss1;
  CHECK(par->loadFile("test3.zip", ss1));

  CHECK(par->addFileSystem(ss1));

  Archive::free(par);
  par = 0;
}

//
// Load file 2.
//

TEST(Archive, loadFile2)
{
  Archive* par = Archive::alloc();
  if (0 == par) {
    return;
  }

  CHECK(par->addFileSystem("./data/test2.zip"));

  std::stringstream ss1;
  CHECK(par->loadFile("test3.zip", ss1));

  CHECK(par->addFileSystem(ss1));       // Memory zip file system.

  std::stringstream ss;
  CHECK(par->loadFile("test3.txt", ss));
  CHECK(ss.str() == "this is test3.txt\r\n");

  Archive::free(par);
  par = 0;
}

//
// Test search order. First added, last searched.
//

TEST(Archive, searchOrder)
{
  Archive* par = Archive::alloc();
  if (0 == par) {
    return;
  }

  CHECK(par->addFileSystem("./data/"));

  std::stringstream ss;
  CHECK(par->loadFile("./test.txt", ss));
  CHECK(ss.str() == "this is test.txt\r\n");

  CHECK(par->addFileSystem("./data/test5.zip"));

  std::stringstream ss2;
  CHECK(par->loadFile("./test.txt", ss2));
  CHECK(ss2.str() == "this is test.txt in test5.zip.\r\n");

  Archive::free(par);
}

//
// User define file system.
//

class TestFileSystem : public ArchiveFileSystem
{
public:
  virtual bool isFileExist(std::string const& name) const
  {
    if ("test" == name || "sub/test" == name) {
      return true;
    } else {
      return false;
    }
  }
  virtual bool loadFile(std::string const& name, std::ostream& outs, std::string const& password) const
  {
    if ("test" == name) {
      outs << "this is test";
      return true;
    } else if ("sub/test" == name) {
      outs << "this is sub/test";
      return true;
    } else {
      return false;
    }
  }
};

TEST(Archive, addFileSystem4)
{
  Archive* par = Archive::alloc();
  if (0 == par) {
    return;
  }
  
  TestFileSystem fs;
  CHECK(par->addFileSystem(&fs));

  CHECK(par->isFileExist("./test"));
  std::stringstream ss;
  CHECK(par->loadFile("./test", ss));
  CHECK(ss.str() == "this is test");

  CHECK(par->isFileExist("./sub/test"));
  std::stringstream ss2;
  CHECK(par->loadFile("./sub/test", ss2));
  CHECK(ss2.str() == "this is sub/test");

  Archive::free(par);
}

class HttpFileSystem : public SocketClientCallback, public ArchiveFileSystem
{
public:

  SocketClient* mClient;
  std::string mData;

  HttpFileSystem()
  {
    mClient = SocketClient::alloc(this);
  }

  virtual ~HttpFileSystem()
  {
    SocketClient::free(mClient);
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

  bool waitConnected() const
  {
    return waitState(CS_CONNECTED);
  }

  bool waitDisconnected() const
  {
    return waitState(CS_DISCONNECTED);
  }

  bool waitState(int s) const
  {
    sw2::TimeoutTimer lt(5000);
    while (!lt.isExpired()) {
      mClient->trigger();
      if (mClient->getConnectionState() == s) {
        return true;
      }
    }
    return false;
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

  bool waitData(const std::string &token) const
  {
    sw2::TimeoutTimer lt(5000);
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
    sw2::TimeoutTimer lt(5000);
    while (!lt.isExpired()) {
      mClient->trigger();
      if (mData.length() >= length) {
        return true;
      }
    }
    return false;
  }

  //
  // ArchiveFileSystem.
  //

  bool get(const std::string& name) const
  {
    assert(mClient);
    size_t urlpos = name.find_first_of('/');
    if (std::string::npos == urlpos) {
      return false;
    }
    const std::string url(name, 0, urlpos);
    if (!connect(url + ":80")) {
      return false;
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
    size_t headlen = mData.find("\r\n\r\n");
    if (!waitData(headlen + datlen)) {
      disconnect();
      return false;
    }
    disconnect();
    return true;
  }

  virtual bool isFileExist(std::string const& name) const
  {
    return false;
  }

  virtual bool loadFile(std::string const& name, std::ostream& outs, std::string const& password) const
  {
    assert(mClient);
    if (!get(name)) {
      return false;
    }
    size_t headlen = mData.find("\r\n\r\n");
    assert(std::string::npos != headlen);
    outs.write(mData.data() + headlen + 4, (int)(mData.length() - headlen - 4));
    return true;
  }
};

TEST(Archive, httpfs)
{
  Archive* par = Archive::alloc();
  if (0 == par) {
    return;
  }

  CHECK(InitializeSocket());
  {
    HttpFileSystem fs;
    CHECK(par->addFileSystem(&fs));

    std::stringstream ss;
    CHECK(par->loadFile("www.rfc-editor.org/rfc/rfc1.txt", ss));
    CHECK(std::string::npos != ss.str().find("Network Working Group Request for Comment:   1"));

    std::stringstream ss2;
    CHECK(par->loadFile("www.rfc-editor.org/rfc/rfc2.txt", ss2));
    CHECK(std::string::npos != ss2.str().find("1a1 Logical link 0 will be a control link between any two HOSTs on"));
  }
  Archive::free(par);
  UninitializeSocket();
}

// end of TestArchive.cpp
