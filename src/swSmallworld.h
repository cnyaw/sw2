
//
//  TCP/IP network [Application layer]
//
//  Copyright (c) 2008 Waync Cheng.
//  All Rights Reserved.
//
//  2008/04/30 Waync created.
//

///
/// \file
/// \brief TCP/IP network [Application layer]
/// \author Waync Cheng
/// \date 2008/04/30
///

#pragma once

#include "swNetwork.h"
#include "swIni.h"

namespace sw2 {

///
/// Constants define.
///

enum SMALLWORLD_CONST1
{
  SMALLWORLD_MAX_PLAYER             = 1000, ///< Max online player count.
  SMALLWORLD_MAX_CHANNEL            = 10,   ///< Max channel count.
  SMALLWORLD_MAX_CHANNEL_PLAYER     = 100,  ///< Max player of a channel.
  SMALLWORLD_MAX_DATA_STREAM_LENGTH = 1000  ///< Max data stream length, in bytes.
};

///
/// \brief Initialize smallworld module.
/// \return Return true if success else return false.
///

bool InitializeSmallworld();

///
/// \brief Uninitialize smallworld module.
///

void UninitializeSmallworld();

///
/// Error code define.
///

enum SMALLWORLD_ERROR_CODE
{
  SMALLWORLD_SUCCESS = 0,               ///< Success, no error.

  //
  // Following fatal errors will disconnect the connection.
  //

  SMALLWORLD_CONNECT_FAILED,            ///< Connect fail, can't connect.
  SMALLWORLD_CONNECT_TIMEOUT,           ///< Connect fail, timeout.
  SMALLWORLD_CONNECT_SERVER_BUSY,       ///< Connect fail, server is busy.
  SMALLWORLD_LOGIN_VERSION,             ///< Version mismatch.
  SMALLWORLD_LOGIN_FAILED,              ///< Login fail.
  SMALLWORLD_LOGIN_ACCOUNT_OR_PASSWORD, ///< Login fail, account or password error.
  SMALLWORLD_LOGIN_DUPLICATE,           ///< Login fail, duplicate login.
  SMALLWORLD_LOGIN_NOT_ALLOWED,         ///< Login fail, not allow.
  SMALLWORLD_STREAM_READ,               ///< Read data stream fail.
  SMALLWORLD_STREAM_WRITE,              ///< Write data stream fail.

  //
  // Following errors will keep connection.
  //

  SMALLWORLD_CHAT_PM_NOT_FOUND,         ///< Can not find PM target.
  SMALLWORLD_CHANNEL_IS_FULL,           ///< Channel is full.
  SMALLWORLD_GAME_NOT_FOUND,            ///< Can not find the game.

  SMALLWORLD_LAST_TAG
};

///
/// Account request code.
///

enum SMALLWORLD_REPLY_ACCOUNT_CODE
{
  SMALLWORLD_RAC_SUCCESS = 0,           ///< Success, no error.
  SMALLWORLD_RAC_ACCOUNT_OR_PASSWORD,   ///< Account or password error.
  SMALLWORLD_RAC_DUPLICATE_LOGIN,       ///< Duplicate login.
  SMALLWORLD_RAC_NOT_ALLOW_LOGIN,       ///< Not allow login.
  SMALLWORLD_RAC_NOT_LOGIN,             ///< Not login.

  SMALLWORLD_RAC_LAST_TAG
};

//
// Forward decl.
//

class SmallworldAccount;
class SmallworldClient;
class SmallworldServer;
class SmallworldGame;

///
/// \brief Smallworld player.
///

class SmallworldPlayer
{
public:

  ///
  /// Get player ID.
  /// \return Return player ID, -1 indicate invalid.
  ///

  virtual int getPlayerId() const=0;

  ///
  /// Get current channel ID.
  /// \return Return current channel ID, -1 indicate invalid.
  ///

  virtual int getChannelId() const=0;

  ///
  /// Get login data stream.
  /// \return Return user defined login data stream.
  ///

