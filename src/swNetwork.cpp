
//
//  TCP/IP network [Packet layer]
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/08/02 Waync created.
//

#include <time.h>

#include <algorithm>

#include "swNetwork.h"
#include "swObjectPool.h"
#include "swUtil.h"

//
//  Packet format.
//
//  +---------------------------------------------------------------+
//  |                         H E A D E R (2 Bytes)                 |
//  +---------------------------------------------------------------+
//  | 4 bits flag | 2 bits packet type  | 10 bits packet length     |
//  +---------------------------------------------------------------+
//  15(high)                                                        0(low)
//
//  00-09(10) bits: Length of the packet.
//  10-11(02) bits: Type of the packet: see below.
//  12-15(04) bits: Misc flag.
//
//  MAX PACKET SIZE IS 1024 BYTES, MAX DATA SIZE IS:
//    (00) Stream: 2(stream beg) + n(stream) + 2(stream end)
//    (11) Keepalive: 2(header only)
//

namespace sw2 {

namespace impl {

//
// Constants.
//

#define MAX_CLIENT 4096                 // Max client connection of a single server.
#define TIMEOUT_KEEP_ALIVE 25           // Send a keep alive signal if there's no data sent after this interval, sec.
#define TIMEOUT_DEAD_CONNECTION 60      // Force disconnect if there's no data received after this interval, sec.
#define MAX_PACKET_BUFFER_SIZE 1024     // Max buffer size, bytes.
#define PACKET_HEADER_SIZE 2            // Size of packet header, bytes.

#define MAKE_PACKET_HEADER(len, type, flag) ((len) | ((type) << 10) | ((flag) << 12))

ushort const keepAlive = MAKE_PACKET_HEADER(0, 3, 0x0);
ushort const streamBeg = MAKE_PACKET_HEADER(0, 0, 0xc);
ushort const streamEnd = MAKE_PACKET_HEADER(0, 0, 0x8);

//
// Implementation.
//

class implNetworkBase
{
public:

  virtual ~implNetworkBase()
  {
  }

  bool isBadHeader(ushort header) const
  {
    if (keepAlive == header || streamBeg == header || streamEnd == header) {
      return false;
    }

    if (((header >> 12) & 0xf) == (m_packetRecv & 0xf)) {
      return false;
    }

    SW2_TRACE_ERROR("Bad header.");

    return true;
  }

  template<class T>
  bool handleStreamReady(T* t, int len, void const* pStream)
  {
    do {

      int l = std::min(MAX_PACKET_BUFFER_SIZE - m_buffLen, len);

      ::memcpy(m_buff + m_buffLen, pStream, l);
      m_buffLen += l;

      pStream = (uchar*)pStream + l;
      len -= l;

      uchar* p = m_buff;

      while (true) {

        if (PACKET_HEADER_SIZE > m_buffLen) {
          break;
        }

        //
        // Is bad header?
        //

        ushort header = (ushort)(uint)(p[1] << 8) | (uint)p[0]; // Get header.
        if (isBadHeader(header)) {
          return false;
        }

        //
        // Wait until receive whole packet data.
        //

        int lenPacket = header & 0x3ff; // # bytes, len of data only not include header.
        if (lenPacket + PACKET_HEADER_SIZE > m_buffLen) {
          break;
        }

        //
        // Process packet stream.
        //

        if (0 == lenPacket) {

          //
          // Process keep-alive or stream packet header.
          //

          if (streamBeg == header) {    // Stream start?
            m_ss = "";                  // Reset buffer.
          } else if (streamEnd == header) { // Stream end.
            onStreamReady_i((int)m_ss.length(), m_ss.data());
          } else if (keepAlive != header) {
            SW2_TRACE_ERROR("Invalid keep alive header.");
            return false;
          }

        } else {

          //
          // Process packet contents. Pre-store packet data to internal buffer.
          //

          switch ((header >> 10) & 0x3)
          {
          case 0:                       // Stream.
            m_packetRecv += 1;
            m_ss.append((char const*)p + PACKET_HEADER_SIZE, lenPacket);
            IncRecvPack();
            break;
          case 3:                       // Keep-alive signal.
            break;
          }
        }

        //
        // People destroy the connection in the event/stream ready callback.
        //

        if (CS_CONNECTED != t->getConnectionState()) {
          return false;
        }

        p += lenPacket + PACKET_HEADER_SIZE;
        assert(m_buffLen >= lenPacket + PACKET_HEADER_SIZE);
        m_buffLen -= lenPacket + PACKET_HEADER_SIZE;
      }

      if (m_buffLen) {
        ::memcpy(m_buff, p, m_buffLen);
      }

    } while (0 < len);

    //
    // Reset timeout timer.
    //

    m_deadConnectionTimeout.setTimeout(1000 * TIMEOUT_DEAD_CONNECTION);

    return true;
  }

