
//
// TCP/IP network [Framework layer]
//
// Copyright (c) 2015 Waync Cheng.
// All Rights Reserved.
//
// 2015/2/2 Waync created.
//

#include <algorithm>
#include <iterator>

#include "swBigworld.h"
#include "swNetwork.h"
#include "swObjectPool.h"

namespace sw2 {

namespace impl {

#define SW2_BIGWORLD_CONF_ID "Id"
#define SW2_BIGWORLD_CONF_KEEP_CONNECTED "KeepConnected"
#define SW2_BIGWORLD_CONF_ADDR_NODE "AddrNode"
#define SW2_BIGWORLD_CONF_DEPEX "Depex"
#define SW2_BIGWORLD_MAX_CHILD_NODE 1024
#define SW2_BIGWORLD_MAX_DEPEX_NODE 64

static const unsigned char SW2_BIGWORLD_REQ_ID[] = {0x1e, 0x33, 0x5e, 0x9f, 0x0f, 0x86, 0xb9, 0x48, 0xae, 0xc6, 0xb, 0xf3, 0x33, 0x4c, 0xa0, 0x8};
static const unsigned char SW2_BIGWORLD_RESP_ID[] = {0xb2, 0x06, 0x50, 0x05, 0x5f, 0xb7, 0x83, 0x44, 0xa1, 0x21, 0x93, 0x50, 0xba, 0x42, 0xf3, 0x4d};

class implBigworldChildNode : public BigworldNode
{
public:
  NetworkConnection *m_pConn;
  std::string m_Id;

  virtual ~implBigworldChildNode()
  {
  }

  //
  // Implement BigworldNode.
  //

  virtual std::string getId() const
  {
    return m_Id;
  }

  virtual std::string getAddr() const
  {
    return m_pConn->getAddr();
  }

  virtual NetworkClientStats getNetStats()
  {
    return m_pConn->getNetStats();
  }

  virtual bool isReady() const
  {
    return true;
  }

  virtual bool startup(Ini const &ini, std::string const &id)
  {
    return false;                       // NOP.
  }

  virtual void shutdown()
  {
    m_pConn->disconnect();
  }

  virtual void trigger()
  {
    //
    // NOP.
    //
  }

  virtual bool send(int len, void const* pStream)
  {
    return m_pConn->send(len, pStream);
  }

  virtual bool send(const NetworkPacket &p)
  {
    return m_pConn->send(p);
  }

  virtual bool addDepex(Ini const &ini, const std::vector<std::string> &ids)
  {
    return false;
  }

  virtual BigworldNode* getFirstChild()
  {
    return 0;
  }

  virtual BigworldNode* getNextChild(BigworldNode *pNode)
  {
    return 0;
  }

  virtual BigworldNode* getFirstDepex()
  {
    return 0;
  }

  virtual BigworldNode* getNextDepex(BigworldNode *pNode)
  {
    return 0;
  }
};

class implBigworldParentNode : public BigworldNode
{
public:
  NetworkClient *m_pClient;
  std::string m_Id;
  bool m_bKeepConnected;
  std::string m_AddrNode;

  virtual ~implBigworldParentNode()
  {
  }

  //
  // Implement BigworldNode.
  //

  virtual std::string getId() const
  {
    return m_Id;
  }

  virtual std::string getAddr() const
  {
    return m_AddrNode;
  }

  virtual NetworkClientStats getNetStats()
  {
    if (m_pClient) {
      return m_pClient->getNetStats();
    } else {
      return NetworkClientStats();
    }
  }

  virtual bool isReady() const
  {
    if (m_pClient) {
      return CS_CONNECTED == m_pClient->getConnectionState();
    } else {
      return false;
    }
  }

  virtual bool startup(Ini const &ini, std::string const &id)
  {
    return false;
  }

  virtual void shutdown()
  {
    if (m_pClient) {
      m_pClient->disconnect();
    }
  }

  virtual void trigger()
  {
    if (m_pClient) {
      m_pClient->trigger();
    }
  }

  virtual bool send(int len, void const* pStream)
  {
    if (m_pClient) {
      return m_pClient->send(len, pStream);
    } else {
      return false;
    }
  }

  virtual bool send(const NetworkPacket &p)
  {
    if (m_pClient) {
      return m_pClient->send(p);
    } else {
      return false;
    }
  }

  virtual bool addDepex(Ini const &ini, const std::vector<std::string> &ids)
  {
    return false;
  }

  virtual BigworldNode* getFirstChild()
  {
    return 0;
  }

