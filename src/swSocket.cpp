
//
//  TCP/IP network [Stream layer]
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/06/05 Waync created.
//

#if defined(WIN32) || defined(_WIN32_WCE)
# define NOMINMAX
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
#define TRIGGER_PROCESS_FREQUENCY 8     // Default frequency of trigger process(send/recv), Hz.
#define MAX_TRIGGER_PROCESS_FREQUENCY 1000 // Max frequency of trigger process, means no limit.
#define MAX_PACKET_BUFFER_SIZE 256      // Max buffer size, bytes.
#define MAX_TRIGGER_READ_SIZE 1024      // Max data size will be read in each trigger process, bytes.
#define MAX_TRIGGER_WIRTE_SIZE 1024     // Max data size will be written in each trigger process, bytes.

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
      SW2_TRACE_ERROR("Unknown host name.");
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
  // Enable TCP_NODELAY.
  //

  if (SOCKET_ERROR == ::setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&v, sizeof(v))) {
    SW2_TRACE_ERROR("Set tcp no delay failed.");
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
    m_pSocketFreeBuff(0),
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
      m_pBuffLast->pNext = m_pSocketFreeBuff;
      m_pSocketFreeBuff = m_pBuff;
      m_pBuff = m_pBuffLast = 0;
    }

    //
    // Free buffers.
    //

    while (m_pSocketFreeBuff) {
      implSocketPacketBuffer* p = m_pSocketFreeBuff;
      m_pSocketFreeBuff = m_pSocketFreeBuff->pNext;
      delete p;
    }
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
      SW2_TRACE_ERROR("Create new socket failed.");
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
      m_pBuffLast->pNext = m_pSocketFreeBuff;
      m_pSocketFreeBuff = m_pBuff;
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

      int alen = std::min((int)MAX_PACKET_BUFFER_SIZE - m_pBuffLast->len, len);
      ::memcpy(m_pBuffLast->buff + m_pBuffLast->len, p, alen);

      //
      // Update pointers.
      //

      len -= alen;
      p += alen;

      m_pBuffLast->len += alen;
      pLast = m_pBuffLast;
    }

    while (len) {

      //
      // Allocate packet buffer.
      //

      if (m_pSocketFreeBuff) {
        pBuff = m_pSocketFreeBuff;
        m_pSocketFreeBuff = m_pSocketFreeBuff->pNext;
      } else {
        pBuff = new implSocketPacketBuffer;
      }

      if (0 == pBuff) {

        SW2_TRACE_ERROR("Send stream, out of memory.");

        if (0 != pHead) {               // Release allocated buffer(s).
          pLast->pNext = m_pSocketFreeBuff;
          m_pSocketFreeBuff = pHead;
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
      pBuff->len = std::min((int)MAX_PACKET_BUFFER_SIZE, len);
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

        p->pNext = m_pSocketFreeBuff;
        m_pSocketFreeBuff = p;
      }

      //
      // Statstics.
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
    // Control the trigger frequency.
    //

    if (MAX_TRIGGER_PROCESS_FREQUENCY > *m_pTriggerFreq) {
      if (!m_lastProcessTimeout.isExpired()) {
        return true;
      }
      m_lastProcessTimeout.setTimeout(1000 / *m_pTriggerFreq);
    }

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
      m_lastProcessTimeout.setTimeout(1000 / TRIGGER_PROCESS_FREQUENCY);
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

  virtual void onConnected()=0;
  virtual void onDisconnected()=0;
  virtual void onStreamReady(int len, void const* pStream)=0;

public:

  int m_state;                          // Current connection state.
  SOCKET m_socket;                      // Socket ID.
  std::string m_addr;                   // Host address.
  SocketClientStats m_netStats;         // Net stats.
  SocketServerStats* m_pSvrNetStats;
  int *m_pTriggerFreq;

  implSocketPacketBuffer* m_pSocketFreeBuff; // Free list of packet buffer.

  implSocketPacketBuffer *m_pBuff, *m_pBuffLast; // Buffer list for queued data.
  TimeoutTimer m_lastProcessTimeout;    // Since last process trigger.
  StageStack<implSocketBase> m_trigger;
};

class implSocketClient : public implSocketBase, public SocketClient
{
public:

  explicit implSocketClient(SocketClientCallback* pCallback) : m_TriggerFreq(TRIGGER_PROCESS_FREQUENCY), m_pCallback(pCallback)
  {
    SocketClient::userData = 0;
    implSocketBase::m_pTriggerFreq = &m_TriggerFreq;
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

  virtual SocketClientStats getNetStats()
  {
    m_netStats.upTime = (time_t)::difftime(::time(0), m_netStats.startTime);
    return m_netStats;
  }

  virtual bool send(int len, void const* pStream)
  {
    return implSocketBase::send_i(len, pStream);
  }

  virtual void trigger()
  {
    implSocketBase::m_trigger.trigger();
  }

  virtual int getTriggerFrequency() const
  {
    return m_TriggerFreq;
  }

  virtual void setTriggerFrequency(int freq)
  {
    m_TriggerFreq = std::min(MAX_TRIGGER_PROCESS_FREQUENCY, std::max(1, freq));
  }

  //
  // Notification.
  //

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

  int m_TriggerFreq;
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

  virtual SocketClientStats getNetStats()
  {
    m_netStats.upTime = (time_t)::difftime(::time(0), m_netStats.startTime);
    return m_netStats;
  }

  virtual bool send(int len, void const* pStream)
  {
    return implSocketBase::send_i(len, pStream);
  }

  //
  // Notification.
  //

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

class implSocketServer : public SocketServer
{
public:

  explicit implSocketServer(SocketServerCallback* pCallback) :
    m_listen(INVALID_SOCKET),
    m_pClient(0),
    m_pFreeClient(0),
    m_pCallback(pCallback),
    m_TriggerFreq(TRIGGER_PROCESS_FREQUENCY)
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

    implSocketConnection* p = m_pClient;
    while (p) {
      p->disconnect();
      p = (implSocketConnection*)p->m_pNext;
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
      m_pFreeClient = (implSocketConnection*)m_pFreeClient->m_pNext;
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

    return (SocketConnection*)(((implSocketConnection*)pClient)->m_pNext);
  }

  virtual SocketServerStats getNetStats()
  {
    m_netStats.upTime = (time_t)difftime(time(0), m_netStats.startTime);
    return m_netStats;
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
      SW2_TRACE_ERROR("Create new socket failed.");
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

    int len = sizeof(sa);
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
#if defined(_linux_)
      SOCKET s = ::accept(m_listen, (struct sockaddr*)&sa, (socklen_t*)&len);
#else
      SOCKET s = ::accept(m_listen, (struct sockaddr*)&sa, &len);
#endif
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
      // Enable TCP_NODELAY.
      //

      if (SOCKET_ERROR == ::setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char*)&v, sizeof(v))) {
        SW2_TRACE_ERROR("New arrive, set tcp no delay failed.");
        closesocket(s);
        break;
      }

      //
      // Accept?
      //

      implSocketConnection* pClient;
      if (m_pFreeClient) {
        pClient = m_pFreeClient;
        m_pFreeClient = (implSocketConnection*)m_pFreeClient->m_pNext;
      } else {
        pClient = new implSocketConnection;
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
      pClient->m_pTriggerFreq = &m_TriggerFreq;

      pClient->m_pNext = m_pClient;     // Link.
      m_pClient = pClient;

      m_netStats.hits += 1;

      //
      // Accept this new connection?
      //

      if (m_pCallback->onSocketNewClientReady(this, (SocketConnection*)pClient)) {
        m_netStats.currOnline += 1;
        m_netStats.maxOnline = std::max(m_netStats.maxOnline, m_netStats.currOnline);
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

    implSocketConnection *client = m_pClient, *prev = 0;
    while (client) {

      client->m_trigger.trigger();

      if (CS_DISCONNECTED == client->m_state) { // Client leave, release it.

        implSocketConnection* curr = client;
        client = (implSocketConnection*)client->m_pNext;

        //
        // Update used list.
        //

        if (0 == prev) {
          m_pClient = (implSocketConnection*)curr->m_pNext;
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
        client = (implSocketConnection*)client->m_pNext;
      }
    }
  }

  virtual std::string getAddr() const
  {
    return m_addr;
  }

  virtual int getTriggerFrequency() const
  {
    return m_TriggerFreq;
  }

  virtual void setTriggerFrequency(int freq)
  {
    m_TriggerFreq = std::min(MAX_TRIGGER_PROCESS_FREQUENCY, std::max(1, freq));
  }

public:

  SOCKET m_listen;                      // Listening socket.
  std::string m_addr;                   // Server addr.
  SocketServerStats m_netStats;

  implSocketConnection* m_pClient;      // Active client(s).
  implSocketConnection* m_pFreeClient;  // Available client(s).

  SocketServerCallback* m_pCallback;

  int m_TriggerFreq;
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

SocketServer* SocketServer::alloc(SocketServerCallback* pCallback)
{
  assert(pCallback);
  return new impl::implSocketServer(pCallback);
}

void SocketServer::free(SocketServer* pServer)
{
  impl::implSocketServer *p = (impl::implSocketServer*)pServer;
  p->destroy();
  delete p;
}

} // namespace sw2

// end of swSocket.cpp