  template<class T>
  bool send_i(T* t, char const *buff, int szBuff, int type, ushort beg, ushort end)
  {
    if (0 >= szBuff) {
      return false;
    }

    //
    // Stream start notification.
    //

    if (!t->send(2, (void*)&beg)) {
      return false;
    }

    //
    // Stream content.
    //

    char const* p = buff;
    int len = szBuff;

    while (0 < len) {
      int len2 = std::min(MAX_PACKET_BUFFER_SIZE - PACKET_HEADER_SIZE, len);
      uint header = MAKE_PACKET_HEADER(len2, type, m_packetSent & 0xf);
      if (!t->send(PACKET_HEADER_SIZE, &header) || !t->send(len2, p)) {
        return false;
      }
      m_packetSent += 1;
      IncSendPack();
      len -= len2;
      p += len2;
    }

    //
    // Stream end notification.
    //

    if (!t->send(2, (void*)&end)) {
      return false;
    }

    //
    // Reset timeout timer.
    //

    m_keepAliveTimeout.setTimeout(1000 * TIMEOUT_KEEP_ALIVE);

    return true;
  }

  template<class T>
  bool send_i(T* t, int len, void const* pStream)
  {
    //
    // Send stream raw data.
    //

    return send_i(t, (char*)pStream, len, 0, streamBeg, streamEnd);
  }

  template<class T>
  bool trigger_(T* t)
  {
    if (CS_CONNECTED != t->getConnectionState()) {
      return true;
    }

    //
    // Check dead connection timeout timer.
    //

    if (m_deadConnectionTimeout.isExpired()) {
      return false;
    }

    //
    // Check keep alive timeout timer, send immediately.
    //

    if (m_keepAliveTimeout.isExpired()) {
      if (!t->send(PACKET_HEADER_SIZE, (void*)&keepAlive)) {
        return false;
      }
      m_keepAliveTimeout.setTimeout(1000 * TIMEOUT_KEEP_ALIVE);
    }

    return true;
  }

  //
  // Callback.
  //

  virtual void onStreamReady_i(int len, void const* pStream)=0;
  virtual void IncRecvPack()=0;
  virtual void IncSendPack()=0;

public:

  int m_buffLen;                        // Buffered receive data length.
  uchar m_buff[MAX_PACKET_BUFFER_SIZE]; // Receive data buffer.

  std::string m_ss;                     // Stream buffer.

  TimeoutTimer m_deadConnectionTimeout; // Since last receive data.
  TimeoutTimer m_keepAliveTimeout;      // Since last send data.

  long m_packetSent;
  long m_packetRecv;
};

class implNetworkClient : public implNetworkBase, public NetworkClient, public SocketClientCallback
{
public:

  explicit implNetworkClient(NetworkClientCallback* pCallback) : m_pInterface(pCallback)
  {
    m_pClient = SocketClient::alloc(this);
    NetworkClient::userData = 0;
    m_packetSent = m_packetRecv = 0;
  }

  virtual ~implNetworkClient()
  {
  }

  void destroy()
  {
    if (CS_DISCONNECTED != getConnectionState()) {
      disconnect();
      while (CS_DISCONNECTED != getConnectionState()) {
        trigger();
#ifdef __EMSCRIPTEN__
        break;                          // FIXME: workaround to avoid blocking browser thread.
#endif
      }
    }
    SocketClient::free(m_pClient);
  }

  //
  // Implement SocketClientCallback.
  //

  virtual void onSocketServerLeave(SocketClient*)
  {
    m_pInterface->onNetworkServerLeave(this);
  }

  virtual void onSocketServerReady(SocketClient*)
  {
    m_buffLen = 0;
    m_deadConnectionTimeout.setTimeout(1000 * TIMEOUT_DEAD_CONNECTION);
    m_keepAliveTimeout.setTimeout(1000 * TIMEOUT_KEEP_ALIVE);
    m_pInterface->onNetworkServerReady(this);
    m_packetSent = m_packetRecv = 0;
  }

