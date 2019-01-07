
//
//  TCP/IP network [Packet layer]
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/08/02 Waync created.
//

#include <ctime>

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
//    (01) Large packet(>= LARGE_PACKET_SIZE bytes): 2(stream beg) + n(packet stream, flagbit=sn) + 2(stream end)
//    (10) Small packet(< LARGE_PACKET_SIZE bytes): 2(header, flagbit=sn) + n(stream:max1020)
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
#define MAX_PACKET_ID_SIZE_BITS 7
#define MAX_PACKET_ID_SIZE (1 << MAX_PACKET_ID_SIZE_BITS)
#define LARGE_PACKET_SIZE (1020 + 1)

#define MAKE_PACKET_HEADER(len, type, flag) ((len) | ((type) << 10) | ((flag) << 12))

ushort const keepAlive = MAKE_PACKET_HEADER(0, 3, 0x0);
ushort const streamBeg = MAKE_PACKET_HEADER(0, 0, 0xc);
ushort const streamEnd = MAKE_PACKET_HEADER(0, 0, 0x8);
ushort const packetEnd = MAKE_PACKET_HEADER(0, 1, 0x8);

//
// Implementation.
//

class implNetworkPacketRuntimeClass
{
public:

  struct Cache
  {
    NetworkPacket* pItem;
    Cache* pNext;
  };

  NetworkPacket::StaticCreatePacket pfCreateObj;
  Cache* pHead;

  implNetworkPacketRuntimeClass() : pfCreateObj(0), pHead(0)
  {
  }

  ~implNetworkPacketRuntimeClass()
  {
    while (pHead) {
      Cache* p = pHead;
      pHead = pHead->pNext;
      delete p->pItem, delete p;
    }

    pHead = 0;
  }

  Cache* allocObj()
  {
    if (pHead) {
      Cache* p = pHead;
      pHead = pHead->pNext;
      return p;
    }

    Cache* p = new Cache;
    if (p) {
      p->pNext = 0;
      p->pItem = pfCreateObj();
      if (p->pItem) {
        return p;
      }
      delete p;
    }

    return 0;
  }

  void freeObj(Cache* p)
  {
    assert(p);
    p->pNext = pHead;
    pHead = p;
  }
};

class implNetworkPacketFactory
{
  implNetworkPacketRuntimeClass fac[MAX_PACKET_ID_SIZE];

  implNetworkPacketFactory()
  {
    memset(fac, 0, sizeof (fac));
  }

public:

  static implNetworkPacketFactory& inst()
  {
    static implNetworkPacketFactory i;
    return i;
  }

  bool registerPacket(int id, NetworkPacket::StaticCreatePacket pf, char const* pname)
  {
    if (0 > id || MAX_PACKET_ID_SIZE <= id) {
      SW2_TRACE_ERROR("Packet [%s:%d] invalid ID.", pname, id);
      assert(0);
      return false;
    }

    if (0 != fac[id].pfCreateObj) {
      SW2_TRACE_ERROR("Packet [%s:%d] already registerd.", pname, id);
      assert(0);
      return false;
    }

    fac[id].pfCreateObj = pf;           // Insert and assign.

    return true;
  }

  bool readPacket(BitStream &bs, NetworkPacket** pp)
  {
    if (0 == pp) {
      return false;
    }

    *pp = 0;

    uint id;
    if (!(bs >> setBitCount(MAX_PACKET_ID_SIZE_BITS) >> id)) {
      return false;
    }

    implNetworkPacketRuntimeClass& rt = fac[id];
    if (0 == rt.pfCreateObj) {
      return false;
    }

    implNetworkPacketRuntimeClass::Cache* c = rt.allocObj();
    if (0 == c) {
      return false;
    }

    if (c->pItem->read(bs)) {
      *pp = c->pItem;
    } else {
      return false;
    }

    rt.freeObj(c);

    return 0 != *pp;
  }

