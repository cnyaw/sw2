
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
#include "swThreadPool.h"
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

  CHECK(par->addPathFileSystem("./data/")); // Exist folder.
  CHECK(par->addPathFileSystem("./dummy/")); // Not exist folder is allowed.

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

  CHECK(!par->addPathFileSystem("test.zip")); // Not exist file.
  CHECK(par->addPathFileSystem("./data/test2.zip")); // Exist file.

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

  CHECK(par->addPathFileSystem("./data/"));
  CHECK(par->addPathFileSystem("./data/test2.zip"));

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

  CHECK(par->addPathFileSystem("./data/"));
  CHECK(par->addPathFileSystem("./data/test2.zip"));
  CHECK(par->addPathFileSystem("./data/test4.zip.dat")); // Password protected file.

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

  CHECK(par->addPathFileSystem("./data/test2.zip"));

  std::stringstream ss1;
  CHECK(par->loadFile("test3.zip", ss1));

  CHECK(par->addStreamFileSystem(ss1.str()));

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

  CHECK(par->addPathFileSystem("./data/test2.zip"));

  std::stringstream ss1;
  CHECK(par->loadFile("test3.zip", ss1));

  CHECK(par->addStreamFileSystem(ss1.str())); // Memory zip file system.

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

  CHECK(par->addPathFileSystem("./data/"));

  std::stringstream ss;
  CHECK(par->loadFile("./test.txt", ss));
  CHECK(ss.str() == "this is test.txt\r\n");

  CHECK(par->addPathFileSystem("./data/test5.zip"));

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
  CHECK(par->addArchiveFileSystem(&fs));

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

class HttpFileServer : public SocketServerCallback
{
public:
  SocketServer* m_pServer;
  std::string m_strThePoolOfTears, m_strContentLength, m_strChunked;

  HttpFileServer()
  {
    m_pServer = SocketServer::alloc(this);
    if (m_pServer) {
      m_pServer->startup("24680");
    }
    Archive *fs = Archive::alloc();
    assert(fs);
    fs->addPathFileSystem("./data/httpGet.zip");
    std::stringstream ss;
    if (fs->loadFile("ThePoolOfTears.txt", ss)) {
      m_strThePoolOfTears = ss.str();
    }
    // Following two test data is from http://www.tcpipguide.com/free/t_HTTPDataLengthIssuesChunkedTransfersandMessageTrai-3.htm.
    ss.str("");
    if (fs->loadFile("ContentLength.txt", ss)) {
      m_strContentLength = ss.str();
    }
    ss.str("");
    if (fs->loadFile("Chunked.txt", ss)) {
      m_strChunked = ss.str();
    }
    Archive::free(fs);
  }

  virtual ~HttpFileServer()
  {
    SocketServer::free(m_pServer);
    m_pServer = 0;
  }

  void trigger()
  {
    if (m_pServer) {
      m_pServer->trigger();
    }
  }

  virtual void onSocketStreamReady(SocketServer*, SocketConnection* pClient, int len, void const* pStream)
  {
    const char *HTTP_GET_THE_POOL_OF_TEARS = "GET /ThePoolOfTears.txt";
    const char *HTTP_GET_CONTENT_LENGTH = "GET /ContentLength.txt";
    const char *HTTP_GET_CHUNKED = "GET /Chunked.txt";
    std::string buff((const char*)pStream, len);
    if (!strncmp(buff.c_str(), HTTP_GET_THE_POOL_OF_TEARS, strlen(HTTP_GET_THE_POOL_OF_TEARS))) {
      const char fmt[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: %d\r\n\r\n" ;
      std::string resp;
      resp.resize(strlen(fmt) + m_strThePoolOfTears.size() + 32);
      sprintf((char*)resp.c_str(), fmt, (int)m_strThePoolOfTears.size());
      std::string s = resp.c_str() + m_strThePoolOfTears;
      pClient->send((int)s.size(), s.data());
    } else if (!strncmp(buff.c_str(), HTTP_GET_CONTENT_LENGTH, strlen(HTTP_GET_CONTENT_LENGTH))) {
      pClient->send((int)m_strContentLength.size(), m_strContentLength.data());
    } else if (!strncmp(buff.c_str(), HTTP_GET_CHUNKED, strlen(HTTP_GET_CHUNKED))) {
      pClient->send((int)m_strChunked.size(), m_strChunked.data());
    } else {
      const char *err = "HTTP/1.0 404 Not Found\r\nContent-Type: text/html\r\nContent-Length: 0\r\n\r\n" ;
      pClient->send((int)strlen(err), err);
    }
    pClient->disconnect();
  }
};

class HttpFileServerTrigger : public ThreadTask
{
  HttpFileServer &m_server;
public:
  HttpFileServerTrigger(HttpFileServer &server) : m_server(server)
  {
  }

  virtual void threadTask()
  {
    TimeoutTimer tt(1000);
    while (!tt.isExpired()) {
      m_server.trigger();
      Util::sleep(1);
    }
  }
};

class HttpFileSystem : public ArchiveFileSystem
{
public:

  //
  // ArchiveFileSystem.
  //

  virtual bool isFileExist(std::string const& name) const
  {
    return false;
  }

  virtual bool loadFile(std::string const& name, std::ostream& outs, std::string const& password) const
  {
    std::string resp;
    if (Util::httpGet(name, resp, 1)) {
      outs.write(resp.data(), (int)resp.length());
      return true;
    } else {
      return false;
    }
  }
};

TEST(Archive, httpfs)
{
  CHECK(InitializeThreadPool(1));
  CHECK(InitializeSocket());

  Archive* par = Archive::alloc();
  if (par) {
    HttpFileServer svr;
    HttpFileServerTrigger httptask(svr);
    httptask.runTask();

    HttpFileSystem fs;
    CHECK(par->addArchiveFileSystem(&fs));

    std::stringstream ss;
    CHECK(par->loadFile("localhost:24680/ThePoolOfTears.txt", ss));
    CHECK(ss.str() == svr.m_strThePoolOfTears);
    ss.str("");
    CHECK(par->loadFile("localhost:24680/ContentLength.txt", ss));
    std::string strCheck = svr.m_strContentLength.substr(4 + svr.m_strContentLength.find("\r\n\r\n"));
    strCheck = strCheck.substr(0, strCheck.find("\r\n\r\n"));
    CHECK(ss.str() == strCheck);
    ss.str("");
    CHECK(par->loadFile("localhost:24680/Chunked.txt", ss));
    CHECK(ss.str() == strCheck);

    while (httptask.isRunning()) {
      Util::sleep(1);
    }
    Archive::free(par);
  }

  UninitializeSocket();
  UninitializeThreadPool();
}

// end of TestArchive.cpp
