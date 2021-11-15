
//
//  Archive unit test.
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/05/31 Waync created.
//

#include <fstream>
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

class HttpFileServer : public SocketServerCallback
{
public:
  SocketServer* m_pServer;
  std::string m_str;

  HttpFileServer()
  {
    m_pServer = SocketServer::alloc(this);
    if (m_pServer) {
      m_pServer->startup("24680");
    }
    std::ifstream ifs("./data/ThePoolOfTears.txt", std::ios_base::binary);
    std::stringstream ss;
    ss << ifs.rdbuf();
    ifs.close();
    m_str = ss.str();
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
    const char *HTTP_GET_FILE = "GET /ThePoolOfTears.txt";
    std::string buff((const char*)pStream, len);
    if (!strncmp(buff.c_str(), HTTP_GET_FILE, strlen(HTTP_GET_FILE))) {
      const char fmt[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nAccess-Control-Allow-Origin: *\r\nContent-Length: %d\r\n\r\n" ;
      std::string resp;
      resp.resize(strlen(fmt) + m_str.size() + 32);
      sprintf((char*)resp.c_str(), fmt, (int)m_str.size());
      std::string s = resp.c_str() + m_str;
      pClient->send((int)s.size(), s.data());
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
    if (Util::httpGet(name, resp)) {
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
    CHECK(par->addFileSystem(&fs));

    std::stringstream ss;
    CHECK(par->loadFile("localhost:24680/ThePoolOfTears.txt", ss));
    CHECK(ss.str() == svr.m_str);

    while (httptask.isRunning()) {
      Util::sleep(1);
    }
    Archive::free(par);
  }

  UninitializeSocket();
  UninitializeThreadPool();
}

// end of TestArchive.cpp
