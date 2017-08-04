
//
//  Smallworld Client implementation.
//
//  Copyright (c) 2014 Waync Cheng.
//  All Rights Reserved.
//
//  2014/03/02 Waync created.
//

#include "swSmallworldEv.h"

#include "swObjectPool.h"
#include "swStageStack.h"
#include "swUtil.h"

namespace sw2 {

namespace impl {

//
// Internal constants.
//

#define SMALLWORLD_TIMEOUT_CONNECTING 12000 // Connecting timeout.

//
// Config.
//

struct CONFIG_CLIENT
{
  std::string addrServer;               // Server address, format: IP:Port.
  bool bNeedPlayerList;                 // Is enable player list?
  bool bNeedGameList;                   // Is enable game list?
  bool bNeedMessage;                    // Is enable chat message?
};

//
// Implementation.
//

class implSmallworldClient;

class implSmallworldClientPlayer : public SmallworldPlayer
{
public:

  virtual int getPlayerId() const
  {
    return m_idPlayer;
  }

  virtual int getChannelId() const
  {
    return m_iChannel;
  }

  virtual std::string getLoginData() const
  {
    return "";
  }

  virtual SmallworldGame* getGame();

  //
  // NOP.
  //

  virtual void disconnect()
  {
  }

  virtual std::string getAddr() const
  {
    return "";
  }

  virtual NetworkClientStats getNetStats()
  {
    return NetworkClientStats();
  }

  virtual bool send(const NetworkPacket& p)
  {
    return false;
  }

  virtual bool send(std::string const& s)
  {
    return false;
  }

  virtual bool sendMessage(const std::string& msg)
  {
    return false;
  }

  virtual bool sendPrivateMessage(int idWho, const std::string& msg)
  {
    return false;
  }

  virtual bool changeChannel(int newChannel)
  {
    return false;
  }

  virtual bool newGame()
  {
    return false;
  }

  virtual bool joinGame(int idGame)
  {
    return false;
  }

  virtual bool quitGame()
  {
    return false;
  }

public:

  int m_idPlayer;                       // This player ID.
  int m_iChannel;                       // Channel index.
  int m_idGame;                         // Joined game.
  implSmallworldClient* m_pClient;      // Reference.
};

class implSmallworldClientGame : public SmallworldGame
{
public:

  virtual int getGameId() const
  {
    return m_idGame;
  }

  virtual int getChannelId() const
  {
    return m_iChannel;
  }

  virtual SmallworldPlayer* getFirstPlayer();
  virtual SmallworldPlayer* getNextPlayer(SmallworldPlayer* pPlayer);

public:

  int m_idGame;                         // This game ID.
  int m_iChannel;                       // Channel index.
  implSmallworldClient* m_pClient;      // Reference.
};

class implSmallworldClient : public SmallworldClient, public NetworkClientCallback
{
public:

  explicit implSmallworldClient(SmallworldClientCallback* pCallback) : m_pCallback(pCallback), m_pClient(0), m_bVerified(false), m_idPlayer(-1), m_iChannel(-1)
  {
    SmallworldPlayer::userData = 0;
  }

  virtual ~implSmallworldClient()
  {
  }

  //
  // Initialization.
  //

  bool init()
  {
    m_pClient = NetworkClient::alloc(this);
    if (0 == m_pClient) {
      return false;
    }

    m_stage.initialize(this, &implSmallworldClient::stageDisconnected);

    return true;
  }

  void uninit()
  {
    NetworkClient::free(m_pClient);
    m_pClient = 0;
    m_stage.popAll();
  }

  //
  // Property.
  //

  virtual int getPlayerId() const
  {
    return m_idPlayer;
  }

  virtual int getChannelId() const
  {
    return m_iChannel;
  }

  virtual std::string getLoginData() const
  {
    return m_stream;
  }

  //
  // Login/out.
  //