  virtual std::string getLoginData() const=0;

  ///
  /// Get current game.
  /// \return Return current game else return 0 if not in a game.
  ///

  virtual SmallworldGame* getGame()=0;

  ///
  /// Disconnect from server.
  ///

  virtual void disconnect()=0;

  ///
  /// \brief Get address.
  /// \return Return address, format: ip:port.
  ///

  virtual std::string getAddr() const=0;

  ///
  /// Get statistics.
  /// \return Return statistics.
  ///

  virtual NetworkClientStats getNetStats()=0;

  ///
  /// Send a data packet to server.
  /// \param [in] p Data packet.
  /// \return Return true if success else return false.
  ///

  virtual bool send(const NetworkPacket& p)=0;

  ///
  /// Send a data stream to server.
  /// \param [in] len Data length(in byte)
  /// \param [in] pStream Data stream.
  /// \return Return true if success else return false.
  ///

  virtual bool send(int len, void const* pStream)=0;

  ///
  /// Send a public message to all players in current channel.
  /// \param [in] msg Message text.
  /// \return Return true if success else return false.
  /// \note If success the message will reflect to self.
  ///

  virtual bool sendMessage(const std::string& msg)=0;

  ///
  /// Send a private message to a player in the server.
  /// \param [in] idWho Target player ID.
  /// \param [in] msg Message text.
  /// \return Return true if success else return false.
  /// \note If success the message will reflect to self.
  ///

  virtual bool sendPrivateMessage(int idWho, const std::string& msg)=0;

  ///
  /// Switch to a different channel.
  /// \param [in] newChannel Target channel.
  /// \return Return true if success else return false.
  ///

  virtual bool changeChannel(int newChannel)=0;

  ///
  /// Open a new game.
  /// \return Return true if success else return false.
  ///

  virtual bool newGame()=0;

  ///
  /// Join an existing game.
  /// \param [in] idGame Game ID.
  /// \return Return true if success else return false.
  ///

  virtual bool joinGame(int idGame)=0;

  ///
  /// Quit current game.
  /// \return Return true if success else return false.
  ///

  virtual bool quitGame()=0;

  uint_ptr userData;                    ///< User define data.
};

///
/// \brief Smallworld game.
///

class SmallworldGame
{
public:

  ///
  /// Get game ID.
  /// \return Return game ID, -1 indicate invalid.
  ///

  virtual int getGameId() const=0;

  ///
  /// Get channel ID of the game.
  /// \return Return channel ID, -1 indicate invalid.
  ///

  virtual int getChannelId() const=0;

  ///
  /// Get first player of the game.
  /// \return Return first player else return 0 if no player.
  ///

  virtual SmallworldPlayer* getFirstPlayer()=0;

  ///
  /// Get next player of the game.
  /// \param [in] pPlayer Current player.
  /// \return Return next player else return 0 if no next player.
  ///

  virtual SmallworldPlayer* getNextPlayer(SmallworldPlayer* pPlayer)=0;

  uint_ptr userData;                    ///< User define data.
};

class SmallworldAccountConnection;

///
/// \brief Smallworld account server notify interface.
///

struct SmallworldAccountCallback
{
  ///
  /// Notify when account server startup, begin to accept new connection.
  /// \param [in] pAccount The account client.
  ///

  virtual void onSmallworldAccountServerStartup(SmallworldAccount* pAccount) {}

  ///
  /// Notify when account server shutdown, not allow new connection.
  /// \param [in] pAccount The account client.
  /// \note If never get a SmallworldAccountCallback::onSmallworldAccountServerStartup
  ///       notify then won't get this notify.
  ///

  virtual void onSmallworldAccountServerShutdown(SmallworldAccount* pAccount) {}

  ///
  /// Notify when any error occur.
  /// \param [in] pAccount The account client.
  /// \param [in] errCode See SMALLWORLD_ERROR_CODE.
  ///

  virtual void onSmallworldError(SmallworldAccount* pAccount, int errCode) {}