  virtual BigworldNode* getNextChild(BigworldNode *pNode)
  {
    return 0;
  }

  virtual BigworldNode* getFirstDepex()
  {
    return 0;
  }

  virtual BigworldNode* getNextDepex(BigworldNode *pNode)
  {
    return 0;
  }
};

class implBigworldNode : public BigworldNode, public NetworkServerCallback, public NetworkClientCallback
{
public:
  BigworldCallback *m_pCallback;
  std::string m_id;                     // Node Type|ID of this node.
  std::string m_addrNode;
  NetworkServer *m_pServer;
  ObjectPool<implBigworldChildNode, SW2_BIGWORLD_MAX_CHILD_NODE> m_poolChild;
  ObjectPool<implBigworldParentNode, SW2_BIGWORLD_MAX_DEPEX_NODE> m_poolDepex;

  explicit implBigworldNode(BigworldCallback *pCallback) : m_pCallback(pCallback), m_pServer(0)
  {
  }

  virtual ~implBigworldNode()
  {
  }

  void destroy()
  {
    shutdown();
    for (int i = m_poolDepex.first(); -1 != i; i = m_poolDepex.next(i)) {
      implBigworldParentNode &p = m_poolDepex[i];
      if (p.m_pClient) {
        NetworkClient::free(p.m_pClient);
        p.m_pClient = 0;
      }
    }
  }

  //
  // Implement NetworkServerCallback.
  //

  virtual bool onNetworkNewClientReady(NetworkServer*, NetworkConnection* pNewClient)
  {
    int id = m_poolChild.alloc();
    if (-1 == id) {
      return false;
    }

    pNewClient->userData = (uint_ptr)id;

    implBigworldChildNode &node = m_poolChild[id];
    node.userData = 0;
    node.m_pConn = pNewClient;
    node.m_Id = "";

    pNewClient->send(sizeof(SW2_BIGWORLD_REQ_ID), SW2_BIGWORLD_REQ_ID);

    return true;
  }

  virtual void onNetworkClientLeave(NetworkServer*, NetworkConnection* pClient)
  {
    int id = (int)pClient->userData;
    m_pCallback->onBigworldNodeClose((BigworldNode*)this, (BigworldNode*)&m_poolChild[id]);
    m_poolChild.free(id);
  }

  virtual void onNetworkStreamReady(NetworkServer*, NetworkConnection* pClient, int len, void const* pStream)
  {
    const int sz = sizeof(SW2_BIGWORLD_RESP_ID);
    int id = (int)pClient->userData;
    implBigworldChildNode &c = m_poolChild[id];
    if (sz < len && !memcmp(pStream, SW2_BIGWORLD_RESP_ID, sz)) {
      if (c.getId().empty()) {
        c.m_Id = std::string((const char*)pStream + sz, len - sz);
        m_pCallback->onBigworldNewNodeReady((BigworldNode*)this, (BigworldNode*)&c);
      }
    } else {
      m_pCallback->onBigworldStreamReady((BigworldNode*)this, (BigworldNode*)&c, len, pStream);
    }
  }

  virtual void onNetworkPacketReady(NetworkServer*, NetworkConnection* pClient, NetworkPacket const& p)
  {
    int id = (int)pClient->userData;
    m_pCallback->onBigworldEventReady((BigworldNode*)this, (BigworldNode*)&m_poolChild[id], p);
  }

  //
  // Implement NetworkClientCallback.
  //

  virtual void onNetworkServerReady(NetworkClient* pClient)
  {
    int id = (int)pClient->userData;
    m_pCallback->onBigworldNewNodeReady((BigworldNode*)this, (BigworldNode*)&m_poolDepex[id]);
  }

  virtual void onNetworkServerLeave(NetworkClient* pClient)
  {
    int id = (int)pClient->userData;
    m_pCallback->onBigworldNodeClose((BigworldNode*)this, (BigworldNode*)&m_poolDepex[id]);
  }

  virtual void onNetworkStreamReady(NetworkClient* pClient, int len, void const* pStream)
  {
    const int sz = sizeof(SW2_BIGWORLD_REQ_ID);
    int id = (int)pClient->userData;
    if (sz == len && !memcmp(pStream, SW2_BIGWORLD_REQ_ID, sz)) {
      std::string s((const char*)SW2_BIGWORLD_RESP_ID, sz);
      s.append(getId());
      pClient->send((int)s.size(), s.c_str());
    } else {
      m_pCallback->onBigworldStreamReady((BigworldNode*)this, (BigworldNode*)&m_poolDepex[id], len, pStream);
    }
  }