  virtual bool login(Ini const& conf1, std::string const& ins)
  {
    Ini conf = conf1;
    m_conf.addrServer = conf["AddrServer"].value;
    m_conf.bNeedGameList = conf["NeedGameList"];
    m_conf.bNeedMessage = conf["NeedMessage"];
    m_conf.bNeedPlayerList = conf["NeedPlayerList"];

    int len = (int)ins.length();
    if (0 > len) {
      len = 0;
    }
    assert(0 <= len && SMALLWORLD_MAX_LOGIN_STREAM_LENGTH >= len);

    if (CS_DISCONNECTED != m_pClient->getConnectionState()) {
      return false;
    }

    m_stream = ins;

    if (!m_pClient->connect(m_conf.addrServer)) {
      return false;
    }

    m_stage.popAndPush(&implSmallworldClient::stageConnecting);

    return true;
  }

  virtual void logout()
  {
    disconnect();
  }

  virtual void disconnect()
  {
    m_stage.popAndPush(&implSmallworldClient::stageDisconnecting);
  }

  virtual std::string getAddr() const
  {
    return m_pClient->getAddr();
  }

  //
  // Trigger.
  //

  virtual void trigger()
  {
    m_pClient->trigger();
    m_stage.trigger();
  }

  //
  // Stats.
  //

  virtual NetworkClientStats getNetStats()
  {
    return m_pClient->getNetStats();
  }

  //
  // User command.
  //

  virtual bool send(const NetworkPacket& p)
  {
    return m_pClient->send(p);
  }

  virtual bool send(std::string const& s)
  {
    return m_pClient->send(s);
  }

  //
  // Channel chat.
  //

  virtual bool sendMessage(const std::string& msg)
  {
    if (!m_conf.bNeedMessage) {
      SW2_TRACE_ERROR("send message not allowed in need no message mode");
      return false;
    }

    if (CS_CONNECTED != m_pClient->getConnectionState() || !m_bVerified) {
      SW2_TRACE_ERROR("send message when server is not ready");
      return false;
    }

    evSmallworldChat ec;
    ec.code = evSmallworldChat::NC_CHAT;
    ec.msg = msg;

    return m_pClient->send(ec);
  }

  virtual bool sendPrivateMessage(int idWho, const std::string& msg)
  {
    if (0 == m_pClient) {
      return false;
    }

    if (!m_conf.bNeedMessage) {
      SW2_TRACE_ERROR("send pmessage not allowed in need no message mode");
      return false;
    }

    if (CS_CONNECTED != m_pClient->getConnectionState() || !m_bVerified) {
      SW2_TRACE_ERROR("send pmessage when server is not ready");
      return false;
    }

    evSmallworldChat ec;
    ec.code = evSmallworldChat::NC_PM_TO;
    ec.idWho = idWho;
    ec.msg = msg;

    return m_pClient->send(ec);
  }

  //
  // Channel.
  //

  virtual bool changeChannel(int newChannel)
  {
    if (CS_CONNECTED != m_pClient->getConnectionState() || !m_bVerified) {
      SW2_TRACE_ERROR("change channel when server is not ready");
      return false;
    }

    if (newChannel == m_iChannel) {
      return true;
    }

    if (-1 != m_poolPlayer[m_idPlayer].m_idGame) {
      SW2_TRACE_ERROR("change channel not allowed in game mode");
      return false;
    }

    evSmallworldChannel ec;
    ec.code = evSmallworldChannel::NC_CHANGE;
    ec.iChannel = newChannel;

    return m_pClient->send(ec);
  }

  //
  // Game.
  //

  virtual bool newGame()
  {
    if (CS_CONNECTED != m_pClient->getConnectionState() || !m_bVerified) {
      SW2_TRACE_ERROR("create new game while server is not ready");
      return false;
    }

    if (-1 != m_poolPlayer[m_idPlayer].m_idGame) {
      SW2_TRACE_ERROR("create new game while alreay in game mode");
      return false;
    }

    evSmallworldGame eg;
    eg.code = evSmallworldGame::NC_NEW;

    return m_pClient->send(eg);
  }

  virtual bool joinGame(int idGame)
  {
    if (CS_CONNECTED != m_pClient->getConnectionState() || !m_bVerified) {
      SW2_TRACE_ERROR("join game while server is not ready");
      return false;
    }

    if (!m_poolGame.isUsed(idGame)) {
      SW2_TRACE_ERROR("join game with an invalid game id");
      return false;
    }

    if (-1 != m_poolPlayer[m_idPlayer].m_idGame) {
      SW2_TRACE_ERROR("join game while alreay in game mode");
      return false;
    }

    evSmallworldGame eg;
    eg.code = evSmallworldGame::NC_JOIN;
    eg.idGame = idGame;

    return m_pClient->send(eg);
  }

