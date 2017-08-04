
//
//  Smallworld Account Server implementation.
//
//  Copyright (c) 2014 Waync Cheng.
//  All Rights Reserved.
//
//  2014/03/02 Waync created.
//

#include "swSmallworldEv.h"

#include "swObjectPool.h"
#include "swUtil.h"

namespace sw2 {

namespace impl {

//
// Constants.
//

#define SMALLWORLD_MAX_PEER 64          // Max AccountPeer(Smallworld Server).
#define SMALLWORLD_TIMEOUT_LOGIN 5000   // Timeout timer for wait for login.

//
// Config.
//

struct CONFIG_ACCOUNT
{
 std::string addrListen;                // Listen address, format: IP:Port, hostname:Port or Port.
 int maxServer;                         // Max server count.
};

//
// SmallworldAccountConnection implementation.
//

class implSmallworldAccountPeer : public SmallworldAccountConnection
{
public:

  virtual void disconnect()
  {
    assert(m_pNetPeer);
    m_pNetPeer->disconnect();
  }

  virtual std::string getAddr() const
  {
    assert(m_pNetPeer);
    return m_pNetPeer->getAddr();
  }

  virtual NetworkClientStats getNetStats()
  {
    assert(m_pNetPeer);
    return m_pNetPeer->getNetStats();
  }

  virtual bool replyPlayerLogin(int code, std::string const& token)
  {
    evSmallworldRequest er;

    BitStream bs((char*)token.data(), (int)token.length());
    bs >> er.idPlayer >> er.time;

    if (SMALLWORLD_RAC_SUCCESS == code) { // Login success.
      er.code = evSmallworldRequest::NC_PLAYER_LOGIN;
    } else if (SMALLWORLD_RAC_ACCOUNT_OR_PASSWORD == code) { // Account or password error.
      er.code = evSmallworldRequest::NC_ACCOUNT_OR_PASSWORD;
    } else if (SMALLWORLD_RAC_DUPLICATE_LOGIN == code) { // Duplicate login.
      er.code = evSmallworldRequest::NC_DUPLICATE_LOGIN;
    } else if (SMALLWORLD_RAC_NOT_ALLOW_LOGIN == code) { //  Not allow login.
      er.code = evSmallworldRequest::NC_NOT_ALLOWED;
    } else {                            // Invalid code.
      return false;
    }

    assert(m_pNetPeer);

    return m_pNetPeer->send(er);
  }

  virtual bool replyPlayerLogout(int code, std::string const& token)
  {
    evSmallworldRequest er;

    BitStream bs((char*)token.data(), (int)token.length());
    bs >> er.idPlayer >> er.time;

    if (SMALLWORLD_RAC_SUCCESS == code) { // Login success.
      er.code = evSmallworldRequest::NC_PLAYER_LOGOUT;
    } else if (SMALLWORLD_RAC_NOT_LOGIN == code) { // Has not login.
      er.code = evSmallworldRequest::NC_NOT_LOGIN;
    } else {                            // Invalid code.
      return false;
    }

    assert(m_pNetPeer);

    return m_pNetPeer->send(er);
  }

  virtual int getServerId() const
  {
    return m_idServer;
  }

public:

  bool m_bVerified;                     // Has logged in?
  int m_idServer;                       // Server ID.
  TimeoutTimer m_timer;                 // Wait for login timeout timer.
  NetworkConnection* m_pNetPeer;        // NetworkConnection.
};

//
// SmallworldAccount implementation.
//

class implSmallworldAccount : public SmallworldAccount, public NetworkServerCallback
{
public:

  explicit implSmallworldAccount(SmallworldAccountCallback* pCallback) : m_pCallback(pCallback), m_pServer(0)
  {
    SmallworldAccount::userData = 0;
  }

  virtual ~implSmallworldAccount()
  {
  }

  //
  // Initialization.
  //

  bool init()
  {
    m_pServer = NetworkServer::alloc(this);
    return 0 != m_pServer;
  }

  void uninit()
  {
    NetworkServer::free(m_pServer);
    m_pServer = 0;
    m_pool.clear();
  }

  //
  // Startup.
  //

  virtual bool startup(Ini const& conf1)
  {
    Ini conf = conf1;
    m_conf.addrListen = conf["AddrListen"].value;
    m_conf.maxServer = conf.find("MaxServer") ? conf["MaxServer"] : (int)SMALLWORLD_MAX_PEER;

    m_conf.maxServer = std::max(0, std::min(m_conf.maxServer, (int)SMALLWORLD_MAX_PEER));
    return m_pServer->startup(m_conf.addrListen);
  }