  virtual void onNetworkPacketReady(NetworkClient* pClient, NetworkPacket const& p)
  {
    int id = (int)pClient->userData;
    m_pCallback->onBigworldEventReady((BigworldNode*)this, (BigworldNode*)&m_poolDepex[id], p);
  }

  //
  // Implement BigworldNode.
  //

  virtual std::string getId() const
  {
    return m_id;
  }

  virtual std::string getAddr() const
  {
    return m_addrNode;
  }

  virtual NetworkClientStats getNetStats()
  {
    NetworkClientStats cs;
    if (m_pServer) {
      NetworkServerStats ss = m_pServer->getNetStats();
      cs.startTime = ss.startTime;
      cs.upTime = ss.upTime;
      cs.bytesRecv = ss.bytesRecv;
      cs.bytesSent = ss.bytesSent;
      cs.packetsRecv = ss.packetsRecv;
      cs.packetsSent = ss.packetsSent;
    } else {
      memset(&cs, 0, sizeof(cs));
      for (int i = m_poolDepex.first(); -1 != i; i = m_poolDepex.next(i)) {
        NetworkClientStats ns = m_poolDepex[i].m_pClient->getNetStats();
        cs.bytesRecv += ns.bytesRecv;
        cs.bytesSent += ns.bytesSent;
        cs.packetsRecv += ns.packetsRecv;
        cs.packetsSent += ns.packetsSent;
        cs.startTime = ns.startTime;
        cs.upTime = ns.upTime;
      }
    }
    return cs;
  }

  virtual bool isReady() const
  {
    return 0 != m_pServer;
  }

  virtual bool startup(Ini const &ini, std::string const &id)
  {
    //
    // Load INI conf.
    //

    if (0 == ini.find(id)) {
      return false;
    }

    //
    // Startup server.
    //

    shutdown();

    Ini conf = ini[id];

    if (!conf.find(SW2_BIGWORLD_CONF_ID)) {
      m_id = id;
    } else {
      m_id = conf[SW2_BIGWORLD_CONF_ID].value;
    }

    if (conf.find(SW2_BIGWORLD_CONF_ADDR_NODE)) {
      m_pServer = NetworkServer::alloc(this);
      if (0 == m_pServer) {
        return false;
      }
      m_addrNode = conf[SW2_BIGWORLD_CONF_ADDR_NODE].value;
      if (!m_pServer->startup(m_addrNode)) {
        return false;
      }
      m_addrNode = getServerAddr_i();
    } else {
      // Ignore fail if this node is client only.
    }

    //
    // Connect depex.
    //

    std::stringstream ss(conf[SW2_BIGWORLD_CONF_DEPEX].value);
    std::vector<std::string> v;
    v.assign(std::istream_iterator<std::string>(ss), std::istream_iterator<std::string>());
    connectDepex(ini, v);

    return true;
  }

  virtual void shutdown()
  {
    if (m_pServer) {
      m_pServer->shutdown();
      NetworkServer::free(m_pServer);
      m_pServer = 0;
    }
  }

  virtual void trigger()
  {
    if (m_pServer) {
      m_pServer->trigger();
    }

    for (int dep = m_poolDepex.first(); -1 != dep; dep = m_poolDepex.next(dep)) {
      implBigworldParentNode &p = m_poolDepex[dep];
      NetworkClient *c = p.m_pClient;
      if (p.m_bKeepConnected && CS_DISCONNECTED == c->getConnectionState()) {
        c->connect(p.m_AddrNode);       // Try to keep connect to depex node.
      }
      c->trigger();
    }
  }

  virtual bool send(int len, void const* pStream)
  {
    return false;                       // NOP.
  }

  virtual bool send(const NetworkPacket &p)
  {
    return false;                       // NOP.
  }

  virtual bool addDepex(Ini const &ini, const std::vector<std::string> &ids)
  {
    std::vector<std::string> depex;
    for (int i = m_poolDepex.first(); -1 != i; i = m_poolDepex.next(i)) {
      const implBigworldParentNode &n = m_poolDepex[i];
      depex.push_back(n.m_Id);
    }

    std::vector<std::string> v;
    for (size_t i = 0; i < ids.size(); i++) {
      const std::string &id = ids[i];
      std::vector<std::string>::iterator itDep = std::find(depex.begin(), depex.end(), id);
      if (depex.end() == itDep) {
        v.push_back(id);                // New depex node.
      } else {
        updateDepex(ini, id);           // Update exist depex node info.
      }
    }

    return connectDepex(ini, v);
  }