  virtual bool quitGame()
  {
    if (CS_CONNECTED != m_pClient->getConnectionState() || !m_bVerified) {
      SW2_TRACE_ERROR("quit game whlie server is not ready");
      return false;
    }

    if (-1 == m_poolPlayer[m_idPlayer].m_idGame) {
      SW2_TRACE_ERROR("quit game while not in game mode");
      return false;
    }

    evSmallworldGame eg;
    eg.code = evSmallworldGame::NC_QUIT;

    return m_pClient->send(eg);
  }

  //
  // Property.
  //

  virtual SmallworldGame* getGame()
  {
    if (-1 == m_idPlayer) {
      return 0;
    }

    int idGame = m_poolPlayer[m_idPlayer].m_idGame;
    if (-1 == idGame) {
      return 0;
    } else {
      return &m_poolGame[idGame];
    }
  }

  //
  // Player list.
  //

  virtual SmallworldPlayer* getFirstPlayer()
  {
    int iter = m_poolPlayer.first();
    if (-1 == iter) {
      return 0;
    } else if (iter == m_idPlayer) {
      return this;
    } else {
      return &m_poolPlayer[iter];
    }
  }

  virtual SmallworldPlayer* getNextPlayer(SmallworldPlayer* pPlayer)
  {
    if (0 == pPlayer) {
      return 0;
    }

    int iter = m_poolPlayer.next(pPlayer->getPlayerId());
    if (-1 == iter) {
      return 0;
    } else if (iter == m_idPlayer) {
      return this;
    } else {
      return &m_poolPlayer[iter];
    }
  }

  //
  // Game list.
  //

  virtual SmallworldGame* getFirstGame()
  {
    int iter = m_poolGame.first();
    if (-1 == iter) {
      return 0;
    } else {
      return &m_poolGame[iter];
    }
  }

  virtual SmallworldGame* getNextGame(SmallworldGame* pGame)
  {
    if (0 == pGame) {
      return 0;
    }

    int iter = m_poolGame.next(pGame->getGameId());
    if (-1 == iter) {
      return 0;
    } else {
      return &m_poolGame[iter];
    }
  }

  //
  // Implement NetworkClientCallback.
  //

  virtual void onNetworkServerReady(NetworkClient*)
  {
    m_stage.popAndPush(&implSmallworldClient::stageConnected);
  }

  virtual void onNetworkServerLeave(NetworkClient*)
  {
    m_stage.popAndPush(&implSmallworldClient::stageDisconnected);
  }

  virtual void onNetworkPacketReady(NetworkClient*, const NetworkPacket& p)
  {
    m_stage.trigger((uint_ptr)(intptr_t)&p);
  }

  virtual void onNetworkStreamReady(NetworkClient*, std::string const& s)
  {
    m_pCallback->onSmallworldStreamReady(this, s);
  }

  //
  // Event handler.
  //