  ///
  /// Notify when a new server is connected.
  /// \param [in] pAccount The account client.
  /// \param [in] pNewServer New server.
  /// \return Return true to accept this new server else return false to
  ///         kick it.
  ///

  virtual bool onSmallworldNewServerReady(SmallworldAccount* pAccount, SmallworldAccountConnection* pNewServer) { return true; }

  ///
  /// Notify when a server is about disconnect.
  /// \param [in] pAccount The account client.
  /// \param [in] pServer The server.
  /// \note If never get a notify of SmallworldAccountCallback::onSmallworldNewServerReady
  ///       and return true then won't get this notify.
  ///

  virtual void onSmallworldServerLeave(SmallworldAccount* pAccount, SmallworldAccountConnection* pServer) {}

  ///
  /// Notify when a player request to login.
  /// \param [in] pAccount The account client.
  /// \param [in] pServer Source server.
  /// \param [in] stream Data stream.
  /// \param [in] token One-time token, require for SmallworldAccountConnection::replayPlayerLogin.
  /// \note The content of stream is application defined, may have player account,
  ///       password or other data that input from SmallworldClient::login.
  /// \note Call SmallworldAccountConnection::replyPlayerLogin to reply verification.
  ///

  virtual void onSmallworldRequestPlayerLogin(SmallworldAccount* pAccount, SmallworldAccountConnection* pServer, std::string const& stream, std::string const& token) {}

  ///
  /// Notify when a player request to logout.
  /// \param [in] pAccount The account client.
  /// \param [in] pServer Source server.
  /// \param [in] stream Data stream.
  /// \param [in] token One-time token, require for SmallworldAccountConnection::replayPlayerLogout.
  /// \note The content of stream is application defined, may have player account,
  ///       password or other data that input from SmallworldClient::login.
  /// \note Call SmallworldAccountConnection::replyPlayerLogout to reply verification.
  ///

  virtual void onSmallworldRequestPlayerLogout(SmallworldAccount* pAccount, SmallworldAccountConnection* pServer, std::string const& stream, std::string const& token) {}
};

///
/// \brief Smallworld account connection(a smallworld server)
///

class SmallworldAccountConnection
{
public:

  ///
  /// Disconnect connection.
  ///

  virtual void disconnect()=0;

  ///
  /// \brief Get address.
  /// \return Return address, format: ip:port.
  ///

  virtual std::string getAddr() const=0;

  ///
  /// Get statistics.
  /// \return Return statistics.
  ///

  virtual NetworkClientStats getNetStats()=0;

  ///
  /// Reply player login request(SmallworldAccountCallback::onSmallworldRequestPlayerLogin)
  /// \param [in] code See SMALLWORLD_REPLY_ACCOUNT_CODE.
  /// \param [in] token One-time token, get from SmallworldAccountCallback::OnRequestPlayerLogin.
  /// \return Return true if success else return false.
  ///

  virtual bool replyPlayerLogin(int code, std::string const& token)=0;

  ///
  /// Reply player logout request(SmallworldAccountCallback::onSmallworldRequestPlayerLogout)
  /// \param [in] code See SMALLWORLD_REPLY_ACCOUNT_CODE.
  /// \param [in] token One-time token, get from SmallworldAccountCallback::OnRequestPlayerLogout.
  /// \return Return true if success else return false.
  ///

  virtual bool replyPlayerLogout(int code, std::string const& token)=0;

  ///
  /// Get server ID.
  /// \return Return server ID, -1 indicate invalid.
  ///

  virtual int getServerId() const=0;

  uint_ptr userData;                    ///< User define data.
};

///
/// \brief Smallworld account server.
///

class SmallworldAccount
{
public:

  ///
  /// Allocate an account server instance.
  /// \param [in] pCallback Event Callback.
  /// \return If success return an interface pointer else return 0.
  ///

  static SmallworldAccount* alloc(SmallworldAccountCallback* pCallback);

