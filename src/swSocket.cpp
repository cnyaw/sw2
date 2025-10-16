
//
//  TCP/IP network [Stream layer]
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/06/05 Waync created.
//

#include <time.h>

#include <algorithm>

#if defined(WIN32) || defined(_WIN32_WCE)
# include <windows.h>
# include <winsock.h>
# if defined(_MSC_VER)
#   pragma comment(lib, "Ws2_32")
# endif
#elif defined(_linux_)
# include <errno.h>
# include <netdb.h>
# include <sys/socket.h>
# include <sys/ioctl.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <unistd.h>
#endif

#include "swSocket.h"
#include "swStageStack.h"
#include "swUtil.h"

namespace sw2 {

namespace impl {

//
// Portable defines.
//

#if defined(WIN32)
# define errorno              (::WSAGetLastError())
# define SOCKET_EINTR         WSAEINTR
# define SOCKET_EINPROGRESS   WSAEWOULDBLOCK
# define SOCKET_EWOULDBLOCK   WSAEWOULDBLOCK
# define SOCKET_EAGAIN        WSAEWOULDBLOCK
# define socklen_t int
#elif defined(_linux_)
# define errorno              errno
# define SOCKET               int
# define INVALID_SOCKET       (-1)
# define SOCKET_ERROR         (-1)
# define SOCKET_EINTR         EINTR
# define SOCKET_EINPROGRESS   EINPROGRESS
# define SOCKET_EAGAIN        EAGAIN
# define SOCKET_EWOULDBLOCK   EWOULDBLOCK
# define closesocket(s)       close((s))
# define ioctlsocket(s,a,b)   ioctl((s),(a),(b))
#endif

//
// Constants.
//

#define TIMEOUT_DISCONNECTING 10        // Disconnecting phase timeout, second.
#define MAX_PACKET_BUFFER_SIZE 512      // Max buffer size, bytes.
#define MAX_TRIGGER_READ_SIZE 2048      // Max data size will be read in each trigger process, bytes.
#define MAX_TRIGGER_WIRTE_SIZE 2048     // Max data size will be written in each trigger process, bytes.

//
// Packet buffer.
//

struct implSocketPacketBuffer
{
  int len;                              // Data length.
  int offset;                           // Offset of buffer for 1st byte to send.
  uchar buff[MAX_PACKET_BUFFER_SIZE];   // Stream buffer.
  struct implSocketPacketBuffer* pNext; // Next block.
};

//
// Implementation.
//

int inet_aton_i(char const *cp, struct in_addr *pin)
{
  int rc = ::inet_addr(cp);
  if (-1 == rc && ::strcmp(cp, "255.255.255.255")) {
    return 0;
  }

  pin->s_addr = rc;

  return 1;
}

bool setAddress_i(std::string const& addr, struct sockaddr_in* sa)
{
  assert(sa);

  ::memset(sa, 0, sizeof(struct sockaddr_in));
  sa->sin_family = AF_INET;

  size_t pos = addr.find(':');
  if (std::string::npos == pos) {       // Port only.
    sa->sin_addr.s_addr = htonl(INADDR_ANY);
    sa->sin_port = htons(::atoi(addr.c_str()));
    return true;
  }

  //
  // IP:port.
  //

  std::string ip = addr.substr(0, pos);

  if (!inet_aton_i(ip.c_str(), &sa->sin_addr)) {
    struct hostent *h = ::gethostbyname(ip.c_str());
    if (0 == h) {                     // Unknown host.
      SW2_TRACE_ERROR("Unknown host name '%s'.", ip.c_str());
      return false;
    }
    sa->sin_addr = *(struct in_addr*)h->h_addr;
  }

  sa->sin_port = htons(::atoi(addr.c_str() + pos + 1));

  return true;
}

SOCKET createSock(std::string const& addr, struct sockaddr_in &sa)
{
  //
  // Create new socket.
  //

  SOCKET s = ::socket(AF_INET, SOCK_STREAM, 0);
  if (INVALID_SOCKET == s) {
    SW2_TRACE_ERROR("Create new socket failed.");
    return INVALID_SOCKET;
  }

  //
  // Set non-block I/O.
  //

  unsigned long v = 1;
  if (SOCKET_ERROR == ioctlsocket(s, FIONBIO, &v)) {
    SW2_TRACE_ERROR("Set non-block i/o failed.");
    closesocket(s);
    return INVALID_SOCKET;
  }

  //
  // Setup sock address.
  //

  if (!setAddress_i(addr, &sa)) {
    closesocket(s);
    return INVALID_SOCKET;
  }

  return s;
}

class implSocketBase
{
public:

  implSocketBase() :
    m_state(CS_DISCONNECTED),
    m_socket(INVALID_SOCKET),
    m_pSvrNetStats(0),
    m_pFreeBuff(0),
    m_pBuff(0),
    m_pBuffLast(0)
  {
    memset(&m_netStats, 0, sizeof(SocketClientStats));
    m_trigger.initialize(this, &implSocketBase::stageDisconnected);
  }