  void handleNotifyEvent(evSmallworldNotify* pNotify)
  {
    switch (pNotify->code)
    {
    case evSmallworldNotify::NC_NEED_LOGIN:
      {
        evSmallworldLogin el;
        el.bNeedPlayerList = m_conf.bNeedPlayerList;
        el.bNeedGameList = m_conf.bNeedGameList;
        el.bNeedMessage = m_conf.bNeedMessage;
        el.stream = m_stream;

        if (!m_pClient->send(el)) {
          m_pCallback->onSmallworldError(this, SMALLWORLD_STREAM_WRITE);
          disconnect();
        }
      }
      break;

    case evSmallworldNotify::NC_SERVER_BUSY:
      m_pCallback->onSmallworldError(this, SMALLWORLD_CONNECT_SERVER_BUSY);
      disconnect();
      break;

    case evSmallworldNotify::NC_VERSION_MISMATCH:
      m_pCallback->onSmallworldError(this, SMALLWORLD_LOGIN_VERSION);
      disconnect();
      break;

    case evSmallworldNotify::NC_LOGIN_ACCEPTED:
      m_idPlayer = pNotify->id;
      m_bVerified = true;
      m_pCallback->onSmallworldServerReady(this);
      break;

    case evSmallworldNotify::NC_ACCOUNT_OR_PASSWORD:
      m_pCallback->onSmallworldError(this, SMALLWORLD_LOGIN_ACCOUNT_OR_PASSWORD);
      disconnect();
      break;

    case evSmallworldNotify::NC_DUPLICATE_LOGIN:
      m_pCallback->onSmallworldError(this, SMALLWORLD_LOGIN_DUPLICATE);
      disconnect();
      break;

    case evSmallworldNotify::NC_LOGIN_NOT_ALLOWED:
      m_pCallback->onSmallworldError(this, SMALLWORLD_LOGIN_NOT_ALLOWED);
      disconnect();
      break;

    case evSmallworldNotify::NC_CHANNEL_IS_FULL:
      m_pCallback->onSmallworldError(this, SMALLWORLD_CHANNEL_IS_FULL);
      break;

    default:
      SW2_TRACE_WARNING("Unknown notify code received, ignore"); // Ignore.
      break;
    }
  }

  void handleUserEvent(NetworkPacket* pEvent)
  {
    m_pCallback->onSmallworldPacketReady(this, *pEvent);
  }

  void handleChannelEvent(evSmallworldChannel* pChannel)
  {
    switch (pChannel->code)
    {
    case evSmallworldChannel::NC_PLAYER_ADD:
      assert(!m_poolPlayer.isUsed(pChannel->idPlayer));
      if (m_poolPlayer.alloc(pChannel->idPlayer) == pChannel->idPlayer) {
        implSmallworldClientPlayer &player = m_poolPlayer[pChannel->idPlayer];
        player.m_idPlayer = pChannel->idPlayer;
        player.m_iChannel = pChannel->iChannel;
        player.m_idGame = -1;
        player.userData = (uint_ptr)0;
        player.m_pClient = this;
        m_pCallback->onSmallworldNewPlayerReady(this, &player);
      } else {
        assert(false);
      }
      break;

    case evSmallworldChannel::NC_PLAYER_REMOVE:
      m_pCallback->onSmallworldPlayerLeave(this, &m_poolPlayer[pChannel->idPlayer]);
      m_poolPlayer.free(pChannel->idPlayer);
      break;

    case evSmallworldChannel::NC_CHANGE:
      m_poolPlayer.reset();
      m_poolGame.reset();
      m_pCallback->onSmallworldChannelChanged(this, pChannel->iChannel, m_iChannel);
      m_iChannel = pChannel->iChannel;
      if (!m_conf.bNeedPlayerList && m_poolPlayer.alloc(m_idPlayer) == m_idPlayer) {
        implSmallworldClientPlayer &player = m_poolPlayer[m_idPlayer];
        player.m_idPlayer = m_idPlayer;
        player.m_iChannel = m_iChannel;
        player.m_idGame = -1;
        player.userData = (uint_ptr)0;
        player.m_pClient = this;
      }
      break;

    default:
      SW2_TRACE_WARNING("Unknown evChannel/code received, ignore"); // Ignore.
      break;
    }
  }