  ///
  /// Release a unused account server instance.
  /// \param [in] pItf Instance to release.
  ///

  static void free(SmallworldAccount* pItf);

  ///
  /// Startup account server, begin to accept new connection.
  /// \param [in] conf Server configurations.
  /// \return Return true if success else return false.
  /// \note Conf format:\n
  ///       addrListen = "1234"\n
  ///       maxServer = 16
  ///

  virtual bool startup(Ini const& conf)=0;

  ///
  /// Shutdown account server, close all connections.
  ///

  virtual void shutdown()=0;

  ///
  /// Trigger account server.
  /// \note Application should call trigger periodically to make module work
  ///       properly.
  ///

  virtual void trigger()=0;

  ///
  /// Get statistics.
  /// \return Return statistics.
  ///

  virtual NetworkServerStats getNetStats()=0;

  ///
  /// Get first server.
  /// \return Return first server else return 0 if no server.
  ///

  virtual SmallworldAccountConnection* getFirstServer()=0;

  ///
  /// Get next server.
  /// \param [in] pServer Current server.
  /// \return Return next server else return 0 if no next server.
  ///

  virtual SmallworldAccountConnection* getNextServer(SmallworldAccountConnection* pServer)=0;

  uint_ptr userData;                    ///< User define data.
};

///
/// \brief Smallworld server notify interface.
///

struct SmallworldServerCallback
{
  ///
  /// Notify when server startup and begin to accept new connection.
  /// \param [in] pServer The server.
  ///

  virtual void onSmallworldServerStartup(SmallworldServer* pServer) {}

  ///
  /// Notify when server shutdown and not allow new connection.
  /// \param [in] pServer The server.
  /// \note If never get a SmallworldServerCallback::onSmallworldServerStartup
  ///       then won't get this notify.
  ///

  virtual void onSmallworldServerShutdown(SmallworldServer* pServer) {}

  ///
  /// Notify when any error occur.
  /// \param [in] pServer The server.
  /// \param [in] errCode See SMALLWORLD_ERROR_CODE.
  ///

  virtual void onSmallworldError(SmallworldServer* pServer, int errCode) {}

  ///
  /// Notify when receive a data packet from a player.
  /// \param [in] pServer The server.
  /// \param [in] pPlayer Data sender.
  /// \param [in] p Data packet.
  ///

  virtual void onSmallworldPacketReady(SmallworldServer* pServer, SmallworldPlayer* pPlayer, const NetworkPacket &p) {}

  ///
  /// Notify when receive a data stream from a player.
  /// \param [in] pServer The server.
  /// \param [in] pPlayer Data sender.
  /// \param [in] len Data length(in byte)
  /// \param [in] pStream Data stream.
  ///

  virtual void onSmallworldStreamReady(SmallworldServer* pServer, SmallworldPlayer* pPlayer, int len, void const* pStream) {}

  ///
  /// Notify when a new player login
  /// \param [in] pServer The server.
  /// \param [in] pNewPlayer New player.
  /// \return Return true to accept this new player else return false to kick it.
  ///

  virtual bool onSmallworldNewPlayerReady(SmallworldServer* pServer, SmallworldPlayer* pNewPlayer) { return true; }

  ///
  /// Notify when a player logout.
  /// \param [in] pServer The server.
  /// \param [in] pPlayer The player.
  /// \note If never get a notify of SmallworldServerCallback::onSmallworldNewPlayerReady
  ///       and return true then won't get this notify.
  ///

  virtual void onSmallworldPlayerLeave(SmallworldServer* pServer, SmallworldPlayer* pPlayer) {}

  ///
  /// Notify when a player switch channel.
  /// \param [in] pServer The server.
  /// \param [in] pPlayer The player.
  /// \param [in] newChannel New channel.
  /// \param [in] prevChannel Previous channel; -1 means this is a new player.
  /// \note Even not use channel mode, this notify is still produced, therefore
  ///       application can do more initialization in this notify. For example
  ///       send ohter init messages to the player, at this time the server has
  ///       sent player list and game list to the player.
  ///