  virtual void shutdown()
  {
    //
    // Shutdown server.
    //

    m_pServer->shutdown();

    //
    // Kill all connections.
    //

    for (int i = m_pool.first(); -1 != i; i = m_pool.next(i)) {
      m_pool[i].disconnect();
    }

    while (m_pool.size()) {
      m_pServer->trigger();
    }
  }

  virtual NetworkServerStats getNetStats()
  {
    return m_pServer->getNetStats();
  }

  //
  // Trigger.
  //

  virtual void trigger()
  {
    //
    // Trigger network.
    //

    m_pServer->trigger();

    //
    // Iterate current online peers, check timeout timer.
    //

    for (int i = m_pool.first(); -1 != i; i = m_pool.next(i)) {
      if (!m_pool[i].m_bVerified) {
        implSmallworldAccountPeer& peer = m_pool[i];
        if (peer.m_timer.isExpired()) { // Wait for login timer timeout, kick.
          assert(peer.m_pNetPeer);
          peer.m_pNetPeer->disconnect();
        }
      }
    }
  }

  //
  // Iterator.
  //

  virtual SmallworldAccountConnection* getFirstServer()
  {
    int iter = m_pool.first();
    if (-1 == iter) {
      return 0;
    } else {
      return &m_pool[iter];
    }
  }

  virtual SmallworldAccountConnection* getNextServer(SmallworldAccountConnection* pServer)
  {
    if (0 == pServer) {
      return 0;
    }

    int iter = m_pool.next(pServer->getServerId());
    if (-1 == iter) {
      return 0;
    } else {
      return &m_pool[iter];
    }
  }

  //
  // Implement NetworkServerCallback.
  //

  virtual void onNetworkServerStartup(NetworkServer* pServer)
  {
    m_pCallback->onSmallworldAccountServerStartup(this);
  }

  virtual void onNetworkServerShutdown(NetworkServer* pServer)
  {
    m_pCallback->onSmallworldAccountServerShutdown(this);
  }

  virtual bool onNetworkNewClientReady(NetworkServer* pServer, NetworkConnection* pNewClient)
  {
    assert(pNewClient);

    //
    // Check is server busy.
    //

    if (m_conf.maxServer == m_pool.size()) {
      evSmallworldNotify n;
      n.code = evSmallworldNotify::NC_SERVER_BUSY;
      pNewClient->send(n);              // Notify server busy and ignore failed.
      return false;
    }

    //
    // Allocate a free space for this new connection.
    //

    int id = m_pool.alloc();
    assert(-1 != id);

    //
    // Notify need login command.
    //

    evSmallworldNotify n;
    n.code = evSmallworldNotify::NC_NEED_LOGIN;

    if (!pNewClient->send(n)) {         // Send failed?
      m_pool.free(id);
      return false;
    }

    //
    // Setup Account connection.
    //

    implSmallworldAccountPeer& peer = m_pool[id];
    peer.m_idServer = id;
    peer.m_bVerified = false;
    peer.m_timer.setTimeout(SMALLWORLD_TIMEOUT_LOGIN);
    peer.m_pNetPeer = pNewClient;
    pNewClient->userData = (uint_ptr)id; // Associate.

    return true;                        // Accept this new connection.
  }

  virtual void onNetworkClientLeave(NetworkServer* pServer, NetworkConnection* pClient)
  {
    assert(pClient);

    implSmallworldAccountPeer& peer = m_pool[(int)pClient->userData];

    //
    // Callback server leave when this peer has logged in.
    //

    if (peer.m_bVerified) {
      m_pCallback->onSmallworldServerLeave(this, &peer);
    }

    //
    // Free this connection.
    //

    m_pool.free(peer.m_idServer);
  }