  void handleChatEvent(evSmallworldChat* pChat)
  {
    switch (pChat->code)
    {
    case evSmallworldChat::NC_CHAT_FROM:
      if (!m_poolPlayer.isUsed(pChat->idWho)) {
        implSmallworldClientPlayer player;
        player.m_idPlayer = pChat->idWho;
        player.m_iChannel = m_iChannel;
        player.m_idGame = -1;
        m_pCallback->onSmallworldMessageReady(this, &player, pChat->msg);
      } else {
        m_pCallback->onSmallworldMessageReady(this, &m_poolPlayer[pChat->idWho], pChat->msg);
      }
      break;

    case evSmallworldChat::NC_PM_FROM:
      if (!m_poolPlayer.isUsed(pChat->idWho)) {
        implSmallworldClientPlayer player;
        player.m_idPlayer = pChat->idWho;
        player.m_iChannel = m_iChannel;
        player.m_idGame = -1;
        m_pCallback->onSmallworldPrivateMessageReady(this, &player, pChat->msg, false);
      } else {
        m_pCallback->onSmallworldPrivateMessageReady(this, &m_poolPlayer[pChat->idWho], pChat->msg, false);
      }
      break;

    case evSmallworldChat::NC_PM_TO:
      if (!m_poolPlayer.isUsed(pChat->idWho)) {
        implSmallworldClientPlayer player;
        player.m_idPlayer = pChat->idWho;
        player.m_iChannel = m_iChannel;
        player.m_idGame = -1;
        m_pCallback->onSmallworldPrivateMessageReady(this, &player, pChat->msg, true);
      } else {
        m_pCallback->onSmallworldPrivateMessageReady(this, &m_poolPlayer[pChat->idWho], pChat->msg, true);
      }
      break;

    case evSmallworldChat::NC_PN_NOT_FOUND:
      m_pCallback->onSmallworldError(this, SMALLWORLD_CHAT_PM_NOT_FOUND);
      break;

    default:
      SW2_TRACE_WARNING("Unknown evChat/code received, ignore"); // ignore
      break;
    }
  }

  void handleGameEvent(evSmallworldGame* pGame)
  {
    switch (pGame->code)
    {
    case evSmallworldGame::NC_GAME_ADD:
      assert(!m_poolGame.isUsed(pGame->idGame));
      if (m_poolGame.alloc(pGame->idGame) == pGame->idGame) {
        implSmallworldClientGame &game = m_poolGame[pGame->idGame];
        game.m_idGame = pGame->idGame;
        game.m_pClient = this;
        game.userData = (uint_ptr)0;
        m_pCallback->onSmallworldNewGameReady(this, &game);
      } else {
        assert(false);
      }
      break;

    case evSmallworldGame::NC_GAME_REMOVE:
      assert(m_poolGame.isUsed(pGame->idGame));
      m_pCallback->onSmallworldGameLeave(this, &m_poolGame[pGame->idGame]);
      for (int iter = m_poolPlayer.first(); -1 != iter; iter = m_poolPlayer.next(iter)) {
        if (m_poolPlayer[iter].m_idGame == pGame->idGame) {
          m_poolPlayer[iter].m_idGame = -1;
        }
      }
      m_poolGame.free(pGame->idGame);
      break;

    case evSmallworldGame::NC_PLAYER_JOIN:
      assert(m_poolPlayer.isUsed(pGame->idPlayer));
      assert(m_poolGame.isUsed(pGame->idGame));
      m_poolPlayer[pGame->idPlayer].m_idGame = pGame->idGame;
      m_pCallback->onSmallworldPlayerJoinGame(this, &m_poolGame[pGame->idGame], &m_poolPlayer[pGame->idPlayer]);
      break;

    case evSmallworldGame::NC_PLAYER_LEAVE:
      assert(m_poolPlayer.isUsed(pGame->idPlayer));
      assert(m_poolGame.isUsed(pGame->idGame));
      m_pCallback->onSmallworldPlayerLeaveGame(this, &m_poolGame[pGame->idGame], &m_poolPlayer[pGame->idPlayer]);
      m_poolPlayer[pGame->idPlayer].m_idGame = -1;
      break;

    case evSmallworldGame::NC_GAME_NOT_FOUND:
      m_pCallback->onSmallworldError(this, SMALLWORLD_GAME_NOT_FOUND);
      break;

    default:
      SW2_TRACE_WARNING("Unknown evGame/code received, ignore"); // ignore
      break;
    }
  }

  void handleConnectedStageEvent(NetworkPacket *pEvent)
  {
    if (0 == pEvent) {
      return;
    }

    switch (pEvent->getId())
    {
    case EID_NOTIFY:
      handleNotifyEvent((evSmallworldNotify*)pEvent);
      break;

    case EID_CHANNEL:
      if (m_bVerified) {
        handleChannelEvent((evSmallworldChannel*)pEvent);
      }
      break;

    case EID_CHAT:
      if (m_bVerified) {
        handleChatEvent((evSmallworldChat*)pEvent);
      }
      break;

    case EID_GAME:
      if (m_bVerified) {
        handleGameEvent((evSmallworldGame*)pEvent);
      }
      break;

    default:
      if (m_bVerified) {
        handleUserEvent((NetworkPacket*)pEvent);
      }
      break;
    }
  }