  virtual void onSmallworldPlayerChannelChanged(SmallworldServer* pServer, SmallworldPlayer* pPlayer, int newChannel, int prevChannel) {}

  ///
  /// Notify when a new game is created.
  /// \param [in] pServer The server.
  /// \param [in] pNewGame The game.
  /// \return Return true to accept this game else return false to destroy it.
  ///

  virtual bool onSmallworldNewGameReady(SmallworldServer* pServer, SmallworldGame* pNewGame) { return true; }

  ///
  /// Notify when a game is closed.
  /// \param [in] pServer The server.
  /// \param [in] pGame The game.
  /// \note When the game is closed, if there are other players in the game then
  ///       won't get any SmallworldServerCallback::onSmallworldPlayerLeaveGame notify.
  ///

  virtual void onSmallworldGameLeave(SmallworldServer* pServer, SmallworldGame* pGame) {}

  ///
  /// Notify when a player join a game.
  /// \param [in] pServer The server.
  /// \param [in] pGame The game.
  /// \param [in] pNewPlayer The player.
  /// \return Return true to accept the player to join the game else return
  ///         false to reject it.
  /// \note When this notify is produced, the player has already add to the game.
  ///

  virtual bool onSmallworldPlayerJoinGame(SmallworldServer* pServer, SmallworldGame* pGame, SmallworldPlayer* pNewPlayer) { return true; }

  ///
  /// Notify when a player is quit a game.
  /// \param [in] pServer The server.
  /// \param [in] pGame The game.
  /// \param [in] pPlayer The player.
  /// \return Return true to allow the game keep existing else return false to
  ///         close the game.
  /// \note When this notify is produced, the player is still in the game. It
  ///       is removed after this notify return.
  /// \note If the last player quit, the game is also closed even return true.
  ///

  virtual bool onSmallworldPlayerLeaveGame(SmallworldServer* pServer, SmallworldGame* pGame, SmallworldPlayer* pPlayer) { return true; }
};

///
/// \brief Smallworld server.
///

class SmallworldServer
{
public:

  ///
  /// Allocate a server instance.
  /// \param [in] pCallback The Callback.
  /// \return Return an interface pointer if success else return 0.
  ///

  static SmallworldServer* alloc(SmallworldServerCallback* pCallback);

  ///
  /// Release a unused server instance.
  /// \param [in] pItf Instance to free.
  ///

  static void free(SmallworldServer* pItf);

  ///
  /// Startup server and begin to accept new connection.
  /// \param [in] conf Server configurations.
  /// \return Return true if success else return false.
  /// \note Conf format:\n
  ///       addrAccount = "localhost:2468"\n
  ///       EnablePlayerList = true\n
  ///       EnableGameList = true\n
  ///       EnableChannel = true\n
  ///       AddrListen = "2266"\n
  ///       MaxPlayer = 1000\n
  ///       MaxChannel = 10\n
  ///       MaxChannelPlayer = 100
  ///

  virtual bool startup(Ini const& conf)=0;

  ///
  /// Shutdown server, disconnect connections and not allow new connection.
  ///

  virtual void shutdown()=0;

  ///
  /// Trigger server.
  /// \note Application should call trigger periodically to make module works
  ///       properly.
  ///

  virtual void trigger()=0;

  ///
  /// Get statistics.
  /// \return Return statistics.
  ///

  virtual NetworkServerStats getNetStats()=0;

  ///
  /// Get first player.
  /// \return Return first player else return 0 if no player.
  ///

  virtual SmallworldPlayer* getFirstPlayer()=0;

  ///
  /// Get next player.
  /// \param [in] pPlayer Current player.
  /// \return Return next player else return 0 if no next player.
  ///

  virtual SmallworldPlayer* getNextPlayer(SmallworldPlayer* pPlayer)=0;

  ///
  /// Get first game.
  /// \return Return first game else return 0 if no game.
  ///