  bool writePacket(BitStream &bs, NetworkPacket const& p) const
  {
    if (0 == fac[p.getId()].pfCreateObj) {
      return false;
    }

    //
    // Data format: ID packet(bit stream)
    //

    if (!(bs << setBitCount(MAX_PACKET_ID_SIZE_BITS) << (uint)p.getId())) {
      return false;
    }

    if (!p.write(bs)) {
      return false;
    }

    return true;
  }
};

class implNetworkBase
{
public:

  virtual ~implNetworkBase()
  {
  }

  bool isBadHeader(ushort header) const
  {
    if (keepAlive == header || streamBeg == header || streamEnd == header || packetEnd == header) {
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
            onStreamReady_(m_ss);
          } else if (packetEnd == header) { // Large packet end.

            //
            // A large packet is ready, read and notify it.
            //

            BitStream bs((char*)m_ss.data(), (int)m_ss.length());

            NetworkPacket* p = 0;
            if (!implNetworkPacketFactory::inst().readPacket(bs, &p)) {
              SW2_TRACE_ERROR("Read packet failed.");
              return false;
            }

            onPacketReady_(*p);

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
          case 1:                       // Large packet.
            m_packetRecv += 1;
            m_ss.append((char const*)p + PACKET_HEADER_SIZE, lenPacket);
            IncRecvPack();
            break;

          case 2:                       // Small packet.
            {
              //
              // A small packet is ready, read and notify it.
              //

              BitStream bs((char*)p + PACKET_HEADER_SIZE, lenPacket);

              NetworkPacket* p = 0;
              if (!implNetworkPacketFactory::inst().readPacket(bs, &p)) {
                SW2_TRACE_ERROR("Read packet failed.");
                return false;
              }

              m_packetRecv += 1;
              onPacketReady_(*p);
              IncRecvPack();
            }
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
  bool send_(T* t, char const *buff, int szBuff, int type, ushort beg, ushort end)
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
  bool send_(T* t, std::string const& s)
  {
    //
    // Send stream raw data.
    //

    return send_(t, s.data(), (int)s.length(), 0, streamBeg, streamEnd);
  }

  template<class T>
  bool send_(T* t, NetworkPacket const& p)
  {
    //
    // Send packet data.
    //

    std::string buff;
    BitStream bs(buff);

    if (!implNetworkPacketFactory::inst().writePacket(bs, p)) {
      return false;
    }

    if (LARGE_PACKET_SIZE > bs.getByteCount()) { // Small packet.
      uint header = MAKE_PACKET_HEADER(bs.getByteCount(), 2, m_packetSent & 0xf);
      if (!t->send(2, (void*)&header) || !t->send(bs.getByteCount(), buff.data())) {
        return false;
      }
      m_packetSent += 1;
      IncSendPack();
    } else {                            // Large packet.
      if (!send_(t, buff.data(), bs.getByteCount(), 0, streamBeg, packetEnd)) {
        return false;
      }
    }

    return true;
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

  virtual void onStreamReady_(std::string const& s)=0;
  virtual void onPacketReady_(NetworkPacket const& p)=0;
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

  virtual NetworkClientStats getNetStats()
  {
    NetworkClientStats ns;

    *(SocketClientStats*)&ns = m_pClient->getNetStats();
    ns.packetsSent = m_packetSent;
    ns.packetsRecv = m_packetRecv;

    return ns;
  }

  virtual bool send(std::string const& s)
  {
    return implNetworkBase::send_(m_pClient, s);
  }

  virtual bool send(NetworkPacket const& p)
  {
    return implNetworkBase::send_(m_pClient, p);
  }

  virtual void trigger()
  {
    m_pClient->trigger();

    if (!implNetworkBase::trigger_(m_pClient)) {
      disconnect();
    }
  }

  virtual int getTriggerFrequency() const
  {
    return m_pClient->getTriggerFrequency();
  }

  virtual void setTriggerFrequency(int freq)
  {
    m_pClient->setTriggerFrequency(freq);
  }

  //
  // Implement implNetworkBase.
  //

  virtual void onStreamReady_(std::string const& s)
  {
    m_pInterface->onNetworkStreamReady(this, s);
  }

  virtual void onPacketReady_(NetworkPacket const& p)
  {
    m_pInterface->onNetworkPacketReady(this, p);
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
    if (!implNetworkBase::trigger_(m_pClientPeer)) {
      disconnect();
    }
  }

  //
  // Implement NetworkConnection.
  //

  virtual void disconnect()
  {
    m_pClientPeer->disconnect();
  }

  virtual int getConnectionState() const
  {
    return m_pClientPeer->getConnectionState();
  }

  virtual std::string getAddr() const
  {
    return m_pClientPeer->getAddr();
  }

  virtual NetworkClientStats getNetStats()
  {
    NetworkClientStats ns;

    *(SocketClientStats*)&ns = m_pClientPeer->getNetStats();
    ns.packetsSent = m_packetSent;
    ns.packetsRecv = m_packetRecv;

    return ns;
  }

  virtual bool send(std::string const& s)
  {
    return implNetworkBase::send_(m_pClientPeer, s);
  }

  virtual bool send(NetworkPacket const& p)
  {
    return implNetworkBase::send_(m_pClientPeer, p);
  }

  //
  // Implement implNetworkBase.
  //

  virtual void onStreamReady_(std::string const& s)
  {
    m_pInterface->onNetworkStreamReady(m_pServer, (NetworkConnection*)this, s);
  }

  virtual void onPacketReady_(NetworkPacket const& p)
  {
    m_pInterface->onNetworkPacketReady(m_pServer, (NetworkConnection*)this, p);
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
  SocketConnection* m_pClientPeer;
  NetworkServerCallback* m_pInterface;
  long *m_svrPacketSent, *m_svrPacketRecv;
};

class implNetworkServer : public NetworkServer, public SocketServerCallback
{
public:

  explicit implNetworkServer(NetworkServerCallback* pCallback) : m_pInterface(pCallback)
  {
    NetworkServer::userData = 0;
    m_pServer = SocketServer::alloc(this);
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

    implNetworkConnection& peer = m_poolClient[id];
    peer.userData = 0;
    peer.m_buffLen = 0;
    peer.m_deadConnectionTimeout.setTimeout(1000 * TIMEOUT_DEAD_CONNECTION);
    peer.m_keepAliveTimeout.setTimeout(1000 * TIMEOUT_KEEP_ALIVE);
    peer.m_pClientPeer = pNewClient;
    peer.m_pServer = this;
    peer.m_pInterface = m_pInterface;
    peer.m_packetSent = peer.m_packetRecv = 0;
    peer.m_svrPacketSent = &m_packetSent;
    peer.m_svrPacketRecv = &m_packetRecv;

    //
    // Accept this new connection?
    //

    if (m_pInterface->onNetworkNewClientReady(this, (NetworkConnection*)&peer)) {
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
    if (!c.handleStreamReady(c.m_pClientPeer, len, pStream)) {
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

    int next = m_poolClient.next((int)((implNetworkConnection*)pClient)->m_pClientPeer->userData);
    if (-1 == next) {
      return 0;
    } else {
      return (NetworkConnection*)&m_poolClient[next];
    }
  }

  virtual NetworkServerStats getNetStats()
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

  virtual int getTriggerFrequency() const
  {
    return m_pServer->getTriggerFrequency();
  }

  virtual void setTriggerFrequency(int freq)
  {
    m_pServer->setTriggerFrequency(freq);
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
  return new impl::implNetworkServer(pCallback);
}

void NetworkServer::free(NetworkServer* pServer)
{
  delete (impl::implNetworkServer*)pServer;
}

NetworkPacketRegister::NetworkPacketRegister(uint id, NetworkPacket::StaticCreatePacket pfn, char const* name)
{
  impl::implNetworkPacketFactory::inst().registerPacket(id, pfn, name);
}

} // namespace sw2

// end of swNetwork.cpp