  virtual void onSocketStreamReady(SocketClient*, int len, void const* pStream)
  {
    if (!implNetworkBase::handleStreamReady(m_pClient, len, pStream)) {
      disconnect();
    }
  }

  //
  // Implement NetworkClient.
  //

  virtual bool connect(std::string const& svrAddr)
  {
    return m_pClient->connect(svrAddr);
  }

  virtual void disconnect()
  {
    m_pClient->disconnect();
  }

  virtual int getConnectionState() const
  {
    return m_pClient->getConnectionState();
  }

  virtual std::string getAddr() const
  {
    return m_pClient->getAddr();
  }

  virtual NetworkClientStats getNetStats() const
  {
    NetworkClientStats ns;

    *(SocketClientStats*)&ns = m_pClient->getNetStats();
    ns.packetsSent = m_packetSent;
    ns.packetsRecv = m_packetRecv;

    return ns;
  }

  virtual bool send(int len, void const* pStream)
  {
    return implNetworkBase::send_i(m_pClient, len, pStream);
  }

  virtual void trigger()
  {
    m_pClient->trigger();

    if (!implNetworkBase::trigger_(m_pClient)) {
      disconnect();
    }
  }

  //
  // Implement implNetworkBase.
  //

  virtual void onStreamReady_i(int len, void const* pStream)
  {
    m_pInterface->onNetworkStreamReady(this, len, pStream);
  }

  void IncRecvPack()
  {
  }

  virtual void IncSendPack()
  {
  }

public:

  SocketClient* m_pClient;
  NetworkClientCallback* m_pInterface;
};

class implNetworkConnection : public implNetworkBase, public NetworkConnection
{
public:

  virtual ~implNetworkConnection()
  {
  }

  void trigger()
  {
    if (!implNetworkBase::trigger_(m_pClient)) {
      disconnect();
    }
  }

  //
  // Implement NetworkConnection.
  //

  virtual void disconnect()
  {
    m_pClient->disconnect();
  }

  virtual int getConnectionState() const
  {
    return m_pClient->getConnectionState();
  }

  virtual std::string getAddr() const
  {
    return m_pClient->getAddr();
  }

  virtual NetworkClientStats getNetStats() const
  {
    NetworkClientStats ns;

    *(SocketClientStats*)&ns = m_pClient->getNetStats();
    ns.packetsSent = m_packetSent;
    ns.packetsRecv = m_packetRecv;

    return ns;
  }

  virtual bool send(int len, void const* pStream)
  {
    return implNetworkBase::send_i(m_pClient, len, pStream);
  }

  //
  // Implement implNetworkBase.
  //

  virtual void onStreamReady_i(int len, void const* pStream)
  {
    m_pInterface->onNetworkStreamReady(m_pServer, (NetworkConnection*)this, len, pStream);
  }

  virtual void IncRecvPack()
  {
    *m_svrPacketRecv += 1;
  }

  virtual void IncSendPack()
  {
    *m_svrPacketSent += 1;
  }

public:

  NetworkServer* m_pServer;
  SocketConnection* m_pClient;
  NetworkServerCallback* m_pInterface;
  long *m_svrPacketSent, *m_svrPacketRecv;
};

template<bool SupportWebSocket>
class implNetworkServer : public NetworkServer, public SocketServerCallback
{
public:

  explicit implNetworkServer(NetworkServerCallback* pCallback) : m_pInterface(pCallback)
  {
    NetworkServer::userData = 0;
    if (SupportWebSocket) {
      m_pServer = WebSocketServer::alloc(this);
    } else {
      m_pServer = SocketServer::alloc(this);
    }
    m_packetSent = m_packetRecv = 0;
  }

  virtual ~implNetworkServer()
  {
    SocketServer::free(m_pServer);
  }

  //
  // Implement SocketServerCallback.
  //

  virtual void onSocketClientLeave(SocketServer*, SocketConnection* pClient)
  {
    int id = (int)pClient->userData;
    m_pInterface->onNetworkClientLeave(this, (NetworkConnection*)&m_poolClient[id]);
    m_poolClient.free(id);
  }