  virtual SmallworldGame* getFirstGame()=0;

  ///
  /// Get next game.
  /// \param [in] pGame Current game.
  /// \return Return next game else return 0 if no next game.
  ///

  virtual SmallworldGame* getNextGame(SmallworldGame* pGame)=0;

  uint_ptr userData;                    ///< User define data.
};

///
/// \brief Smallworld client notify interface.
///

struct SmallworldClientCallback
{
  ///
  /// Notify when login server successfully.
  /// \param [in] pClient The client.
  /// \note Before get this notify there is no other notify except error occurs.
  ///

  virtual void onSmallworldServerReady(SmallworldClient* pClient) {}

  ///
  /// Notify when logout server.
  /// \param [in] pClient The client.
  /// \note If never get a notify of SmallworldClientCallback::onSmallworldServerReady
  ///       then won't get this notify.
  ///

  virtual void onSmallworldServerLeave(SmallworldClient* pClient) {}

  ///
  /// Notify when any error occur.
  /// \param [in] pClient The client.
  /// \param [in] errCode See SMALLWORLD_ERROR_CODE.
  ///

  virtual void onSmallworldError(SmallworldClient* pClient, int errCode) {}

  ///
  /// Notify when receive a data packet from server.
  /// \param [in] pClient The client.
  /// \param [in] p Data packet.
  ///

  virtual void onSmallworldPacketReady(SmallworldClient* pClient, const NetworkPacket &p) {}

  ///
  /// Notify when receive a data stream from server.
  /// \param [in] pClient The client.
  /// \param [in] len Data length(in byte)
  /// \param [in] pStream Data stream.
  ///

  virtual void onSmallworldStreamReady(SmallworldClient* pClient, int len, void const* pStream) {}

  ///
  /// Notify when current channel is changed.
  /// \param [in] pClient The client.
  /// \param [in] newChannel New channel.
  /// \param [in] prevChannel Previous channel, -1 indicate join the channel
  ///             first time.
  /// \note This notify only produce if server is in channel mode.
  ///

  virtual void onSmallworldChannelChanged(SmallworldClient* pClient, int newChannel, int prevChannel) {}

  ///
  /// Notify when a player broadcast a public message.
  /// \param [in] pClient The client.
  /// \param [in] pPlayer The player.
  /// \param [in] msg Message text.
  /// \note "The player" also includes self.
  ///

  virtual void onSmallworldMessageReady(SmallworldClient* pClient, SmallworldPlayer* pPlayer, const std::string& msg) {}

  ///
  /// Notify when a player send a private message to self.
  /// \param [in] pClient The client.
  /// \param [in] pPlayer The sender player or the receiver, see below.
  /// \param [in] msg Message text.
  /// \param [in] bFeedback true means the message is sent by self, pPlayer
  ///             indicates the receiver; false means pPlayer sends a private
  ///             message to self.
  /// \note There are 2 conditions to get this notify, 1: someone sends private
  ///       message to self, 2: self sends private message to someone.
  ///

  virtual void onSmallworldPrivateMessageReady(SmallworldClient* pClient, SmallworldPlayer* pPlayer, const std::string& msg, bool bFeedback) {}

  ///
  /// Notify when a player enter to current channel.
  /// \param [in] pClient The client.
  /// \param [in] pNewPlayer New player
  /// \note To get this notify, client and server should enable player list.
  ///

  virtual void onSmallworldNewPlayerReady(SmallworldClient* pClient, SmallworldPlayer* pNewPlayer) {}

  ///
  /// Notify when a player exit current channel.
  /// \param [in] pClient The client.
  /// \param [in] pPlayer The player.
  /// \note To get this notify, client and server should enable player list.
  ///

  virtual void onSmallworldPlayerLeave(SmallworldClient* pClient, SmallworldPlayer* pPlayer) {}

  ///
  /// Notify when a new game is created in current channel.
  /// \param [in] pClient The client.
  /// \param [in] pNewGame New game.
  /// \note To get this notify, client and server should enable game list.
  ///