  //
  // Implement stages.
  //

  void stageConnected(int state, uint_ptr pEvent)
  {
    if (JOIN == state) {
      m_iChannel = -1;
    }

    if (TRIGGER == state) {
      handleConnectedStageEvent((NetworkPacket*)(intptr_t)pEvent);
    }
  }

  void stageConnecting(int state, uint_ptr)
  {
    if (JOIN == state) {
      m_timer.setTimeout(SMALLWORLD_TIMEOUT_CONNECTING);
    }

    if (TRIGGER == state) {
      if (m_timer.isExpired()) {
        m_pCallback->onSmallworldError(this, SMALLWORLD_CONNECT_TIMEOUT);
        disconnect();
      }
    }
  }

  void stageDisconnected(int state, uint_ptr)
  {
    if (JOIN == state && m_bVerified) {
      m_pCallback->onSmallworldServerLeave(this);
      m_bVerified = false;
      m_idPlayer = -1;
      m_iChannel = -1;
    }
  }

  void stageDisconnecting(int state, uint_ptr)
  {
    if (JOIN == state) {
      m_pClient->disconnect();
    }

    if (TRIGGER == state) {
      if (CS_DISCONNECTED == m_pClient->getConnectionState()) {
        m_stage.popAndPush(&implSmallworldClient::stageDisconnected);
      }
    }
  }

public:

  SmallworldClientCallback* m_pCallback; // Callback interface.
  CONFIG_CLIENT m_conf;                 // Configuration.
  NetworkClient* m_pClient;             // Network server.
  StageStack<implSmallworldClient> m_stage; // Connection stage.
  TimeoutTimer m_timer;                 // Timeout timer.
  ObjectPool<implSmallworldClientGame, SMALLWORLD_MAX_PLAYER, true> m_poolGame; // Game pool.
  ObjectPool<implSmallworldClientPlayer, SMALLWORLD_MAX_PLAYER, true> m_poolPlayer; // Player pool.
  std::string m_stream;                 // Saved user login data stream.
  bool m_bVerified;                     // Has logged in?
  int m_idPlayer;                       // Player ID.
  int m_iChannel;                       // Current channel index(0..MAX_CHANNEL-1)
};

//
// implSmallworldClientPlayer.
//

SmallworldGame* implSmallworldClientPlayer::getGame()
{
  if (-1 == m_idGame) {
    return 0;
  } else {
    return &m_pClient->m_poolGame[m_idGame];
  }
}

//
// implSmallworldClientGame.
//

SmallworldPlayer* implSmallworldClientGame::getFirstPlayer()
{
  SmallworldPlayer* player = m_pClient->getFirstPlayer();
  while (player) {
    SmallworldGame* game = player->getGame();
    if (game && game->getGameId() == m_idGame) {
      return player;
    }
    player = m_pClient->getNextPlayer(player);
  }
  return 0;
}

SmallworldPlayer* implSmallworldClientGame::getNextPlayer(SmallworldPlayer* pPlayer)
{
  SmallworldPlayer* player = m_pClient->getNextPlayer(pPlayer);
  while (player) {
    SmallworldGame* game = player->getGame();
    if (game && game->getGameId() == m_idGame) {
      return player;
    }
    player = m_pClient->getNextPlayer(player);
  }
  return 0;
}

} // namespace impl

SmallworldClient* SmallworldClient::alloc(SmallworldClientCallback* pCallback)
{
  assert(pCallback);
  impl::implSmallworldClient* p = new impl::implSmallworldClient(pCallback);
  if (p && !p->init()) {
    delete p;
    p = 0;
  }
  return p;
}

void SmallworldClient::free(SmallworldClient* pInstance)
{
  impl::implSmallworldClient* p = (impl::implSmallworldClient*)pInstance;
  if (p) {
    p->uninit();
  }
  delete p;
}

} // namespace sw2

// end of swSmallworldClient.cpp
