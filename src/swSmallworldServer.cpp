
//
//  Smallworld Server implementation.
//
//  Copyright (c) 2014 Waync Cheng.
//  All Rights Reserved.
//
//  2014/03/02 Waync created.
//

#include <algorithm>
#include <sstream>

#include "swObjectPool.h"
#include "swStageStack.h"
#include "swUtil.h"

#include "swSmallworldEv.h"

namespace sw2 {

namespace impl {

//
// Internal constants.
//

#define SMALLWORLD_TIMEOUT_LOGIN 5000   // Timeout timer for wait for login.
#define SMALLWORLD_INIT_GAME_PLAYERS 8  // Initial size of game players pool.

//
// Config.
//

struct CONFIG_SERVER
{
 std::string addrAccount;               // Account server address, format: IP:Port.

 bool bEnablePlayerList;                // Is enable send player list to client?
 bool bEnableGameList;                  // Is enable send game list to client?
 bool bEnableChannel;                   // Is enable channel mode?

 std::string addrListen;                // Listen address, format: IP:Port, hostname:Port or Port.
 int maxPlayer;                         // Max player count at the same time.
 int maxChannel;                        // Max channel count.
 int maxChannelPlayer;                  // Max player count in a channel.
};

//
// Internal class declaration.
//

class implSmallworldServer;

class implSmallworldServerGame : public SmallworldGame
{
public:

  virtual ~implSmallworldServerGame();

  //
  // Implement SmallworldGame.
  //

  virtual int getGameId() const;
  virtual int getChannelId() const;

  virtual SmallworldPlayer* getFirstPlayer();
  virtual SmallworldPlayer* getNextPlayer(SmallworldPlayer* pPlayer);

public:

  int m_idGame;                         // This game ID.
  int m_iChannel;                       // Channel index.
  ObjectPool<int, SMALLWORLD_INIT_GAME_PLAYERS, true> m_players; // Players' id who join the game, first one is the creator(game master).
  int m_idChannelGame;                  // ID of SmallworldServer::m_channelGame[m_iChannel].
  implSmallworldServer* m_pServer;      // Interface to impl::implSmallworldServer.
};

class implSmallworldServerPlayer : public SmallworldPlayer
{
public:

  virtual ~implSmallworldServerPlayer();

  //
  // Implement SmallworldPlayer.
  //

  virtual void disconnect();

  virtual std::string getAddr() const;
  virtual NetworkClientStats getNetStats();

  virtual bool send(const NetworkPacket& p);
  virtual bool send(int len, void const* pStream);

  virtual bool sendMessage(const std::string& msg);
  virtual bool sendPrivateMessage(int idWho, const std::string& msg);

  virtual bool changeChannel(int newChannel);

  virtual bool newGame();
  virtual bool joinGame(int idGame);
  virtual bool quitGame();

  virtual int getPlayerId() const;
  virtual int getChannelId() const;

  virtual std::string getLoginData() const;

  virtual SmallworldGame* getGame();

  //
  // Implement stages.
  //

  void stageDisconnecting(int state, uint_ptr);
  void stageReady(int state, uint_ptr);
  void stageWait4AccountServerReply(int state, uint_ptr event);
  void stageWait4Login(int state, uint_ptr event);

  //
  // Ready stage handler.
  //

  void handleWait4LoginEvent(evSmallworldLogin *pLogin);
  void handleReadyStageEvent(NetworkPacket *pEvent);
  void handleChannelEvent(evSmallworldChannel* pChannel);
  void handleChatEvent(evSmallworldChat* pChat);
  void handleGameEvent(evSmallworldGame* pGame);
  void handleUserEvent(NetworkPacket* pEvent);

  void initReadyStage();
  void uninitReadyStage();

  //
  // Helper.
  //

  void broadcastEnterChannel();
  void broadcastLeaveChannel();

public:

  NetworkConnection* m_pNetPeer;        // NetworkConnection.
  bool m_bVerified;                     // Is verified(logged in).
  bool m_bAccept;                       // Is accepted by server?
  bool m_bWait4Login;                   // Is waiting for client login?
  int m_idPlayer;                       // This player ID.
  int m_idGame;                         // Joined game ID.
  int m_iChannel;                       // Channel index.
  int m_idChannel;                      // ID of SmallworldServer::m_channelPlayer[m_iChannel].
  int m_idGameSeat;                     // Seat ID of the game.
  TimeoutTimer m_timer;                 // Timeout timer and time stamp for account server.
  std::string m_stream;                 // Login user stream.
  implSmallworldServer* m_pServer;      // Interface to impl::implSmallworldServer.
  bool m_bNeedPlayerList, m_bNeedGameList, m_bNeedMessage; // Notify flag.
  StageStack<implSmallworldServerPlayer> m_stage; // Stage controller.
};

class implSmallworldServerAccountClient : public NetworkClientCallback
{
public:

  implSmallworldServerAccountClient();

  //
  // Implement Network::NetworkClientCallback
  //

  virtual void onNetworkServerLeave(NetworkClient* pClient);
  virtual void onNetworkPacketReady(NetworkClient* pClient, const NetworkPacket& p);

public:

  implSmallworldServer* m_pServer;      // Interface to impl::implSmallworldServer.
  NetworkClient* m_pClient;             // Network client, account client.
};

class implSmallworldServer : public SmallworldServer, public NetworkServerCallback
{
public:

  explicit implSmallworldServer(SmallworldServerCallback* pCallback);
  virtual ~implSmallworldServer();

  //
  // Initialization.
  //

  bool init();
  void uninit();

  //
  // Startup.
  //

  virtual bool startup(Ini const& conf);
  virtual void shutdown();

  //
  // Trigger.
  //

  virtual void trigger();

  //
  // Stats.
  //

  virtual NetworkServerStats getNetStats();

  //
  // Iterator.
  //

  virtual SmallworldPlayer* getFirstPlayer();
  virtual SmallworldPlayer* getNextPlayer(SmallworldPlayer* pPlayer);

  virtual SmallworldGame* getFirstGame();
  virtual SmallworldGame* getNextGame(SmallworldGame* pGame);

  //
  // Implement NetworkServerCallback.
  //

  virtual void onNetworkServerStartup(NetworkServer*);
  virtual void onNetworkServerShutdown(NetworkServer*);

  virtual bool onNetworkNewClientReady(NetworkServer*, NetworkConnection* pNewClient);
  virtual void onNetworkClientLeave(NetworkServer*, NetworkConnection* pClient);
  virtual void onNetworkPacketReady(NetworkServer*, NetworkConnection* pClient, const NetworkPacket& p);
  virtual void onNetworkStreamReady(NetworkServer*, NetworkConnection* pClient, int len, void const* pStream);

  //
  // Implement stages.
  //

  void stageDummy(int, uint_ptr);
  void stageInitialization(int, uint_ptr);
  void stagePhaseAccount(int, uint_ptr);
  void stageStartup(int, uint_ptr);
  void stageReady(int, uint_ptr);

public:

  SmallworldServerCallback* m_pCallback; // Callback interface.
  CONFIG_SERVER m_conf;                 // Configuration.
  implSmallworldServerAccountClient m_acClient; // Network client, account client.
  TimeoutTimer m_timer;                 // Timeout timer.
  NetworkServer* m_pServer;             // Network server.
  ObjectPool<implSmallworldServerPlayer, SMALLWORLD_MAX_PLAYER> m_player; // implSmallworldServerPlayer object pool(all players in the server).
  ObjectPool<implSmallworldServerGame, SMALLWORLD_MAX_PLAYER> m_game; // STRUCT_GAME object pool(all games in the server).
  ObjectPool<int, SMALLWORLD_MAX_PLAYER> m_channelPlayer[SMALLWORLD_MAX_CHANNEL]; // Channel player ID pool.
  ObjectPool<int, SMALLWORLD_MAX_PLAYER> m_channelGame[SMALLWORLD_MAX_CHANNEL]; // Channel game ID pool.
  StageStack<implSmallworldServer> m_stage; // Stage controller.
  bool m_bReady2Go;                     // Ready flag, initialized but not up.
  bool m_bReady;                        // Ready flag, intialized and up.
};

//
// Implementation.
//

implSmallworldServerGame::~implSmallworldServerGame()
{
}

int implSmallworldServerGame::getGameId() const
{
  return m_idGame;
}

int implSmallworldServerGame::getChannelId() const
{
  return m_iChannel;
}

SmallworldPlayer* implSmallworldServerGame::getFirstPlayer()
{
  int iter = m_players.first();
  if (-1 == iter) {
    return 0;
  } else {
    return &m_pServer->m_player[m_players[iter]];
  }
}

SmallworldPlayer* implSmallworldServerGame::getNextPlayer(SmallworldPlayer* pPlayer)
{
  if (0 == pPlayer) {
    return 0;
  }

  int iter = m_players.next(pPlayer->getPlayerId());
  if (-1 == iter) {
    return 0;
  } else {
    return &m_pServer->m_player[m_players[iter]];
  }
}

implSmallworldServer::implSmallworldServer(SmallworldServerCallback* pCallback) : m_pCallback(pCallback), m_pServer(0), m_bReady2Go(false), m_bReady(false)
{
  m_acClient.m_pServer = this;
  SmallworldServer::userData = 0;
}

implSmallworldServer::~implSmallworldServer()
{
}

bool implSmallworldServer::init()
{
  m_bReady2Go = true;
  m_bReady = false;

  m_pServer = NetworkServer::alloc(this);
  if (0 == m_pServer) {
    return false;
  }

  m_stage.initialize(this, &implSmallworldServer::stageDummy);

  return true;
}

void implSmallworldServer::uninit()
{
  shutdown();

  NetworkServer::free(m_pServer);
  m_pServer = 0;

  if (0 != m_acClient.m_pClient) {
    NetworkClient::free(m_acClient.m_pClient);
    m_acClient.m_pClient = 0;
  }
}

bool implSmallworldServer::startup(Ini const& conf1)
{
  if (!m_bReady2Go) {
    return false;
  }

  Ini conf = conf1;
  m_conf.addrAccount = conf["AddrAccount"].value;
  m_conf.bEnablePlayerList = conf["EnablePlayerList"];
  m_conf.bEnableGameList = conf["EnableGameList"];
  m_conf.bEnableChannel = conf["EnableChannel"];
  m_conf.addrListen = conf["AddrListen"].value;
  m_conf.maxPlayer = conf.find("MaxPlayer") ? conf["MaxPlayer"] : (int)SMALLWORLD_MAX_PLAYER;
  m_conf.maxChannel = conf.find("MaxChannel") ? conf["MaxChannel"] : (int)SMALLWORLD_MAX_CHANNEL;
  m_conf.maxChannelPlayer = conf.find("MaxChannelPlayer") ? conf["MaxChannelPlayer"] : (int)SMALLWORLD_MAX_CHANNEL_PLAYER;

  m_conf.maxChannel = std::max(0, std::min(m_conf.maxChannel, (int)SMALLWORLD_MAX_CHANNEL));
  m_conf.maxChannelPlayer = std::max(0, std::min(m_conf.maxChannelPlayer, (int)SMALLWORLD_MAX_CHANNEL_PLAYER));
  m_conf.maxPlayer = std::max(0, std::min(m_conf.maxPlayer, (int)SMALLWORLD_MAX_PLAYER));

  if (!m_conf.addrAccount.empty()) {
    m_acClient.m_pClient = NetworkClient::alloc(&m_acClient);
    if (0 == m_acClient.m_pClient) {
      return false;
    }
  }

  m_stage.popAll();
  m_stage.initialize(this, &implSmallworldServer::stageInitialization);

  return true;
}

void implSmallworldServer::shutdown()
{
  m_stage.popAll();
  m_stage.initialize(this, &implSmallworldServer::stageDummy);

  if (0 != m_acClient.m_pClient) {
    NetworkClient::free(m_acClient.m_pClient);
    m_acClient.m_pClient = 0;
  }
}

void implSmallworldServer::trigger()
{
  if (m_acClient.m_pClient) {
    m_acClient.m_pClient->trigger();
  }

  m_stage.trigger();
}

NetworkServerStats implSmallworldServer::getNetStats()
{
  return m_pServer->getNetStats();
}

SmallworldPlayer* implSmallworldServer::getFirstPlayer()
{
  int iter = m_player.first();
  if (-1 == iter) {
    return 0;
  } else {
    return &m_player[iter];
  }
}

SmallworldPlayer* implSmallworldServer::getNextPlayer(SmallworldPlayer* pPlayer)
{
  if (0 == pPlayer) {
    return 0;
  }

  int iter = m_player.next(pPlayer->getPlayerId());
  if (-1 == iter) {
    return 0;
  } else {
    return &m_player[iter];
  }
}

SmallworldGame* implSmallworldServer::getFirstGame()
{
  int iter = m_game.first();
  if (-1 == iter) {
    return 0;
  } else {
    return &m_game[iter];
  }
}

SmallworldGame* implSmallworldServer::getNextGame(SmallworldGame* pGame)
{
  if (0 == pGame) {
    return 0;
  }

  int iter = m_game.next(pGame->getGameId());
  if (-1 == iter) {
    return 0;
  } else {
    return &m_game[iter];
  }
}

void implSmallworldServer::onNetworkServerStartup(NetworkServer*)
{
  m_pCallback->onSmallworldServerStartup(this);
  m_stage.push(&implSmallworldServer::stageReady);
}

void implSmallworldServer::onNetworkServerShutdown(NetworkServer*)
{
  m_pCallback->onSmallworldServerShutdown(this);
}

bool implSmallworldServer::onNetworkNewClientReady(NetworkServer*, NetworkConnection* pNewClient)
{
  //
  // Check is server busy?
  //

  if (m_player.size() == m_conf.maxPlayer) {
    evSmallworldNotify n;
    n.code = evSmallworldNotify::NC_SERVER_BUSY;
    pNewClient->send(n);                // Ignore send fail.
    return false;                       // Not accept this connection.
  }

  //
  // Allocate a free space for this new connection.
  //

  int id = m_player.alloc();
  assert(-1 != id);

  //
  // Notify need login command no matter need an account server or not.
  //

  evSmallworldNotify n;
  n.code = evSmallworldNotify::NC_NEED_LOGIN;

  if (!pNewClient->send(n)) {
    m_player.free(id);
    return false;
  }

  //
  // Setup this connection.
  //

  implSmallworldServerPlayer& peer = m_player[id];
  peer.m_idPlayer = id;
  peer.m_bWait4Login = true;
  peer.m_bVerified = peer.m_bNeedPlayerList = peer.m_bNeedGameList = peer.m_bNeedMessage = false;
  peer.userData = (uint_ptr)0;
  peer.m_pServer = this;
  peer.m_pNetPeer = pNewClient;
  peer.m_idGame = peer.m_iChannel = peer.m_idChannel = peer.m_idGameSeat = -1;
  pNewClient->userData = (uint_ptr)id;

  peer.m_stage.initialize(&peer, &implSmallworldServerPlayer::stageWait4Login);

  return true;                          // Accept this new connection.
}

void implSmallworldServer::onNetworkClientLeave(NetworkServer*, NetworkConnection* pClient)
{
  //
  // Enter disconnecting process.
  //

  int id = (int)pClient->userData;
  m_player[id].m_stage.popAndPush(&implSmallworldServerPlayer::stageDisconnecting);
}

void implSmallworldServer::onNetworkPacketReady(NetworkServer*, NetworkConnection* pClient, const NetworkPacket& p)
{
  //
  // Pass to client peer's stage controller to valid the event.
  //

  int id = (int)pClient->userData;
  m_player[id].m_stage.trigger((uint_ptr)(intptr_t)&p);
}

void implSmallworldServer::onNetworkStreamReady(NetworkServer*, NetworkConnection* pClient, int len, void const* pStream)
{
  //
  // TODO: valid the stream by correct stage.
  //

  int id = (int)pClient->userData;
  m_pCallback->onSmallworldStreamReady(this, &m_player[id], len, pStream);
}

void implSmallworldServer::stageDummy(int, uint_ptr)
{
}

void implSmallworldServer::stageInitialization(int state, uint_ptr)
{
  if (JOIN == state) {
    m_bReady2Go = false;
  }

  if (TRIGGER == state) {
    if (!m_conf.addrAccount.empty()) {
      m_stage.push(&implSmallworldServer::stagePhaseAccount);
    } else {
      m_stage.push(&implSmallworldServer::stageStartup);
    }
  }

  if (LEAVE == state) {
    m_bReady2Go = true;
  }
}

void implSmallworldServer::stagePhaseAccount(int state, uint_ptr)
{
  assert(m_acClient.m_pClient);

  if (JOIN == state) {
    SW2_TRACE_MESSAGE("Connect Account Server...");
    if (!m_acClient.m_pClient->connect(m_conf.addrAccount)) {
      SW2_TRACE_ERROR("Connect Account Server Failed!!!");
    }
    m_timer.setTimeout(SMALLWORLD_TIMEOUT_LOGIN);
  }

  if (TRIGGER == state) {

    //
    // Check connect and login timeout.
    //

    if (m_timer.isExpired()) {
      SW2_TRACE_ERROR("Timeout, Retry...");
      m_stage.popAndPush(&implSmallworldServer::stagePhaseAccount);
    }
  }

  if (LEAVE == state)
  {
    //
    // Disconnect and wait until disconnected.
    //

    m_acClient.m_pClient->disconnect();
    while (CS_DISCONNECTED != m_acClient.m_pClient->getConnectionState()) {
      m_acClient.m_pClient->trigger();
    }
  }
}

void implSmallworldServer::stageStartup(int state, uint_ptr)
{
  if (JOIN == state) {
    SW2_TRACE_MESSAGE("Startup Server...");
    if (!m_pServer->startup(m_conf.addrListen)) {
      SW2_TRACE_ERROR("Startup Server Failed!!!");
    }
  }

  if (LEAVE == state) {

    //
    // Shutdown to avoid new connection.
    //

    m_pServer->shutdown();

    //
    // Disconnect all online players.
    //

    for (int i = m_player.first(); -1 != i; i = m_player.next(i)) {
      if (0 != m_player[i].m_pNetPeer) {
        m_player[i].m_pNetPeer->disconnect();
      }
    }

    //
    // Wait for all players logged out.
    //

    while (m_player.size()) {

      //
      // Trigger server.
      //

      m_pServer->trigger();

      if (m_acClient.m_pClient) {
        m_acClient.m_pClient->trigger();
      }

      //
      // Trigger client connections.
      //

      for (int i = m_player.first(); -1 != i; ) {
        int next = m_player.next(i);
        m_player[i].m_stage.trigger();
        i = next;
      }
    }
  }
}

void implSmallworldServer::stageReady(int state, uint_ptr)
{
  if (JOIN == state) {
    m_bReady = true;
  }

  if (TRIGGER == state) {

    //
    // Is account alive?
    //

    if (m_acClient.m_pClient && CS_CONNECTED != m_acClient.m_pClient->getConnectionState()) {
      m_stage.popAndPush(&implSmallworldServer::stagePhaseAccount, 3);
      return;
    }

    //
    // Trigger server.
    //

    m_pServer->trigger();

    //
    // Trigger client connections.
    //

    for (int i = m_player.first(); -1 != i; ) {
      int next = m_player.next(i);
      m_player[i].m_stage.trigger();
      i = next;
    }
  }

  if (LEAVE == state) {
    m_bReady = false;
  }
}

implSmallworldServerAccountClient::implSmallworldServerAccountClient() : m_pServer(0), m_pClient(0)
{
}

void implSmallworldServerAccountClient::onNetworkServerLeave(NetworkClient* pClient)
{
  SW2_TRACE_ERROR("Lost Connection with Account Server");
}

void implSmallworldServerAccountClient::onNetworkPacketReady(NetworkClient* pClient, const NetworkPacket& p)
{
  assert(m_pServer);

  if (EID_NOTIFY == p.getId()) {
    evSmallworldNotify* pNotify = (evSmallworldNotify*)&p;
    switch (pNotify->code)
    {
    case evSmallworldNotify::NC_NEED_LOGIN:
      {
        evSmallworldLogin el;
        assert(m_pClient);
        if (!m_pClient->send(el)) {
          m_pClient->disconnect();
        }
      }
      break;

    case evSmallworldNotify::NC_LOGIN_ACCEPTED:
      m_pServer->m_stage.push(&implSmallworldServer::stageStartup);
      break;

    default:
      SW2_TRACE_WARNING("Unknown notify code received, ignore"); // Ignore.
      break;
    }

    return;
  }

  if (EID_REQUEST == p.getId()) {
    evSmallworldRequest* pReq = (evSmallworldRequest*)&p;

    implSmallworldServerPlayer& peer = m_pServer->m_player[pReq->idPlayer];

    //
    // Is still alive?
    //

    if (0 != peer.m_pNetPeer) {

      //
      // Verify time stamp, because it is internal verification so use assert.
      //

      assert(peer.m_timer.getExpiredTime() == pReq->time);

      switch (pReq->code)
      {
      case evSmallworldRequest::NC_PLAYER_LOGIN: // Login success(login).
        peer.m_bVerified = true;
        peer.m_stage.popAndPush(&implSmallworldServerPlayer::stageReady);
        break;

      case evSmallworldRequest::NC_PLAYER_LOGOUT: // Logout success(logout).
        m_pServer->m_player.free(peer.m_idPlayer);
        peer.m_stage.popAll();
        break;

      case evSmallworldRequest::NC_ACCOUNT_OR_PASSWORD: // (login)
        {
          evSmallworldNotify n;
          n.code = evSmallworldNotify::NC_ACCOUNT_OR_PASSWORD;
          peer.m_pNetPeer->send(n);
          peer.m_pNetPeer->disconnect();
        }
        break;

      case evSmallworldRequest::NC_DUPLICATE_LOGIN: // (login)
        {
          evSmallworldNotify n;
          n.code = evSmallworldNotify::NC_DUPLICATE_LOGIN;
          peer.m_pNetPeer->send(n);
          peer.m_pNetPeer->disconnect();
        }
        break;

      case evSmallworldRequest::NC_NOT_ALLOWED: // (login)
        {
          evSmallworldNotify n;
          n.code = evSmallworldNotify::NC_LOGIN_NOT_ALLOWED;
          peer.m_pNetPeer->send(n);
          peer.m_pNetPeer->disconnect();
        }
        break;

      case evSmallworldRequest::NC_NOT_LOGIN: // (logout)
        assert(0); // bug
        break;

      default:
        SW2_TRACE_WARNING("Unknown request code received, ignore"); // Ignore.
        break;
      }

      return;
    }

    //
    // Connections is dead.
    //

    switch (pReq->code)
    {
    case evSmallworldRequest::NC_PLAYER_LOGIN: // Login success(login)
      pReq->code = evSmallworldRequest::NC_PLAYER_LOGOUT;
      if (!m_pServer->m_acClient.m_pClient->send(*pReq)) { // Logout immediately.
        m_pServer->m_player.free(peer.m_idPlayer);
        peer.m_stage.popAll();
      }
      break;

    case evSmallworldRequest::NC_NOT_LOGIN: // (logout)
      assert(0);                        // Should be a bug.
      break;

    default:                            // Clean up anyway.
      m_pServer->m_player.free(peer.m_idPlayer);
      peer.m_stage.popAll();
      break;
    }

    return;
  }

  //
  // Ignore unknown event.
  //

  SW2_TRACE_WARNING("Unknown Event received, ignore");
}

implSmallworldServerPlayer::~implSmallworldServerPlayer()
{
}

void implSmallworldServerPlayer::disconnect()
{
  if (m_pNetPeer) {
    m_pNetPeer->disconnect();
  }
}

std::string implSmallworldServerPlayer::getAddr() const
{
  if (m_pNetPeer) {
    return m_pNetPeer->getAddr();
  } else {
    return "";
  }
}

NetworkClientStats implSmallworldServerPlayer::getNetStats()
{
  if (m_pNetPeer) {
    return m_pNetPeer->getNetStats();
  } else {
    return NetworkClientStats();
  }
}

bool implSmallworldServerPlayer::send(const NetworkPacket& p)
{
  if (!m_bVerified) {
    SW2_TRACE_ERROR("[LB] try to send event while not ready");
    return false;
  }

  if (0 == m_pNetPeer) {
    return false;
  }

  return m_pNetPeer->send(p);
}

bool implSmallworldServerPlayer::send(int len, void const* pStream)
{
  if (!m_bVerified) {
    SW2_TRACE_ERROR("[LB] try to send stream while not ready");
    return false;
  }

  if (0 == m_pNetPeer) {
    return false;
  }

  return m_pNetPeer->send(len, pStream);
}

bool implSmallworldServerPlayer::sendMessage(const std::string& msg)
{
  if (!m_bVerified) {
    SW2_TRACE_ERROR("[LB] try to send message while not ready");
    return false;
  }

  if (!m_bNeedMessage) {
    SW2_TRACE_ERROR("[LB] send message in need no message mode");
    return false;
  }

  if (msg.empty()) {                    // Ignore.
    return true;
  }

  //
  // Broadcast message to players in the same channel(include self).
  //

  evSmallworldChat chat;
  chat.code = evSmallworldChat::NC_CHAT_FROM;
  chat.idWho = m_idPlayer;
  chat.msg = msg;

  for (int i = m_pServer->m_channelPlayer[m_iChannel].first(); -1 != i; i = m_pServer->m_channelPlayer[m_iChannel].next(i)) {
    implSmallworldServerPlayer& peer = m_pServer->m_player[m_pServer->m_channelPlayer[m_iChannel][i]];
    if (!peer.m_bVerified || !peer.m_bNeedMessage) { // Only send to verified player.
      continue;
    }
    if (!peer.m_pNetPeer->send(chat)) {
      peer.m_pNetPeer->disconnect();
    }
  }

  return true;
}

bool implSmallworldServerPlayer::sendPrivateMessage(int idWho, const std::string& msg)
{
  if (!m_bVerified) {
    SW2_TRACE_ERROR("[LB] send pmessage while not ready");
    return false;
  }

  if (!m_bNeedMessage) {
    SW2_TRACE_ERROR("[LB] send pmessage in need no message mode");
    return false;
  }

  if (msg.empty()) {                    // Ignore.
    return true;
  }

  //
  // Send private message only when target is online and verified.
  //

  evSmallworldChat chat;
  chat.msg = msg;

  if (m_pServer->m_player.isUsed(idWho) && m_pServer->m_player[idWho].m_bVerified) {

    //
    // Send back to sender.
    //

    if (m_bNeedMessage) {
      chat.code = evSmallworldChat::NC_PM_TO;
      chat.idWho = idWho;
      if (!m_pNetPeer->send(chat)) {
        m_pNetPeer->disconnect();
      }
    }

    //
    // Send to target.
    //

    if (m_pServer->m_player[idWho].m_bNeedMessage) {
      chat.code = evSmallworldChat::NC_PM_FROM;
      chat.idWho = m_idPlayer;
      if (!m_pServer->m_player[idWho].m_pNetPeer->send(chat)) {
        m_pServer->m_player[idWho].m_pNetPeer->disconnect();
      }
    }
  } else {

    //
    // Target not found or not verified.
    //

    chat.code = evSmallworldChat::NC_PN_NOT_FOUND;
    if (!m_pNetPeer->send(chat)) {
      m_pNetPeer->disconnect();
    }
  }

  return true;
}

bool implSmallworldServerPlayer::changeChannel(int newChannel)
{
  if (!m_bVerified) {
    SW2_TRACE_ERROR("[LB] change channel while not ready");
    return false;
  }

  //
  // Channel mode not allowed?
  //

  if (!m_pServer->m_conf.bEnableChannel) {
    SW2_TRACE_ERROR("[LB] try to change channel while not in channel mode");
    m_pNetPeer->disconnect();
    return false;
  }

  //
  // Not allowed because already in game, and this must be checked and avoid by client.
  //

  if (-1 != m_idGame) {
    SW2_TRACE_ERROR("[LB] attempt to change channel in game mode");
    m_pNetPeer->disconnect();
    return false;
  }

  //
  // Should not happen, client must check and avoid.
  //

  if (newChannel == m_iChannel) {
    SW2_TRACE_ERROR("[LB] change channel with same channel id, invalid iChannel");
    m_pNetPeer->disconnect();
    return false;
  }

  //
  // Is out of space?
  //

  if (m_pServer->m_conf.maxChannelPlayer == m_pServer->m_channelPlayer[newChannel].size()) {
    evSmallworldNotify n;
    n.code = evSmallworldNotify::NC_CHANNEL_IS_FULL;
    if (!m_pNetPeer->send(n)) {
      m_pNetPeer->disconnect();
    }
    return false;
  }

  //
  // Remove from current channel to avoid receive player remove notify.
  //

  m_pServer->m_channelPlayer[m_iChannel].free(m_idChannel);

  //
  // Broadcast leave to other players.
  //

  broadcastLeaveChannel();

  //
  // Join new channel.
  //

  int iCh = m_iChannel;
  m_iChannel = newChannel;
  m_idChannel = m_pServer->m_channelPlayer[m_iChannel].alloc();
  assert(-1 != m_idChannel);
  m_pServer->m_channelPlayer[m_iChannel][m_idChannel] = m_idPlayer;

  //
  // Broadcast enter new channel to all players also notify channel change to self(client side).
  //

  broadcastEnterChannel();

  //
  // Notify channel changed.
  //

  m_pServer->m_pCallback->onSmallworldPlayerChannelChanged(m_pServer, this, newChannel, iCh);

  return true;
}

bool implSmallworldServerPlayer::newGame()
{
  if (!m_bVerified) {
    SW2_TRACE_ERROR("[LB] create game while not ready");
    return false;
  }

  //
  // Invalid action.
  //

  if (-1 != m_idGame)  {
    SW2_TRACE_ERROR("[LB] create game while already in a game");
    m_pNetPeer->disconnect();
    return false;
  }

  //
  // Create new game.
  //

  m_idGame = m_pServer->m_game.alloc();
  assert(-1 != m_idGame);

  implSmallworldServerGame& game = m_pServer->m_game[m_idGame];
  game.m_pServer = m_pServer;
  game.m_idGame = m_idGame;
  game.userData = (uint_ptr)0;

  game.m_iChannel = m_iChannel;
  game.m_idChannelGame = m_pServer->m_channelGame[m_iChannel].alloc();
  assert(-1 != game.m_idChannelGame);
  m_pServer->m_channelGame[m_iChannel][game.m_idChannelGame] = m_idGame;

  //
  // Query create new game.
  //

  if (!m_pServer->m_pCallback->onSmallworldNewGameReady(m_pServer, &m_pServer->m_game[m_idGame])) {
    m_pServer->m_channelGame[m_iChannel].free(game.m_idChannelGame);
    m_pServer->m_game.free(m_idGame);
    m_idGame = -1;
    return false;
  }

  //
  // Join game.
  //

  m_idGameSeat = game.m_players.alloc();
  assert(-1 != m_idGameSeat);
  game.m_players[m_idGameSeat] = m_idPlayer;

  //
  // Query join game.
  //

  if (!m_pServer->m_pCallback->onSmallworldPlayerJoinGame(m_pServer, &m_pServer->m_game[m_idGame], this)) {

    //
    // Join game not allowed so quit the game.
    //

    game.m_players.free(m_idGameSeat);

    //
    // Release the game.
    //

    m_pServer->m_channelGame[m_iChannel].free(game.m_idChannelGame);
    m_pServer->m_game.free(m_idGame);
    m_idGame = -1;

    return false;
  }

  //
  // Notify game list.
  //

  if (m_pServer->m_conf.bEnableGameList) {
    evSmallworldGame eg;
    eg.idGame = m_idGame;
    eg.idPlayer = m_idPlayer;

    int i = m_pServer->m_channelPlayer[m_iChannel].first();
    for (; -1 != i; i = m_pServer->m_channelPlayer[m_iChannel].next(i)) {
      const implSmallworldServerPlayer& peer = m_pServer->m_player[m_pServer->m_channelPlayer[m_iChannel][i]];

      if (!peer.m_bVerified || !peer.m_bNeedGameList || 0 == peer.m_pNetPeer) {
        continue;
      }

      //
      // Notify new game created.
      //

      eg.code = evSmallworldGame::NC_GAME_ADD;
      if (!peer.m_pNetPeer->send(eg)) {
        peer.m_pNetPeer->disconnect();
        continue;
      }

      //
      // Notify player join the game.
      //

      if (!peer.m_bNeedPlayerList && peer.m_idPlayer != m_idPlayer) {
        continue;
      }

      eg.code = evSmallworldGame::NC_PLAYER_JOIN;
      if (!peer.m_pNetPeer->send(eg)) {
        m_pNetPeer->disconnect();
      }
    }
  }

  return true;
}

bool implSmallworldServerPlayer::joinGame(int idGame)
{
  if (!m_bVerified) {
    SW2_TRACE_ERROR("[LB] try to join game while not ready");
    return false;
  }

  //
  // Invalid action.
  //

  if (-1 != m_idGame)  {
    SW2_TRACE_ERROR("[LB] join game while already in a game");
    m_pNetPeer->disconnect();
    return false;
  }

  //
  // Is the game exist or valid?
  //

  if (!m_pServer->m_game.isUsed(idGame) || m_pServer->m_game[idGame].m_iChannel != m_iChannel) {
    evSmallworldGame eg;
    eg.code = evSmallworldGame::NC_GAME_NOT_FOUND;
    if (!m_pNetPeer->send(eg)) {
      m_pNetPeer->disconnect();
    }
    return false;
  }

  //
  // Join the game.
  //

  m_idGameSeat = m_pServer->m_game[idGame].m_players.alloc();
  assert(-1 != m_idGameSeat);
  m_pServer->m_game[idGame].m_players[m_idGameSeat] = m_idPlayer;

  m_idGame = idGame;

  //
  // Is allow to join?
  //

  if (!m_pServer->m_pCallback->onSmallworldPlayerJoinGame(m_pServer, &m_pServer->m_game[idGame], this)) {
    m_pServer->m_game[idGame].m_players.free(m_idGameSeat);
    m_idGame = -1;
    return false;
  }

  //
  // Notify.
  //

  evSmallworldGame eg;
  eg.code = evSmallworldGame::NC_PLAYER_JOIN;
  eg.idGame = idGame;
  eg.idPlayer = m_idPlayer;

  int i = m_pServer->m_channelPlayer[m_iChannel].first();
  for (; -1 != i; i = m_pServer->m_channelPlayer[m_iChannel].next(i)) {
    const implSmallworldServerPlayer& peer = m_pServer->m_player[m_pServer->m_channelPlayer[m_iChannel][i]];
    if (!peer.m_bVerified || !peer.m_bNeedGameList || 0 == peer.m_pNetPeer) {
      continue;
    }
    if (!peer.m_bNeedPlayerList && peer.m_idPlayer != m_idPlayer) {
      continue;
    }
    if (!peer.m_pNetPeer->send(eg)) {
      m_pNetPeer->disconnect();
    }
  }

  return true;
}

bool implSmallworldServerPlayer::quitGame()
{
  if (!m_bVerified) {
    SW2_TRACE_ERROR("[LB] try to quit game while not ready");
    return false;
  }

  //
  // Invalid action.
  //

  if (-1 == m_idGame) {
    SW2_TRACE_ERROR("[LB] quit game while not in a game");
    m_pNetPeer->disconnect();
    return false;
  }

  //
  // Do quit the game and notify leave game.
  //

  m_pServer->m_game[m_idGame].m_players.free(m_idGameSeat); // Remove self from the game.

  //
  // Keep or destroy the game?
  //

  if (m_pServer->m_pCallback->onSmallworldPlayerLeaveGame(m_pServer, &m_pServer->m_game[m_idGame], this) &&
      0 != m_pServer->m_game[m_idGame].m_players.size()) {
    evSmallworldGame eg;
    eg.code = evSmallworldGame::NC_PLAYER_LEAVE;
    eg.idGame = m_idGame;
    eg.idPlayer = m_idPlayer;

    int i = m_pServer->m_channelPlayer[m_iChannel].first();
    for (; -1 != i; i = m_pServer->m_channelPlayer[m_iChannel].next(i)) {
      const implSmallworldServerPlayer& peer = m_pServer->m_player[m_pServer->m_channelPlayer[m_iChannel][i]];
      if (!peer.m_bVerified || !peer.m_bNeedGameList || 0 == peer.m_pNetPeer) {
        continue;
      }
      if (!peer.m_bNeedPlayerList && peer.m_idPlayer != m_idPlayer) {
        continue;
      }
      if (!peer.m_pNetPeer->send(eg)) {
        m_pNetPeer->disconnect();
      }
    }
  } else  {

    //
    // Destroy the game if no player in the game and notify game remove.
    //

    evSmallworldGame eg;
    eg.code = evSmallworldGame::NC_GAME_REMOVE;
    eg.idGame = m_idGame;

    int i = m_pServer->m_channelPlayer[m_iChannel].first();
    for (; -1 != i; i = m_pServer->m_channelPlayer[m_iChannel].next(i)) {
      const implSmallworldServerPlayer& peer = m_pServer->m_player[m_pServer->m_channelPlayer[m_iChannel][i]];
      if (!peer.m_bVerified || !peer.m_bNeedGameList || 0 == peer.m_pNetPeer) {
        continue;
      }
      if (!peer.m_pNetPeer->send(eg)) {
        m_pNetPeer->disconnect();
      }
    }

    //
    // Remove remain player in the game.
    //

    i = m_pServer->m_game[m_idGame].m_players.first();
    for (; -1 != i; i = m_pServer->m_game[m_idGame].m_players.next(i)) {
      m_pServer->m_player[m_pServer->m_game[m_idGame].m_players[i]].m_idGame = -1;
    }

    m_pServer->m_game[m_idGame].m_players.clear();

    //
    // Release the game.
    //

    m_pServer->m_pCallback->onSmallworldGameLeave(m_pServer, &m_pServer->m_game[m_idGame]);
    m_pServer->m_channelGame[m_iChannel].free(m_pServer->m_game[m_idGame].m_idChannelGame);
    m_pServer->m_game.free(m_idGame);
  }

  m_idGame = -1;

  return true;
}

void implSmallworldServerPlayer::stageDisconnecting(int state, uint_ptr)
{
  assert(m_pServer);

  if (JOIN == state) {

    // Logout to Account Server if necessary.
    //

    if (!m_pServer->m_conf.addrAccount.empty()) { // Account server needed.
      if (m_bWait4Login || !m_pServer->m_bReady) {
        m_pServer->m_player.free(m_idPlayer);
        m_stage.popAll();
      } else if (m_bVerified) {
        evSmallworldRequest req;
        req.code = evSmallworldRequest::NC_PLAYER_LOGOUT;
        req.idPlayer = m_idPlayer;      // Verify code 1.
        req.time = Util::getTickCount(); // Verify code 2.
        req.stream = m_stream;

        //
        // Pass the logout event to account server, and wait reply...
        //

        assert(m_pServer->m_acClient.m_pClient);
        if (!m_pServer->m_acClient.m_pClient->send(req)) {
          m_pServer->m_player.free(m_idPlayer);
          m_stage.popAll();
        }

        m_timer.setExpiredTime(req.time);
        m_bVerified = false;            // Avoid to send again.
      }
    } else {

      //
      // No Account Server needed.
      //

      m_pServer->m_player.free(m_idPlayer);
      m_stage.popAll();
    }

    //
    // Mark as dead connection.
    //

    m_pNetPeer = 0;

    //
    // Disconnect timeout.
    //

    m_timer.setTimeout(8000);
  }

  if (TRIGGER == state) {
    if (m_timer.isExpired()) {          // TODO, to find a better way to handle this!!!
      m_pServer->m_player.free(m_idPlayer);
      m_stage.popAll();
    }
  }
}

void implSmallworldServerPlayer::stageReady(int state, uint_ptr pEvent)
{
  if (JOIN == state) {
    initReadyStage();
    return;
  }

  if (TRIGGER == state) {

    //
    // Handle events.
    //

    handleReadyStageEvent((NetworkPacket*)(intptr_t)pEvent);

    return;
  }

  if (LEAVE == state) {
    uninitReadyStage();
  }
}

void implSmallworldServerPlayer::stageWait4AccountServerReply(int state, uint_ptr pEvent)
{
  if (TRIGGER == state && pEvent) {
    SW2_TRACE_ERROR("[LB] IE received in W4AccReply");
    assert(m_pNetPeer);
    m_pNetPeer->disconnect();
  }
}

void implSmallworldServerPlayer::handleWait4LoginEvent(evSmallworldLogin *pLogin)
{
  //
  // Only login event is valid, otherwise kick this connection.
  //

  if (EID_LOGIN != pLogin->getId()) {
    SW2_TRACE_ERROR("[LB] IE received in W4L");
    m_pNetPeer->disconnect();
    return;
  }

  if (SMALLWORLD_VERSION_MAJOR != pLogin->verMajor ||
      SMALLWORLD_VERSION_MINOR != pLogin->verMinor) {
    evSmallworldNotify n;
    n.code = evSmallworldNotify::NC_VERSION_MISMATCH;
    m_pNetPeer->send(n);            // Ignore failed.
    m_pNetPeer->disconnect();
    return;
  }

  m_bNeedPlayerList = pLogin->bNeedPlayerList;
  m_bNeedGameList = pLogin->bNeedGameList;
  m_bNeedMessage = pLogin->bNeedMessage;
  m_stream = pLogin->stream;

  //
  // Need account server to verify?
  //

  if (!m_pServer->m_conf.addrAccount.empty())  {
    assert(m_pServer->m_acClient.m_pClient);

    evSmallworldRequest req;
    req.code = evSmallworldRequest::NC_PLAYER_LOGIN;
    req.idPlayer = m_idPlayer;      // Verify code 1.
    req.time = Util::getTickCount(); // Verify code 2.
    req.stream = m_stream;

    //
    // Pass the login event to account server, and wait reply...
    //

    if (!m_pServer->m_acClient.m_pClient->send(req)) {
      m_pNetPeer->disconnect();
      return;
    }

    m_timer.setExpiredTime(req.time);
    m_stage.popAndPush(&implSmallworldServerPlayer::stageWait4AccountServerReply);

  } else {

    //
    // Need an account server.
    //

    m_stage.popAndPush(&implSmallworldServerPlayer::stageReady);
  }

  m_bWait4Login = false;
}

void implSmallworldServerPlayer::stageWait4Login(int state, uint_ptr pEvent)
{
  if (JOIN == state) {
    m_timer.setTimeout(SMALLWORLD_TIMEOUT_LOGIN); // Setup wait for login timeout timer.
    return;
  }

  if (TRIGGER == state) {

    //
    // Check event.
    //

    if (pEvent) {
      handleWait4LoginEvent((evSmallworldLogin*)(intptr_t)pEvent);
      return;
    }

    //
    // Check wait for login timeout timer.
    //

    if (m_timer.isExpired()) {
      assert(m_pNetPeer);
      m_pNetPeer->disconnect();
    }
  }
}

void implSmallworldServerPlayer::handleReadyStageEvent(NetworkPacket *pEvent)
{
  if (0 == pEvent) {
    return;
  }

  switch (pEvent->getId())
  {
  case EID_CHANNEL:                 // Internal.
    handleChannelEvent((evSmallworldChannel*)pEvent);
    break;

  case EID_CHAT:                    // Internal.
    handleChatEvent((evSmallworldChat*)pEvent);
    break;

  case EID_GAME:                    // Internal.
    handleGameEvent((evSmallworldGame*)pEvent);
    break;

  default:                          // User defined.
    handleUserEvent((NetworkPacket*)pEvent);
    break;
  }
}

void implSmallworldServerPlayer::handleChannelEvent(evSmallworldChannel* pChannel)
{
  //
  // Only NC_CHANGE code is valid.
  //

  if (evSmallworldChannel::NC_CHANGE != pChannel->code) {
    SW2_TRACE_ERROR("[LB] IC received in Ready/Channel");
    m_pNetPeer->disconnect();
    return;
  }

  changeChannel(pChannel->iChannel);
}

void implSmallworldServerPlayer::handleChatEvent(evSmallworldChat* pChat)
{
  if (!m_bNeedMessage) {
    SW2_TRACE_ERROR("[LB] IA received in Ready/Chat, need no message");
    m_pNetPeer->disconnect();
    return;
  }

  switch (pChat->code)
  {
  case evSmallworldChat::NC_CHAT:
    sendMessage(pChat->msg);
    break;

  case evSmallworldChat::NC_PM_TO:
    sendPrivateMessage(pChat->idWho, pChat->msg);
    break;

  default:
    SW2_TRACE_ERROR("[LB] IC received in Ready/Chat");
    m_pNetPeer->disconnect();
    break;
  }
}

void implSmallworldServerPlayer::handleGameEvent(evSmallworldGame* pGame)
{
  switch (pGame->code)
  {
  case evSmallworldGame::NC_NEW:
    newGame();
    break;

  case evSmallworldGame::NC_JOIN:
    joinGame(pGame->idGame);
    break;

  case evSmallworldGame::NC_QUIT:
    quitGame();
    break;

  default:
    SW2_TRACE_ERROR("[LB] IC received in Ready/Game");
    m_pNetPeer->disconnect();
    break;
  }
}

void implSmallworldServerPlayer::handleUserEvent(NetworkPacket* pEvent)
{
  m_pServer->m_pCallback->onSmallworldPacketReady(m_pServer, this, *pEvent);
}

void implSmallworldServerPlayer::initReadyStage()
{
  m_idGame = -1;
  m_iChannel = m_idChannel = -1;

  if (m_pServer->m_conf.addrAccount.empty()) {
    m_bVerified = true;
  }

  //
  // Notify client login successful.
  //

  evSmallworldNotify n;
  n.code = evSmallworldNotify::NC_LOGIN_ACCEPTED;
  n.id = m_idPlayer;

  if (!m_pNetPeer->send(n)) {           // Kick out if send failed.
    m_pNetPeer->disconnect();
    return;
  }

  //
  // Select and put in a channel.
  //

  if (m_pServer->m_conf.bEnableChannel) { // Try to select a channel with least players.
    m_iChannel = 0;
    for (int i = 1; i < m_pServer->m_conf.maxChannel; i++) {
      if (m_pServer->m_channelPlayer[i].size() >= m_pServer->m_channelPlayer[m_iChannel].size()) {
        continue;
      }
      m_iChannel = i;
    }
  } else {                              // If not use channel mode, always select channel 0.
    m_iChannel = 0;
  }

  m_idChannel = m_pServer->m_channelPlayer[m_iChannel].alloc(); // Put into the selected channel.
  assert(-1 != m_idChannel);            // Should not happened.
  m_pServer->m_channelPlayer[m_iChannel][m_idChannel] = m_idPlayer;

  //
  // Notify player ready.
  //

  m_bAccept = true;
  if (!m_pServer->m_pCallback->onSmallworldNewPlayerReady(m_pServer, this)) { // Accept?

    //
    // To avoid notify a player leave.
    //

    m_bAccept = false;

    //
    // Remove from selected channel.
    //

    m_pServer->m_channelPlayer[m_iChannel].free(m_idChannel);
    m_iChannel = m_idChannel = -1;

    m_pNetPeer->disconnect();

    return;
  }

  //
  // Notify player ready to all online and verified players in the channel.
  //

  broadcastEnterChannel();

  //
  // Notify player channel changed. New added to the channel.
  //

  m_pServer->m_pCallback->onSmallworldPlayerChannelChanged(m_pServer, this, m_iChannel, -1);
}

void implSmallworldServerPlayer::uninitReadyStage()
{
  m_bNeedPlayerList = m_bNeedGameList = m_bNeedMessage = false;

  //
  // Notify player leave.
  //

  if (m_bAccept) {
    m_pServer->m_pCallback->onSmallworldPlayerLeave(m_pServer, this);
  }

  //
  // Quit from joined game if in a game.
  //

  if (-1 != m_idGame) {
    quitGame();
  }

  //
  // Remove from select channel if in a channel.
  //

  if (-1 != m_iChannel) {

    //
    // Remove from current channel.
    //

    m_pServer->m_channelPlayer[m_iChannel].free(m_idChannel);

    //
    // Notify player leave to players in the channel except self.
    //

    broadcastLeaveChannel();

    m_iChannel = m_idChannel = -1;
  }

  if (m_pServer->m_conf.addrAccount.empty()) {
    m_bVerified = false;
  }
}

void implSmallworldServerPlayer::broadcastEnterChannel()
{
  assert(m_pServer);

  if (!m_pServer->m_bReady) {
    return;
  }

  //
  // Player list enable and server is ready?
  //

  if (m_pServer->m_conf.bEnablePlayerList) {
    evSmallworldChannel ch;

    //
    // Send a channel change notify to self.
    //

    ch.code = evSmallworldChannel::NC_CHANGE;
    ch.iChannel = m_iChannel;

    if (!m_pNetPeer->send(ch)) {
      m_pNetPeer->disconnect();
      return;
    }

    //
    // Send player list to self except self.
    //

    if (m_bNeedPlayerList) {
      ch.code = evSmallworldChannel::NC_PLAYER_ADD;
      int iter = m_pServer->m_channelPlayer[m_iChannel].first();
      for (; -1 != iter; iter = m_pServer->m_channelPlayer[m_iChannel].next(iter)) {
        const implSmallworldServerPlayer& peer = m_pServer->m_player[m_pServer->m_channelPlayer[m_iChannel][iter]];
        if (!peer.m_bVerified || 0 == peer.m_pNetPeer || m_idPlayer == peer.m_idPlayer) {
          continue;
        }
        ch.idPlayer = peer.m_idPlayer;
        if (!m_pNetPeer->send(ch)) {
          m_pNetPeer->disconnect();
          return;
        }
      }
    }

    //
    // Send player add to all players include self.
    //

    ch.code = evSmallworldChannel::NC_PLAYER_ADD;
    ch.idPlayer = m_idPlayer;

    int iter = m_pServer->m_channelPlayer[m_iChannel].first();
    for (; -1 != iter; iter = m_pServer->m_channelPlayer[m_iChannel].next(iter)) {
      const implSmallworldServerPlayer& peer = m_pServer->m_player[m_pServer->m_channelPlayer[m_iChannel][iter]];
      if (!peer.m_bVerified || !peer.m_bNeedPlayerList || 0 == peer.m_pNetPeer) {
        continue;
      }
      if (!peer.m_pNetPeer->send(ch)) {
        peer.m_pNetPeer->disconnect();
      }
    }
  }

  //
  // Game list.
  //

  if (m_pServer->m_conf.bEnableGameList && m_bNeedGameList) {
    evSmallworldGame eg;

    int i = m_pServer->m_channelGame[m_iChannel].first();
    for (; -1 != i; i = m_pServer->m_channelGame[m_iChannel].next(i)) {
      eg.code = evSmallworldGame::NC_GAME_ADD;
      eg.idGame = m_pServer->m_channelGame[m_iChannel][i];
      if (!m_pNetPeer->send(eg)) {
        m_pNetPeer->disconnect();
        return;
      }

      if (!m_bNeedPlayerList) {
        continue;
      }

      eg.code = evSmallworldGame::NC_PLAYER_JOIN;

      int j = m_pServer->m_game[eg.idGame].m_players.first();
      for (; -1 != j; j = m_pServer->m_game[eg.idGame].m_players.next(j)) {
        eg.idPlayer = m_pServer->m_game[eg.idGame].m_players[j];
        if (!m_pNetPeer->send(eg)) {
          m_pNetPeer->disconnect();
          return;
        }
      }
    }
  }
}

void implSmallworldServerPlayer::broadcastLeaveChannel()
{
  assert(m_pServer);

  if (!m_pServer->m_bReady) {
    return;
  }

  if (m_pServer->m_conf.bEnablePlayerList) {
    evSmallworldChannel ch;
    ch.code = evSmallworldChannel::NC_PLAYER_REMOVE;
    ch.idPlayer = m_idPlayer;
    int i = m_pServer->m_channelPlayer[m_iChannel].first();
    for (; -1 != i; i = m_pServer->m_channelPlayer[m_iChannel].next(i)) {
      implSmallworldServerPlayer& peer = m_pServer->m_player[m_pServer->m_channelPlayer[m_iChannel][i]];
      if (!peer.m_bVerified || !peer.m_bNeedPlayerList || 0 == peer.m_pNetPeer || m_idPlayer == peer.m_idPlayer) {
        continue;
      }
      if (!peer.m_pNetPeer->send(ch)) {
        peer.m_pNetPeer->disconnect();
      }
    }
  }
}

int implSmallworldServerPlayer::getPlayerId() const
{
  return m_idPlayer;
}

int implSmallworldServerPlayer::getChannelId() const
{
  return m_iChannel;
}

std::string implSmallworldServerPlayer::getLoginData() const
{
  return m_stream;
}

SmallworldGame* implSmallworldServerPlayer::getGame()
{
  return -1 == m_idGame ? 0 : &m_pServer->m_game[m_idGame];
}

} // namespace impl

SmallworldServer* SmallworldServer::alloc(SmallworldServerCallback* pCallback)
{
  assert(pCallback);
  impl::implSmallworldServer* p = new impl::implSmallworldServer(pCallback);
  if (p && !p->init()) {
    delete p;
    p = 0;
  }
  return p;
}

void SmallworldServer::free(SmallworldServer* pInstance)
{
  impl::implSmallworldServer* p = (impl::implSmallworldServer*)pInstance;
  if (p) {
    p->uninit();
  }

  delete p;
}

} // namespace sw2

// end of swSmallworldServer.cpp