  virtual ~implSocketBase()
  {
    //
    // Release used block(s) if necessary.
    //

    if (m_pBuff) {
      assert(m_pBuffLast && 0 == m_pBuffLast->pNext);
      m_pBuffLast->pNext = m_pFreeBuff;
      m_pFreeBuff = m_pBuff;
      m_pBuff = m_pBuffLast = 0;
    }

    //
    // Free buffers.
    //

    while (m_pFreeBuff) {
      implSocketPacketBuffer* p = m_pFreeBuff;
      m_pFreeBuff = m_pFreeBuff->pNext;
      delete p;
    }
  }

  unsigned long long getBytesSendBuff() const
  {
    unsigned long long s = 0;
    implSocketPacketBuffer *p = m_pBuff;
    while (p) {
      s += p->len - p->offset;
      p = p->pNext;
    }
    return s;
  }

  bool connect(std::string const& svrAddr)
  {
    assert(CS_DISCONNECTED == m_state);
    if (CS_DISCONNECTED != m_state) {
      return false;
    }

    //
    // Create new socket.
    //

    struct sockaddr_in sa;
    SOCKET s = createSock(svrAddr, sa);
    if (INVALID_SOCKET == s) {
      return false;
    }

    char addr[128];
    ::sprintf(addr, "%s:%hu", ::inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
    m_addr = addr;

    //
    // Connect.
    //

    if (SOCKET_ERROR == ::connect(s, (const sockaddr*)&sa, sizeof(sa))) {

      if (SOCKET_EINPROGRESS != errorno) { // Something wrong.
        SW2_TRACE_ERROR("Something is wrong when connect.");
        closesocket(s);
        return false;
      }

      m_socket = s;
      m_trigger.popAndPush(&implSocketBase::stageConnecting);

      return true;
    }

    //
    // OOPS, connected immediately.
    //

    m_socket = s;
    m_trigger.popAndPush(&implSocketBase::stageConnected);

    return true;
  }

  void disconnect_i()
  {
    switch (m_state)
    {
    case CS_CONNECTED:
      m_trigger.popAndPush(&implSocketBase::stageDisconnecting1);
      break;

    case CS_CONNECTING:
      m_trigger.popAndPush(&implSocketBase::stageDisconnected);
      break;

    case CS_DISCONNECTED:
    case CS_DISCONNECTING:
      break;
    }
  }

  void doDisconnect()
  {
    //
    // Release used block(s) if any.
    //

    if (m_pBuff) {
      assert(m_pBuffLast && 0 == m_pBuffLast->pNext);
      m_pBuffLast->pNext = m_pFreeBuff;
      m_pFreeBuff = m_pBuff;
      m_pBuff = m_pBuffLast = 0;
    }

    //
    // Disconnect.
    //

    if (INVALID_SOCKET != m_socket) {
      closesocket(m_socket);
      m_socket = INVALID_SOCKET;
      if (CS_CONNECTED == m_state || CS_DISCONNECTING == m_state) {
        m_state = CS_DISCONNECTED;
        onDisconnected();               // Notify disconnected.
      }
    }

    m_state = CS_DISCONNECTED;
  }

  bool send_i(int len, void const* pStream)
  {
    assert(pStream);

    if (CS_CONNECTED != m_state) {
      return false;
    }

    uchar* p = (uchar*)pStream;
    implSocketPacketBuffer *pBuff = 0, *pHead = 0, *pLast = 0;

    int lenBuffLast = 0;
    if (m_pBuffLast && MAX_PACKET_BUFFER_SIZE > m_pBuffLast->len) {

      int alen = (std::min)((int)MAX_PACKET_BUFFER_SIZE - m_pBuffLast->len, len);
      ::memcpy(m_pBuffLast->buff + m_pBuffLast->len, p, alen);

      //
      // Update pointers.
      //

      len -= alen;
      p += alen;

      m_pBuffLast->len += alen;
      pLast = m_pBuffLast;
    }

    while (0 < len) {

      //
      // Allocate packet buffer.
      //

      if (m_pFreeBuff) {
        pBuff = m_pFreeBuff;
        m_pFreeBuff = m_pFreeBuff->pNext;
      } else {
        pBuff = new implSocketPacketBuffer;
      }

      if (0 == pBuff) {

        SW2_TRACE_ERROR("Send stream, out of memory.");

        if (0 != pHead) {               // Release allocated buffer(s).
          pLast->pNext = m_pFreeBuff;
          m_pFreeBuff = pHead;
        }

        //
        // Recover previous length of m_pBuffLast.
        //

        if (m_pBuffLast && 0 != lenBuffLast) {
          m_pBuffLast->len = lenBuffLast;
        }

        return false;
      }

      if (0 != pLast) {
        pLast->pNext = pBuff;
      }

      pLast = pBuff;

      if (0 == pHead) {
        pHead = pBuff;
      }

      //
      // Fill packet buffer.
      //

      pBuff->pNext = 0;
      pBuff->offset = 0;
      pBuff->len = (std::min)((int)MAX_PACKET_BUFFER_SIZE, len);
      ::memcpy(pBuff->buff, p, pBuff->len);

      //
      // Update pointers.
      //

      len -= pBuff->len;
      p += pBuff->len;
    }

    //
    // Link queued buffer(s).
    //

    if (0 != m_pBuffLast) {
      m_pBuffLast->pNext = pHead;
    }

    m_pBuffLast = pLast;

    if (0 == m_pBuff) {
      m_pBuff = pHead;
    }

    return true;
  }

  //
  // Connection phase.
  //

  int processSendData()
  {
    int n = send(m_socket, (const char*)m_pBuff->buff + m_pBuff->offset, (int)(m_pBuff->len - m_pBuff->offset), 0);
    if (0 < n) {
      m_pBuff->offset += n;
      if (m_pBuff->offset >= m_pBuff->len) { // Block sent completely?

        //
        // Release buffer.
        //

        implSocketPacketBuffer* p = m_pBuff;
        m_pBuff = m_pBuff->pNext;

        if (0 == m_pBuff) {
          m_pBuffLast = 0;
        }

        p->pNext = m_pFreeBuff;
        m_pFreeBuff = p;
      }

      //
      // Statistics.
      //

      m_netStats.bytesSent += (uint)n;
      if (m_pSvrNetStats) {
        m_pSvrNetStats->bytesSent += (uint)n;
      }
    }

    return n;
  }

  bool phaseConnected()
  {
    //
    // Return false to disconnect if things are not expected.
    //

    assert(CS_CONNECTED == m_state);

    //
    // Process receive data.
    //

    int n;
    uchar buff[MAX_TRIGGER_READ_SIZE];  // Receive data buffer.

    if (0 == (n = ::recv(m_socket, (char*)buff, MAX_TRIGGER_READ_SIZE, 0))) {

      //
      // FIN received, disconnected normally.
      //

      return false;
    }

    if (SOCKET_ERROR == n && SOCKET_EWOULDBLOCK != errorno && SOCKET_EINTR != errorno) {

      //
      // RST received or something wrong.
      //

      return false;
    }

    //
    // Something received.
    //

    if (0 < n) {
      m_netStats.bytesRecv += (uint)n;
      if (m_pSvrNetStats) {
        m_pSvrNetStats->bytesRecv += (uint)n;
      }
      onStreamReady(n, buff);
    }

    //
    // Process send data.
    //

    int byteSent = 0;

    while (0 != m_pBuff) {

      n = processSendData();
      if (0 < n) {

        //
        // Send continuously until no data or reach flow upper or error occur.
        //

        byteSent += n;
        if (MAX_TRIGGER_WIRTE_SIZE <= byteSent) {
          break;
        }

        continue;
      }

      if (SOCKET_ERROR == n && SOCKET_EWOULDBLOCK != errorno && SOCKET_EINTR != errorno) {

        //
        // Received RST or something wrong.
        //

        return false;
      }

      //
      // EWOULDBLOCK or EINTR.
      //

      break;
    }

    return true;
  }

  bool phaseDisconnect1()
  {
    //
    // Before disconnect, send queued data(s).
    //

    while (0 != m_pBuff) {

      int n = processSendData();
      if (0 < n) {

        //
        // Send continuously until no data or error occur.
        //

        continue;
      }

      if (SOCKET_ERROR == n && SOCKET_EWOULDBLOCK != errorno && SOCKET_EINTR != errorno) {

        //
        // RST received or something wrong.
        //

        break;
      }

      //
      // EWOULDBLOCK or EINTR.
      //

    }

    //
    // Shutdown, send FIN.
    //

    if (SOCKET_ERROR == shutdown(m_socket, 1)) {
      closesocket(m_socket);
      m_socket = INVALID_SOCKET;
      return false;
    }

    return true;
  }

  bool phaseDisconnect2()
  {
    int n;
    char buf[MAX_TRIGGER_READ_SIZE];

    //
    // Checking FIN, RST, timeout or errors.
    //

    if (0 == (n = ::recv(m_socket, buf, MAX_TRIGGER_READ_SIZE, 0))) {

      //
      // Disconnected normally.
      //

      return true;
    }

    if (SOCKET_ERROR == n && SOCKET_EWOULDBLOCK != errorno && SOCKET_EINTR != errorno) {

      //
      // RST received or something wrong.
      //

      return true;
    }

    //
    // Something received, but discard.
    //

    if (0 < n) {
      m_netStats.bytesRecv += (uint)n;
      if (m_pSvrNetStats) {
        m_pSvrNetStats->bytesRecv += (uint)n;
      }
    }

    //
    // Check timeout timer.
    //

    if (m_lastProcessTimeout.isExpired()) {

      //
      // Disconnected, timeout.
      //

      return true;
    }

    return false;
  }

  //
  // Connection stages.
  //

  void stageConnected(int state, uint_ptr)
  {
    if (JOIN == state) {
      m_state = CS_CONNECTED;
      ::memset(&m_netStats, 0, sizeof(SocketClientStats));
      m_netStats.startTime = ::time(0);
      onConnected();
    }

    if (TRIGGER == state) {
      if (!phaseConnected()) {
        m_trigger.popAndPush(&implSocketBase::stageDisconnected);
      }
    }
  }

  void stageConnecting(int state, uint_ptr)
  {
    if (JOIN == state) {
      m_state = CS_CONNECTING;
    }

    if (TRIGGER == state) {
      int n;
      fd_set wset;
      FD_ZERO(&wset);
      FD_SET(m_socket, &wset);
#if defined(WIN32)
      fd_set eset;
      FD_ZERO(&eset);
      FD_SET(m_socket, &eset);
#endif
      struct timeval tval;
      tval.tv_sec = tval.tv_usec = 0;

#if defined(WIN32)
      if ((n = ::select((int)(m_socket + 1), 0, &wset, &eset, &tval)) <= 0) {
#else
      if ((n = ::select((int)(m_socket + 1), 0, &wset, 0, &tval)) <= 0) {
#endif

        //
        // Select failed or timeout.
        //

        if (0 == n || errorno == SOCKET_EINTR) {

          //
          // Still pending.
          //

        } else {

          //
          // Select failed.
          //

          m_trigger.popAndPush(&implSocketBase::stageDisconnected);
        }

      } else {

        //
        // Select success, check is connected or error occur.
        //

#if defined(WIN32)
        int eset_set = FD_ISSET(m_socket, &eset);
        if (FD_ISSET(m_socket, &wset) || eset_set) {
          if (eset_set) {
#else
        if (FD_ISSET(m_socket, &wset)) {
          int error = 0, len = sizeof(error);
          if (SOCKET_ERROR == ::getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char*)&error, (socklen_t*)&len) || 0 != error) {
#endif

            //
            // Connect failed.
            //

            m_trigger.popAndPush(&implSocketBase::stageDisconnected);

          } else {                      // Connected.
            m_trigger.popAndPush(&implSocketBase::stageConnected);
          }
        } else {                        // Something wrong.
          m_trigger.popAndPush(&implSocketBase::stageDisconnected);
        }
      }
    }
  }

  void stageDisconnected(int state, uint_ptr)
  {
    if (JOIN == state) {
      doDisconnect();
    }
  }

  void stageDisconnecting1(int state, uint_ptr)
  {
    if (JOIN == state) {
      m_state = CS_DISCONNECTING;
    }

    if (TRIGGER == state) {
      if (phaseDisconnect1()) {
        m_trigger.popAndPush(&implSocketBase::stageDisconnecting2);
      } else {
        m_trigger.popAndPush(&implSocketBase::stageDisconnected);
      }
    }
  }

  void stageDisconnecting2(int state, uint_ptr)
  {
    if (JOIN == state) {
      m_lastProcessTimeout.setTimeout(1000 * TIMEOUT_DISCONNECTING);
    }

    if (TRIGGER == state) {
      if (phaseDisconnect2()) {
        m_trigger.popAndPush(&implSocketBase::stageDisconnected);
      }
    }
  }

  //
  // Notification.
  //

  virtual void onBeforeCheckNewClientReady()=0;
  virtual void onConnected()=0;
  virtual void onDisconnected()=0;
  virtual void onStreamReady(int len, void const* pStream)=0;

public:

  int m_state;                          // Current connection state.
  SOCKET m_socket;                      // Socket ID.
  std::string m_addr;                   // Host address.
  SocketClientStats m_netStats;         // Net stats.
  SocketServerStats* m_pSvrNetStats;

  implSocketPacketBuffer* m_pFreeBuff;  // Free list of packet buffer.

  implSocketPacketBuffer *m_pBuff, *m_pBuffLast; // Buffer list for queued data.
  TimeoutTimer m_lastProcessTimeout;    // Since last process trigger.
  StageStack<implSocketBase> m_trigger;
};

class implSocketClient : public implSocketBase, public SocketClient
{
public:

  explicit implSocketClient(SocketClientCallback* pCallback) : m_pCallback(pCallback)
  {
    SocketClient::userData = 0;
  }

  virtual ~implSocketClient()
  {
  }

  void destroy()
  {
    if (CS_DISCONNECTED != m_state) {
      disconnect();
      while (CS_DISCONNECTED != m_state) {
        trigger();
      }
    }
  }

  //
  // SocketClient.
  //

  virtual bool connect(std::string const& svrAddr)
  {
    return implSocketBase::connect(svrAddr);
  }

  virtual void disconnect()
  {
    implSocketBase::disconnect_i();
  }

  virtual int getConnectionState() const
  {
    return implSocketBase::m_state;
  }

  virtual std::string getAddr() const
  {
    return implSocketBase::m_addr;
  }

  virtual SocketClientStats getNetStats() const
  {
    SocketClientStats s = m_netStats;
    s.upTime = (time_t)::difftime(::time(0), s.startTime);
    s.bytesBuff = getBytesSendBuff();
    return s;
  }

  virtual bool send(int len, void const* pStream)
  {
    return implSocketBase::send_i(len, pStream);
  }

  virtual void trigger()
  {
    implSocketBase::m_trigger.trigger();
  }

  //
  // Notification.
  //

  virtual void onBeforeCheckNewClientReady()
  {
  }

  virtual void onConnected()
  {
    m_pCallback->onSocketServerReady(this);
  }

  virtual void onDisconnected()
  {
    m_pCallback->onSocketServerLeave(this);
  }

  virtual void onStreamReady(int len, void const* pStream)
  {
    m_pCallback->onSocketStreamReady(this, len, pStream);
  }

public:

  SocketClientCallback* m_pCallback;
};

class implSocketConnection : public implSocketBase, public SocketConnection
{
public:

  implSocketConnection() : m_pServer(0), m_pCallback(0), m_pNext(0), m_bAccept(false) {}

  //
  // SocketConnection.
  //

  virtual void disconnect()
  {
    implSocketBase::disconnect_i();
  }

  virtual int getConnectionState() const
  {
    return implSocketBase::m_state;
  }

  virtual std::string getAddr() const
  {
    return implSocketBase::m_addr;
  }

  virtual SocketClientStats getNetStats() const
  {
    SocketClientStats s = m_netStats;
    s.upTime = (time_t)::difftime(::time(0), s.startTime);
    s.bytesBuff = getBytesSendBuff();
    return s;
  }

  virtual bool send(int len, void const* pStream)
  {
    return implSocketBase::send_i(len, pStream);
  }

  //
  // Notification.
  //

  virtual void onBeforeCheckNewClientReady()
  {
  }

  virtual void onConnected()
  {
  }

  virtual void onDisconnected()
  {
    assert(m_pCallback);
    if (m_bAccept) {
      m_pCallback->onSocketClientLeave(m_pServer, (SocketConnection*)this);
    }
  }

  virtual void onStreamReady(int len, void const* pStream)
  {
    assert(m_pCallback);
    m_pCallback->onSocketStreamReady(m_pServer, (SocketConnection*)this, len, pStream);
  }

public:

  SocketServer *m_pServer;
  SocketServerCallback* m_pCallback;
  implSocketConnection* m_pNext;
  bool m_bAccept;
};

class implWebSocketConnection : public implSocketBase, public SocketConnection
{
public:

  implWebSocketConnection() : m_pServer(0), m_pCallback(0), m_pNext(0), m_bAccept(false), m_hasUpgrade(false) {}

  //
  // SocketConnection.
  //

  virtual void disconnect()
  {
    implSocketBase::disconnect_i();
  }

  virtual int getConnectionState() const
  {
    return implSocketBase::m_state;
  }

  virtual std::string getAddr() const
  {
    return implSocketBase::m_addr;
  }

  virtual SocketClientStats getNetStats() const
  {
    SocketClientStats s = m_netStats;
    s.upTime = (time_t)::difftime(::time(0), s.startTime);
    s.bytesBuff = getBytesSendBuff();
    return s;
  }

  virtual bool send(int lenStream, void const* pStream)
  {
    if (!m_hasUpgrade) {
      m_cache.append((const char*)pStream, lenStream);
      return true;
    }

    unsigned char buff[10] = {0x82};    // Binary data.
    int64 len = lenStream;
    if (125 >= len) {
      buff[1] = (unsigned char)(len & 0xff);
      len = 2;
    } else if (65535 >= len) {
      buff[1] = 126;
      buff[2] = (unsigned char)((len >> 8) & 0xff);
      buff[3] = (unsigned char)(len & 0xff);
      len = 4;
    } else {
      buff[1] = 127;
      buff[2] = (unsigned char)((len >> 56) & 0xff);
      buff[3] = (unsigned char)((len >> 48) & 0xff);
      buff[4] = (unsigned char)((len >> 40) & 0xff);
      buff[5] = (unsigned char)((len >> 32) & 0xff);
      buff[6] = (unsigned char)((len >> 24) & 0xff);
      buff[7] = (unsigned char)((len >> 16) & 0xff);
      buff[8] = (unsigned char)((len >> 8) & 0xff);
      buff[9] = (unsigned char)(len & 0xff);
      len = 10;
    }

    return implSocketBase::send_i((int)len, buff) && implSocketBase::send_i(lenStream, pStream);
  }

  //
  // Notification.
  //

  virtual void onBeforeCheckNewClientReady()
  {
    m_stream.clear();
    m_cache.clear();
    m_hasUpgrade = false;
  }

  virtual void onConnected()
  {
  }

  virtual void onDisconnected()
  {
    assert(m_pCallback);
    if (m_bAccept) {
      m_pCallback->onSocketClientLeave(m_pServer, (SocketConnection*)this);
    }
  }

  virtual void onStreamReady(int len, void const* pStream)
  {
    assert(m_pCallback);
    m_stream.append((const char*)pStream, len);
    if (m_hasUpgrade) {
      while (websockReadFrame()) {
        // NOP.
      }
    } else {
      webSockUpgrade();
    }
  }

  //
  // WebSocket support.
  //

  std::string getConnectionInfo(const std::string& request)
  {
    std::string connStr;
    size_t connIndex = request.find("Connection:");
    if (std::string::npos != connIndex) {
      size_t lineEnd = request.find("\r\n", connIndex);
      if (std::string::npos != lineEnd) {
        connStr = request.substr(connIndex, lineEnd - connIndex);
      } else {
        connStr = request.substr(connIndex);
      }
      connStr.erase(0, connStr.find_first_not_of(" \t"));
      connStr.erase(connStr.find_last_not_of(" \t") + 1);
    }
    return connStr;
  }

  std::string getWebsockAcceptKey(const std::string& req)
  {
    if (req.find("Upgrade: websocket") == std::string::npos ||
        req.find("Connection:") == std::string::npos ||
        req.find("Sec-WebSocket-Version: 13") == std::string::npos ||
        req.find("Sec-WebSocket-Protocol: sw2") == std::string::npos) {
      return "";
    }

    size_t keyIndex = req.find("Sec-WebSocket-Key"); // Extract Sec-WebSocket-Key.
    if (keyIndex == std::string::npos) {
      return "";
    }

    size_t keyStart = keyIndex + std::string("Sec-WebSocket-Key:").length();
    size_t keyEnd = req.find("\r\n", keyStart);
    std::string key = req.substr(keyStart, keyEnd - keyStart);
    key.erase(0, key.find_first_not_of(" \n\r\t")); // Trim leading whitespace.
    key.erase(key.find_last_not_of(" \n\r\t") + 1); // Trim trailing whitespace.

    return phpWebSockHash(key);
  }

  std::string phpWebSockHash(const std::string &key)
  {
    std::string script = "$combined=$key.'258EAFA5-E914-47DA-95CA-C5AB0DC85B11';$hash=sha1($combined);$base64Hash=base64_encode(hex2bin($hash));file_put_contents('hash.txt',$base64Hash);";
    std::string cmd = "php -r \"$key='" + key + "';" + script + "\"";
    system(cmd.c_str());
    std::string hash;
    Util::loadFileContent("hash.txt", hash);
    return hash;
  }

  std::string readRequestHeader() {
    std::string request;
    size_t pos = 0;
    size_t next_line_end;
    while (true) {
      next_line_end = m_stream.find("\r\n", pos);
      if (std::string::npos == next_line_end) {
        break;
      }
      std::string line = m_stream.substr(pos, next_line_end - pos);
      request += line + "\r\n";
      pos = next_line_end + 2;
      if (line.empty()) {
        break;
      }
    }
    return request;
  }

  bool websockReadFrame()
  {
    //
    // Check message header.
    //

    const char *p = m_stream.data();
    size_t req_len = 2;
    if (m_stream.size() < req_len) {    // p[0]=0x81 for text, 0x82 for binary.
      return false;
    }

    //
    // Read message length.
    //

    int len = (p[1] & 0xff) - 128;
    p += 2;

    if (126 == len) {                   // 16-bits length.
      req_len += 2;
      if (m_stream.size() < req_len) {
        return false;
      }
      len = (p[0] & 0xff) | ((p[1] & 0xff) << 8);
      p += 2;
    } else if (127 == len) {            // 64-bits length.
      req_len += 8;
      if (m_stream.size() < req_len) {
        return false;
      }
      len = p[0] & 0xff;
      for (int i = 1; i < 8; i++) {
        len |= ((p[i] & 0xff) << (8 * i));
      }
      p += 8;
    }

    //
    // Read decode key.
    //

    req_len += 4;
    if (m_stream.size() < req_len) {
      return false;
    }

    const char* pKey = p;
    p += 4;

    //
    // Decode message.
    //

    req_len += len;
    if (m_stream.size() < req_len) {
      return false;
    }

    char *pMsg = (char*)p;
    for (int i = 0; i < len; i++) {
      pMsg[i] ^= (unsigned char)pKey[i % 4];
    }

    m_pCallback->onSocketStreamReady(m_pServer, (SocketConnection*)this, len, pMsg);

    m_stream.erase(0, req_len);         // Removed handled messages.

    return true;
  }

  void webSockUpgrade()
  {
    //
    // Upgrade HTTP connection to WebSocket connection.
    //

    std::string req = readRequestHeader();
    if (std::string::npos == req.find("\r\n\r\n")) {
      return;
    }

    m_stream.erase(0, m_stream.find("\r\n\r\n") + 4); // Removed handled messages.

    std::string keyAccept = getWebsockAcceptKey(req);
    if ("" == keyAccept) {
      return;
    }

    std::string conn = getConnectionInfo(req);

    std::string resp = std::string("HTTP/1.1 101 Switching Protocols\r\n") +
             "Upgrade: websocket\r\n" +
             conn + "\r\n" +
             "Sec-WebSocket-Accept: " + keyAccept + "\r\n" +
             "Sec-WebSocket-Protocol: sw2\r\n\r\n";

    if (implSocketBase::send_i((int)resp.size(), resp.c_str())) {
      m_hasUpgrade = true;
      if (!m_cache.empty()) {
        send((int)m_cache.size(), m_cache.c_str());
        m_cache.clear();
      }
    }
  }

public:

  WebSocketServer *m_pServer;
  SocketServerCallback* m_pCallback;
  implWebSocketConnection* m_pNext;
  bool m_bAccept, m_hasUpgrade;
  std::string m_stream;
  std::string m_cache;                  // Saved stream that send before connection is upgraded.
};

template<class ConnT, class BaseT>
class implSocketServer : public BaseT
{
public:

  explicit implSocketServer(SocketServerCallback* pCallback) :
    m_listen(INVALID_SOCKET),
    m_pClient(0),
    m_pFreeClient(0),
    m_pCallback(pCallback)
  {
    SocketServer::userData = 0;
    ::memset(&m_netStats, 0, sizeof(SocketServerStats));
  }

  virtual ~implSocketServer()
  {
  }

  void destroy()
  {
    shutdown();

    //
    // Disconnect all connected clients and wait done.
    //

    ConnT* p = m_pClient;
    while (p) {
      p->disconnect();
      p = (ConnT*)p->m_pNext;
    }

    while (m_pClient) {
      trigger();
    }

    //
    // Free allocated.
    //

    assert(0 == m_pClient);

    while (m_pFreeClient) {
      p = m_pFreeClient;
      m_pFreeClient = (ConnT*)m_pFreeClient->m_pNext;
      delete p;
    }

    m_pFreeClient = 0;
  }

  //
  // Implement SocketServer.
  //

  virtual SocketConnection* getFirstConnection() const
  {
    return (SocketConnection*)m_pClient;
  }

  virtual SocketConnection* getNextConnection(SocketConnection* pClient) const
  {
    if (0 == pClient) {
      return 0;
    }

    return (SocketConnection*)(((ConnT*)pClient)->m_pNext);
  }

  virtual SocketServerStats getNetStats() const
  {
    SocketServerStats s = m_netStats;
    s.upTime = (time_t)difftime(time(0), s.startTime);
    s.bytesBuff = 0;
    ConnT *client = m_pClient;
    while (client) {
      s.bytesBuff += client->getBytesSendBuff();
      client = client->m_pNext;
    }
    return s;
  }

  virtual void shutdown()
  {
    if (INVALID_SOCKET != m_listen) {
      closesocket(m_listen);
      m_listen = INVALID_SOCKET;
      m_pCallback->onSocketServerShutdown(this);
    }
  }

  virtual bool startup(std::string const& addr)
  {
    //
    // Re-startup.
    //

    shutdown();

    //
    // Create new socket.
    //

    struct sockaddr_in sa;
    SOCKET s = createSock(addr, sa);
    if (INVALID_SOCKET == s) {
      return false;
    }

    //
    // Bind.
    //

    if (SOCKET_ERROR == ::bind(s, (const sockaddr*)&sa, sizeof(sa))) {
      SW2_TRACE_ERROR("Bind failed.");
      closesocket(s);
      return false;
    }

    //
    // Start listen.
    //

    if (SOCKET_ERROR == ::listen(s, SOMAXCONN)) {
      SW2_TRACE_ERROR("Start to listen failed.");
      closesocket(s);
      return false;
    }

    m_listen = s;
    ::memset(&m_netStats, 0, sizeof(SocketServerStats));
    m_netStats.startTime = ::time(0);

    socklen_t len = sizeof(sa);
    if (SOCKET_ERROR != getsockname(s, (sockaddr*)&sa, &len)) {
      char addr[128];
      ::sprintf(addr, "%s:%hu", ::inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
      m_addr = addr;
    }

    //
    // Notify startup.
    //

    m_pCallback->onSocketServerStartup(this);

    return true;
  }

  virtual void trigger()
  {
    //
    // Checking new connection.
    //

    while (true) {

      //
      // Check new connection.
      //

      struct sockaddr_in sa;
      int len = sizeof(struct sockaddr_in);
      SOCKET s = ::accept(m_listen, (struct sockaddr*)&sa, (socklen_t*)&len);
      if (INVALID_SOCKET == s) {

        if (SOCKET_EINTR == errorno) {
          continue;
        }

        if (SOCKET_EAGAIN != errorno && SOCKET_EWOULDBLOCK != errorno) {

          //
          // Something is wrong.
          //

        }

        //
        // No new connection.
        //

        break;
      }

      //
      // New arrive, set non-block IO.
      //

      unsigned long v = 1;
      if (SOCKET_ERROR == ioctlsocket(s, FIONBIO, &v)) {
        SW2_TRACE_ERROR("New arrive, set non-block i/o failed.");
        closesocket(s);
        break;
      }

      //
      // Accept?
      //

      ConnT* pClient;
      if (m_pFreeClient) {
        pClient = m_pFreeClient;
        m_pFreeClient = (ConnT*)m_pFreeClient->m_pNext;
      } else {
        pClient = new ConnT;
      }

      if (0 == pClient) {               // Out of memory?
        SW2_TRACE_ERROR("New arrive, out of memory.");
        closesocket(s);
        continue;
      }

      //
      // Pre-init connection context.
      //

      char addr[128];
      ::sprintf(addr, "%s:%hu", ::inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));
      pClient->m_addr = addr;
      pClient->m_pCallback = m_pCallback;
      pClient->m_pServer = this;
      pClient->m_pSvrNetStats = &m_netStats;
      pClient->userData = 0;
      pClient->m_socket = s;
      pClient->m_state = CS_CONNECTED;

      pClient->m_pNext = m_pClient;     // Link.
      m_pClient = pClient;

      m_netStats.hits += 1;

      //
      // Accept this new connection?
      //

      pClient->onBeforeCheckNewClientReady();

      if (m_pCallback->onSocketNewClientReady(this, (SocketConnection*)pClient)) {
        m_netStats.currOnline += 1;
        m_netStats.maxOnline = (std::max)(m_netStats.maxOnline, m_netStats.currOnline);
        pClient->m_bAccept = true;
        pClient->m_trigger.popAndPush(&implSocketBase::stageConnected);
      } else {                          // Not allowed.
        pClient->m_bAccept = false;
        pClient->m_trigger.popAndPush(&implSocketBase::stageDisconnecting1);
      }
    }

    //
    // Trigger active client(s).
    //

    ConnT *client = m_pClient, *prev = 0;
    while (client) {

      client->m_trigger.trigger();

      if (CS_DISCONNECTED == client->m_state) { // Client leave, release it.

        ConnT* curr = client;
        client = (ConnT*)client->m_pNext;

        //
        // Update used list.
        //

        if (0 == prev) {
          m_pClient = (ConnT*)curr->m_pNext;
        } else {
          prev->m_pNext = curr->m_pNext;
        }

        //
        // Release to free list.
        //

        curr->m_pNext = m_pFreeClient;
        m_pFreeClient = curr;

        if (curr->m_bAccept) {
          m_netStats.currOnline -= 1;
        }

      } else {
        prev = client;
        client = (ConnT*)client->m_pNext;
      }
    }
  }

  virtual std::string getAddr() const
  {
    return m_addr;
  }

public:

  SOCKET m_listen;                      // Listening socket.
  std::string m_addr;                   // Server addr.
  SocketServerStats m_netStats;

  ConnT* m_pClient;                     // Active client(s).
  ConnT* m_pFreeClient;                 // Available client(s).

  SocketServerCallback* m_pCallback;
};

} // namespace impl

bool InitializeSocket()
{
#if defined(WIN32)
  WSADATA wsadata;
  ::WSAStartup(MAKEWORD(2,2), &wsadata);
#endif

  SW2_TRACE_MESSAGE("swSocket initialized.");
  return true;
}

void UninitializeSocket()
{
#if defined(WIN32)
  (void)::WSACleanup();
#endif

  SW2_TRACE_MESSAGE("swSocket uninitialized.");
}

SocketClient* SocketClient::alloc(SocketClientCallback* pCallback)
{
  assert(pCallback);
  return new impl::implSocketClient(pCallback);
}

void SocketClient::free(SocketClient* pClient)
{
  impl::implSocketClient *p = (impl::implSocketClient*)pClient;
  p->destroy();
  delete p;
}

typedef impl::implSocketServer<impl::implSocketConnection, SocketServer> implSocketServerT;

SocketServer* SocketServer::alloc(SocketServerCallback* pCallback)
{
  assert(pCallback);
  return new implSocketServerT(pCallback);
}

void SocketServer::free(SocketServer* pServer)
{
  implSocketServerT *p = (implSocketServerT*)pServer;
  p->destroy();
  delete p;
}

typedef impl::implSocketServer<impl::implWebSocketConnection, WebSocketServer> implWebSocketServerT;

WebSocketServer* WebSocketServer::alloc(SocketServerCallback* pCallback)
{
  assert(pCallback);
  return new implWebSocketServerT(pCallback);
}

void WebSocketServer::free(WebSocketServer* pServer)
{
  implWebSocketServerT *p = (implWebSocketServerT*)pServer;
  p->destroy();
  delete p;
}

} // namespace sw2

// end of swSocket.cpp