  virtual BigworldNode* getFirstChild()
  {
    int first = m_poolChild.first();
    if (-1 == first) {
      return 0;
    } else {
      return &m_poolChild[first];
    }
  }

  virtual BigworldNode* getNextChild(BigworldNode *pNode)
  {
    if (!pNode) {
      return 0;
    }

    int next = m_poolChild.next((int)((implBigworldChildNode*)pNode)->m_pConn->userData);
    if (-1 == next) {
      return 0;
    } else {
      return &m_poolChild[next];
    }
  }

  virtual BigworldNode* getFirstDepex()
  {
    int first = m_poolDepex.first();
    if (-1 == first) {
      return 0;
    } else {
      return &m_poolDepex[first];
    }
  }

  virtual BigworldNode* getNextDepex(BigworldNode *pNode)
  {
    if (!pNode) {
      return 0;
    }

    NetworkClient *pClient = ((implBigworldParentNode*)pNode)->m_pClient;
    if (!pClient) {
      return 0;
    }

    int next = m_poolDepex.next((int)pClient->userData);
    if (-1 == next) {
      return 0;
    } else {
      return &m_poolDepex[next];
    }
  }

  //
  // Common.
  //

  bool connectDepex(Ini const &ini, const std::vector<std::string> &v)
  {
    for (int i = 0; i < (int)v.size(); i++) {

      //
      // Find depex conf.
      //

      std::string idNode = v[i];

      if (0 == ini.find(idNode)) {
        continue;
      }

      Ini conf = ini[idNode];

      //
      // Alloc pool id.
      //

      int id = m_poolDepex.alloc();
      if (-1 == id) {
        continue;
      }

      //
      // Alloc network clien inst.
      //

      NetworkClient *pClient = NetworkClient::alloc(this);
      if (!pClient) {
        m_poolDepex.free(id);
        continue;
      }

      pClient->userData = (int_ptr)id;

      //
      // Init bigworld depex node.
      //

      implBigworldParentNode &node = m_poolDepex[id];
      node.userData = 0;
      node.m_pClient = pClient;

      setupDepex(conf, idNode, node);

      //
      // Try to connect to depex node.
      //

      pClient->connect(node.m_AddrNode);
    }

    return true;
  }

  std::string getServerAddr_i() const
  {
    std::string addr = m_pServer->getAddr();
    const std::string AnyAddr("0.0.0.0");
    if (std::string::npos != addr.find(AnyAddr)) {
      addr.replace(0, AnyAddr.size(), "localhost");
    }
    return addr;
  }

  void updateDepex(const Ini &ini, const std::string &idNode)
  {
    if (0 == ini.find(idNode)) {
      return;
    }

    for (int i = m_poolDepex.first(); -1 != i; i = m_poolDepex.next(i)) {
      implBigworldParentNode &n = m_poolDepex[i];
      if (idNode == n.m_Id) {
        setupDepex(ini[idNode], idNode, n);
        break;
      }
    }
  }

  void setupDepex(const Ini &conf, const std::string &idNode, implBigworldParentNode &node)
  {
    node.m_AddrNode = conf[SW2_BIGWORLD_CONF_ADDR_NODE].value;

    if (conf.find(SW2_BIGWORLD_CONF_KEEP_CONNECTED)) {
      node.m_bKeepConnected = conf[SW2_BIGWORLD_CONF_KEEP_CONNECTED];
    } else {
      node.m_bKeepConnected = true;
    }

    if (!conf.find(SW2_BIGWORLD_CONF_ID)) {
      node.m_Id = idNode;
    } else {
      node.m_Id = conf[SW2_BIGWORLD_CONF_ID].value;
    }
  }
};

} // namespace impl

BigworldNode* BigworldNode::alloc(BigworldCallback *pCallback)
{
  assert(pCallback);
  return new impl::implBigworldNode(pCallback);
}

void BigworldNode::free(BigworldNode *pNode)
{
  impl::implBigworldNode *p = (impl::implBigworldNode*)pNode;
  p->destroy();
  delete p;
}

bool InitializeBigworld()
{
  if (!InitializeNetwork()) {
    return false;
  }

  SW2_TRACE_MESSAGE("swBigworld initialized.");
  return true;
}

void UninitializeBigworld()
{
  SW2_TRACE_MESSAGE("swBigworld initialized.");
  UninitializeNetwork();
}

} // namespace sw2

// end of swBigworld.cpp