  virtual bool onSocketNewClientReady(SocketServer*, SocketConnection* pNewClient)
  {
    int id = m_poolClient.alloc();
    if (-1 == id) {
      return false;
    }

    pNewClient->userData = (uint_ptr)id;

    implNetworkConnection& c = m_poolClient[id];
    c.userData = 0;
    c.m_buffLen = 0;
    c.m_deadConnectionTimeout.setTimeout(1000 * TIMEOUT_DEAD_CONNECTION);
    c.m_keepAliveTimeout.setTimeout(1000 * TIMEOUT_KEEP_ALIVE);
    c.m_pClient = pNewClient;
    c.m_pServer = this;
    c.m_pInterface = m_pInterface;
    c.m_packetSent = c.m_packetRecv = 0;
    c.m_svrPacketSent = &m_packetSent;
    c.m_svrPacketRecv = &m_packetRecv;

    //
    // Accept this new connection?
    //

    if (m_pInterface->onNetworkNewClientReady(this, (NetworkConnection*)&c)) {
      return true;
    }

    m_poolClient.free(id);

    return false;                       // Not accept.
  }

  virtual void onSocketServerShutdown(SocketServer*)
  {
    m_pInterface->onNetworkServerShutdown(this);
  }

  virtual void onSocketServerStartup(SocketServer*)
  {
    m_pInterface->onNetworkServerStartup(this);
  }

  virtual void onSocketStreamReady(SocketServer*, SocketConnection* pClient, int len, void const* pStream)
  {
    int id = (int)pClient->userData;
    implNetworkConnection &c = m_poolClient[id];
    if (!c.handleStreamReady(c.m_pClient, len, pStream)) {
      c.disconnect();
    }
  }

  //
  // Implement NetworkServer.
  //

  virtual NetworkConnection* getFirstConnection() const
  {
    int first = m_poolClient.first();
    if (-1 == first) {
      return 0;
    } else {
      return (NetworkConnection*)&m_poolClient[first];
    }
  }

  virtual NetworkConnection* getNextConnection(NetworkConnection* pClient) const
  {
    if (0 == pClient) {
      return 0;
    }

    int next = m_poolClient.next((int)((implNetworkConnection*)pClient)->m_pClient->userData);
    if (-1 == next) {
      return 0;
    } else {
      return (NetworkConnection*)&m_poolClient[next];
    }
  }

  virtual NetworkServerStats getNetStats() const
  {
    NetworkServerStats ns;

    *(SocketServerStats*)&ns = m_pServer->getNetStats();
    ns.packetsSent = m_packetSent;
    ns.packetsRecv = m_packetRecv;

    return ns;
  }

  virtual bool startup(std::string const& addr)
  {
    return m_pServer->startup(addr);
  }

  virtual void shutdown()
  {
    m_pServer->shutdown();
  }

  virtual void trigger()
  {
    m_pServer->trigger();

    for (int i = m_poolClient.first(); -1 != i; i = m_poolClient.next(i)) {
      m_poolClient[i].trigger();
    }
  }

  virtual std::string getAddr() const
  {
    return m_pServer->getAddr();
  }

public:

  ObjectPool<implNetworkConnection, MAX_CLIENT> m_poolClient;

  SocketServer* m_pServer;
  NetworkServerCallback* m_pInterface;

  long m_packetSent, m_packetRecv;
};

} // namespace impl

bool InitializeNetwork()
{
  if (!InitializeSocket()) {
    return false;
  }

  SW2_TRACE_MESSAGE("swNetwork initialized.");
  return true;
}

void UninitializeNetwork()
{
  SW2_TRACE_MESSAGE("swNetwork uninitialized.");
  UninitializeSocket();
}

NetworkClient* NetworkClient::alloc(NetworkClientCallback* pCallback)
{
  assert(pCallback);
  return new impl::implNetworkClient(pCallback);
}

void NetworkClient::free(NetworkClient* pClient)
{
  impl::implNetworkClient *p = (impl::implNetworkClient*)pClient;
  p->destroy();
  delete p;
}

NetworkServer* NetworkServer::alloc(NetworkServerCallback* pCallback)
{
  assert(pCallback);
  return new impl::implNetworkServer<false>(pCallback);
}

void NetworkServer::free(NetworkServer* pServer)
{
  delete (impl::implNetworkServer<false>*)pServer;
}

WebNetworkServer* WebNetworkServer::alloc(NetworkServerCallback* pCallback)
{
  assert(pCallback);
  return (WebNetworkServer*)new impl::implNetworkServer<true>(pCallback);
}

void WebNetworkServer::free(WebNetworkServer* pServer)
{
  delete (impl::implNetworkServer<true>*)pServer;
}

} // namespace sw2

// end of swNetwork.cpp