  virtual void onNetworkPacketReady(NetworkServer* pServer, NetworkConnection* pClient, const NetworkPacket& p)
  {
    assert(pClient);

    //
    // Server Login.
    //

    if (EID_LOGIN == p.getId()) {
      implSmallworldAccountPeer& peer = m_pool[(int)pClient->userData];

      //
      // Duplicated login?
      //

      if (peer.m_bVerified) {
        SW2_TRACE_ERROR("[AC] Duplicate login received from %s, Kick", pClient->getAddr().c_str());
        pClient->disconnect();
        return;
      }

      //
      // Normal login.
      //

      evSmallworldLogin* pl = (evSmallworldLogin*)&p; // Store login information.

      if (SMALLWORLD_VERSION_MAJOR != pl->verMajor ||
          SMALLWORLD_VERSION_MINOR != pl->verMinor) {
        evSmallworldNotify n;
        n.code = evSmallworldNotify::NC_VERSION_MISMATCH;
        pClient->send(n);               // Ignore failed.
        pClient->disconnect();
        return;
      }

      peer.userData = (uint_ptr)0;
      peer.m_bVerified = true;

      //
      // Callback new server ready.
      //

      if (m_pCallback->onSmallworldNewServerReady(this, &peer)) { // Accept?
        evSmallworldNotify n;
        n.code = evSmallworldNotify::NC_LOGIN_ACCEPTED;
        n.id = peer.m_idServer;
        if (!pClient->send(n)) {
          SW2_TRACE_ERROR("[AC] Reply Login Accepted Failed from %s, Kick", pClient->getAddr().c_str());
          pClient->disconnect();
          m_pCallback->onSmallworldServerLeave(this, &peer);
          peer.m_bVerified = false;
        }
      } else {
        evSmallworldNotify n;
        n.code = evSmallworldNotify::NC_LOGIN_NOT_ALLOWED;
        pClient->send(n);               // Ignore failed.
        pClient->disconnect();
        peer.m_bVerified = false;       // Avoid to notify server leave.
      }

      return;
    }

    //
    // Request player login/out.
    //

    if (EID_REQUEST == p.getId()) {
      implSmallworldAccountPeer& peer = m_pool[(int)pClient->userData];

      //
      // Verified?
      //

      if (!peer.m_bVerified) {
        SW2_TRACE_ERROR("[AC] Request before login from %s, Kick", pClient->getAddr().c_str());
        pClient->disconnect();
        return;
      }

      //
      // Handle request.
      //

      evSmallworldRequest* pr = (evSmallworldRequest*)&p;

      std::string token(32, 0);
      BitStream bs((char*)token.data(), (int)token.length());
      bs << pr->idPlayer << pr->time;
      token.resize(bs.getByteCount());

      if (evSmallworldRequest::NC_PLAYER_LOGIN == pr->code) { // Player login request.
        m_pCallback->onSmallworldRequestPlayerLogin(this, &peer, pr->stream, token);
      } else if (evSmallworldRequest::NC_PLAYER_LOGOUT == pr->code) { // Player logout request.
        m_pCallback->onSmallworldRequestPlayerLogout(this, &peer, pr->stream, token);
      } else {                          // Invalid request, kick.
        SW2_TRACE_ERROR("[AC] Invalid request from %s, Kick", pClient->getAddr().c_str());
        pClient->disconnect();
      }

      return;
    }

    //
    // unknown command
    //

    SW2_TRACE_ERROR("[AC] Unknown event received from %s, Kick", pClient->getAddr().c_str());
    pClient->disconnect();
  }

  virtual void onNetworkStreamReady(NetworkServer* pServer, NetworkConnection* pClient, std::string const&)
  {
    //
    // Never used, kick.
    //

    SW2_TRACE_ERROR("[AC] Unknown stream received from %s, Kick", pClient->getAddr().c_str());
    pClient->disconnect();
  }

public:

  SmallworldAccountCallback* m_pCallback; // Callback interface.
  NetworkServer* m_pServer;               // Network server.
  ObjectPool<implSmallworldAccountPeer, SMALLWORLD_MAX_PEER> m_pool; // implSmallworldAccountPeer object pool.
  CONFIG_ACCOUNT m_conf;                  // Configuration.
};

} // namespace impl

SmallworldAccount* SmallworldAccount::alloc(SmallworldAccountCallback* pCallback)
{
  assert(pCallback);
  impl::implSmallworldAccount* p = new impl::implSmallworldAccount(pCallback);
  if (p && !p->init()) {
    delete p;
    p = 0;
  }
  return p;
}

void SmallworldAccount::free(SmallworldAccount* pInstance)
{
  impl::implSmallworldAccount* p = (impl::implSmallworldAccount*)pInstance;
  if (p) {
    p->uninit();
  }
  delete p;
}

} // namespace sw2

// end of swSmallworldAccount.cpp