  virtual void onSmallworldNewGameReady(SmallworldClient* pClient, SmallworldGame* pNewGame) {}

  ///
  /// Notify when a game is closed in current channel.
  /// \param [in] pClient The client.
  /// \param [in] pGame The game.
  /// \note To get this notify, client and server should enable game list.
  /// \note If there are other players in the game, then no more notify of
  ///       SmallworldClientCallback::onSmallworldPlayerLeaveGame will be produced.
  ///

  virtual void onSmallworldGameLeave(SmallworldClient* pClient, SmallworldGame* pGame) {}

  ///
  /// Notify when a player joins a game.
  /// \param [in] pClient The client.
  /// \param [in] pGame The game.
  /// \param [in] pNewPlayer The player.
  /// \note To get this notify, client and server should enable game list.
  /// \note The player is already added to the game when this notify is produced.
  ///

  virtual void onSmallworldPlayerJoinGame(SmallworldClient* pClient, SmallworldGame* pGame, SmallworldPlayer* pNewPlayer) {}

  ///
  /// Notify when a player exits a game.
  /// \param [in] pClient The client.
  /// \param [in] pGame The game.
  /// \param [in] pPlayer The player.
  /// \note To get this notify, client and server should enable game list.
  /// \note The player is removed after this notify is returned.
  ///

  virtual void onSmallworldPlayerLeaveGame(SmallworldClient* pClient, SmallworldGame* pGame, SmallworldPlayer* pPlayer) {}
};

///
/// \brief Smallworld client.
///

class SmallworldClient : public SmallworldPlayer
{
public:

  ///
  /// Allocate a client instance.
  /// \param [in] pCallback The Callback.
  /// \return If success return an interface pointer else return 0.
  ///

  static SmallworldClient* alloc(SmallworldClientCallback* pCallback);

  ///
  /// Release a unused client instance.
  /// \param [in] pItf Instance to free.
  ///

  static void free(SmallworldClient* pItf);

  ///
  /// Login server.
  /// \param [in] conf Client configurations.
  /// \param [in] ins Data stream.
  /// \return Return true if success else return false.
  /// \note Content of stream is application defined.
  /// \note If it has an account server then this stream is sent to account server
  ///       as the verification data, see SmallworldAccountCallback::OnRequestPlayerLogin,
  ///       SmallworldAccountCallback::OnRequestPlayerLogout.
  /// \note Max length of stream is SMALLWORLD_MAX_LOGIN_STREAM_LENGTH(in bytes).
  /// \note Conf format:\n
  ///       AddrServer = "localhost:1234"\n
  ///       NeedGameList = true\n
  ///       NeedMessage = true\n
  ///       NeedPlayerList = true\n
  ///

  virtual bool login(Ini const& conf, std::string const& ins = "")=0;

  ///
  /// Logout server.
  /// \note This function is same as SmallworldPlayer::disconnect.
  ///

  virtual void logout()=0;

  ///
  /// Trigger client.
  /// \note Application should call trigger periodically to make module works
  ///       properly.
  ///

  virtual void trigger()=0;

  ///
  /// Get first player.
  /// \return Return first player else return 0 if no player.
  ///

  virtual SmallworldPlayer* getFirstPlayer()=0;

  ///
  /// Get next player.
  /// \param [in] pPlayer Current player.
  /// \return Return next player else return 0 if no next player.
  ///

  virtual SmallworldPlayer* getNextPlayer(SmallworldPlayer* pPlayer)=0;

  ///
  /// Get first game.
  /// \return Return first game else return 0 if no game.
  ///

  virtual SmallworldGame* getFirstGame()=0;

  ///
  /// Get next game.
  /// \param [in] pGame Current game.
  /// \return Return next game else return 0 if no next game.
  ///

  virtual SmallworldGame* getNextGame(SmallworldGame* pGame)=0;
};

} // namespace sw2

// end of swSmallworld.h
