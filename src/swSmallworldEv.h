
//
//  Smallworld internal network packet declaration.
//
//  Copyright (c) 2014 Waync Cheng.
//  All Rights Reserved.
//
//  2014/03/02 Waync created.
//

#pragma once

#include "swSmallworld.h"

namespace sw2 {

namespace impl {

//
// Internal const.
//

#define SMALLWORLD_VERSION_MAJOR 1      // Version major.
#define SMALLWORLD_VERSION_MINOR 1      // Version minor.
#define SMALLWORLD_MAX_LOGIN_STREAM_LENGTH 127 // Max data stream length, in bytes.

//
// Event ID.
//

enum SMALLWORLD_EVENT_ID
{
  EID_DUMMY = 0,                        // Dummy, not used.
  EID_NOTIFY,                           // General notify.
  EID_LOGIN,                            // Login server.
  EID_CHANNEL,                          // Channel command/notify.
  EID_CHAT,                             // Channel chat message.
  EID_GAME,                             // Game command(create/join/quit).
  EID_REQUEST,                          // Request command.

  EID_LAST_TAG
};

//
// Events.
//

struct evSmallworldNotify : public NetworkPacket
{
  SW2_DECLARE_PACKET(EID_NOTIFY, evSmallworldNotify)

  enum NOTIFY_CODE_
  {
    NC_NONE = 0,                        // Dummy, not used.
    NC_NEED_LOGIN,                      // Connected, and need login command.
    NC_SERVER_BUSY,                     // Server is busy, try later.
    NC_VERSION_MISMATCH,                // Version mismatch.
    NC_LOGIN_ACCEPTED,                  // Login successful.
    NC_ACCOUNT_OR_PASSWORD,             // Account or password error.
    NC_DUPLICATE_LOGIN,                 // Duplicate login.
    NC_LOGIN_NOT_ALLOWED,               // Not allowed to login.
    NC_CHANNEL_IS_FULL,                 // Target channel is full, cannot change to.

    NC_LAST_TAG
  };

  virtual bool read(BitStream& bs);
  virtual bool write(BitStream& bs) const;

  int code;                             // Notify code.
  int id;                               // Player or server ID, only valid when code==NC_LOGIN_ACCEPTED.
};

struct evSmallworldLogin : public NetworkPacket
{
  SW2_DECLARE_PACKET(EID_LOGIN, evSmallworldLogin)

  virtual bool read(BitStream& bs);
  virtual bool write(BitStream& bs) const;

  int verMajor;                         // Major version.
  int verMinor;                         // Minor version.
  bool bNeedPlayerList;                 // For client login.
  bool bNeedGameList;                   // For client login.
  bool bNeedMessage;                    // For client login.
  std::string stream;
};

struct evSmallworldRequest : public NetworkPacket
{
  SW2_DECLARE_PACKET(EID_REQUEST, evSmallworldRequest)

  enum NOTIFY_CODE_
  {
    NC_NONE = 0,                        // Dummy, not used.

    // cmd
    NC_PLAYER_LOGIN,                    // Player login request.
    NC_PLAYER_LOGOUT,                   // Player logout request.

    // reply
    NC_ACCOUNT_OR_PASSWORD,             // Account or password error.
    NC_DUPLICATE_LOGIN,                 // Duplicate login.
    NC_NOT_ALLOWED,                     // Not allow login.
    NC_NOT_LOGIN,                       // Not login.

    NC_LAST_TAG
  };

  virtual bool read(BitStream& bs);
  virtual bool write(BitStream& bs) const;

  int code;                             // Notify code.
  int idPlayer;                         // Request id, for verify.
  uint time;                            // Time stamp, for verify.
  std::string stream;
};

struct evSmallworldChannel : public NetworkPacket
{
  SW2_DECLARE_PACKET(EID_CHANNEL, evSmallworldChannel)

  enum NOTIFY_CODE_
  {
    NC_NONE = 0,                        // Dummy, not used.
    NC_PLAYER_ADD,                      // New player enter channel.
    NC_PLAYER_REMOVE,                   // Player leave channel.
    NC_CHANGE,                          // Change channel.

    NC_LAST_TAG
  };

  virtual bool read(BitStream& bs);
  virtual bool write(BitStream& bs) const;

  int code;                             // Notify code.
  int idPlayer;                         // Player ID.
  int iChannel;                         // Channel index(0..MAX_CHANNEL-1).
};

struct evSmallworldChat : public NetworkPacket
{
  SW2_DECLARE_PACKET(EID_CHAT, evSmallworldChat)

  enum NOTIFY_CODE_
  {
    NC_NONE = 0,                        // Dummy, not used.
    NC_CHAT,                            // General channel chat.
    NC_CHAT_FROM,                       // General channel chat from idWho.
    NC_PM_FROM,                         // Private message from idWho.
    NC_PM_TO,                           // Private message to idWho.
    NC_PN_NOT_FOUND,                    // Private message target not found.

    NC_LAST_TAG
  };

  virtual bool read(BitStream& bs);
  virtual bool write(BitStream& bs) const;

  int code;                             // Notify code.
  int idWho;                            // Who sent the message.
  std::string msg;                      // Message.
};

struct evSmallworldGame : public NetworkPacket
{
  SW2_DECLARE_PACKET(EID_GAME, evSmallworldGame)

  enum NOTIFY_CODE_
  {
    NC_NONE = 0,                        // Dummy, not used.
    NC_NEW,                             // Create new game.
    NC_JOIN,                            // Join a game.
    NC_QUIT,                            // Quit game.
    NC_GAME_ADD,                        // New game added.
    NC_GAME_REMOVE,                     // A game removed.
    NC_PLAYER_JOIN,                     // A player joins a game.
    NC_PLAYER_LEAVE,                    // A player leaves a game.
    NC_GAME_NOT_FOUND,                  // Game not exist.

    NC_LAST_TAG
  };

  virtual bool read(BitStream& bs);
  virtual bool write(BitStream& bs) const;

  int code;                             // Notify code.
  int idGame;                           // Game ID.
  int idPlayer;                         // Player ID.
};

} // namespace impl

} // namespace sw2

// end of swSmallworldEv.h
