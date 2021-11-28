
//
//  Smallworld internal network packet implementation.
//
//  Copyright (c) 2014 Waync Cheng.
//  All Rights Reserved.
//
//  2014/03/02 Waync created.
//

#include "swSmallworldEv.h"

#include "swUtil.h"

namespace sw2 {

namespace impl {

//
// Internal constants.
//

#define SW2_SMALLWORLD_TAG "sw2sw"
#define SW2_SMALLWORLD_HEADER_MAGIC_BITS 32
#define SW2_SMALLWORLD_HEADER_MAGIC 0xfeed

//
// evSmallworldNotify.
//

bool evSmallworldNotify::read(BitStream& bs)
{
  if (!(bs >> setBitCount(BITCOUNT<NC_LAST_TAG - 1>::value) >> (uint&)code)) {
    return false;
  }

  if (NC_LAST_TAG <= code) {
    return false;
  }

  if (NC_LOGIN_ACCEPTED == code) {
    if (!(bs >> setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) >> (uint&)id)) {
      return false;
    }
    if (SMALLWORLD_MAX_PLAYER <= id) {
      return false;
    }
  }

  return true;
}

bool evSmallworldNotify::write(BitStream& bs) const
{
  assert(NC_LAST_TAG > (uint)code);

  if (!(bs << setBitCount(BITCOUNT<NC_LAST_TAG - 1>::value) << (uint)code)) {
    return false;
  }

  if (NC_LOGIN_ACCEPTED == code) {
    assert(SMALLWORLD_MAX_PLAYER > (uint)id);
    if (!(bs << setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) << (uint)id)) {
      return false;
    }
  }

  return true;
}

//
// evSmallworldLogin.
//

bool evSmallworldLogin::read(BitStream& bs)
{
  std::string tag;

  if (!(bs >> tag)) {
    return false;
  }

  if (!(tag == SW2_SMALLWORLD_TAG)) {
    return false;
  }

  if (!(bs >> setBitCount(BITCOUNT<100 - 1>::value) >> (uint&)verMajor)) { // Version vMM.NN, MM/NN:00-99.
    return false;
  }

  if (!(bs >> setBitCount(BITCOUNT<100 - 1>::value) >> (uint&)verMinor)) {
    return false;
  }

  if (!(bs >> bNeedPlayerList)) {
    return false;
  }

  if (!(bs >> bNeedGameList)) {
    return false;
  }

  if (!(bs >> bNeedMessage)) {
    return false;
  }

  if (!(bs >> setBitCount(BITCOUNT<SMALLWORLD_MAX_LOGIN_STREAM_LENGTH>::value) >> stream)) {
    return false;
  }

  if (SMALLWORLD_MAX_LOGIN_STREAM_LENGTH < (int)stream.length()) {
    return false;
  }

  return true;
}

bool evSmallworldLogin::write(BitStream& bs) const
{
  std::string tag(SW2_SMALLWORLD_TAG);

  if (!(bs << tag)) {
    return false;
  }

  if (!(bs << setBitCount(BITCOUNT<100 - 1>::value) << (uint)SMALLWORLD_VERSION_MAJOR)) {
    return false;
  }

  if (!(bs << setBitCount(BITCOUNT<100 - 1>::value) << (uint)SMALLWORLD_VERSION_MINOR)) {
    return false;
  }

  if (!(bs << bNeedPlayerList)) {
    return false;
  }

  if (!(bs << bNeedGameList)) {
    return false;
  }

  if (!(bs << bNeedMessage)) {
    return false;
  }

  assert(SMALLWORLD_MAX_LOGIN_STREAM_LENGTH >= (int)stream.length());

  if (!(bs << setBitCount(BITCOUNT<SMALLWORLD_MAX_LOGIN_STREAM_LENGTH>::value) << stream)) {
    return false;
  }

  return true;
}

//
// evSmallworldRequest.
//

bool evSmallworldRequest::read(BitStream& bs)
{
  if (!(bs >> setBitCount(BITCOUNT<NC_LAST_TAG - 1>::value) >> (uint&)code)) {
    return false;
  }

  if (NC_LAST_TAG <= code) {
    return false;
  }

  if (!(bs >> setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) >> (uint&)idPlayer)) {
    return false;
  }

  if (SMALLWORLD_MAX_PLAYER <= idPlayer) {
    return false;
  }

  if (!(bs >> time)) {
    return false;
  }

  if (!(bs >> setBitCount(BITCOUNT<SMALLWORLD_MAX_DATA_STREAM_LENGTH>::value) >> stream)) {
    return false;
  }

  if (SMALLWORLD_MAX_DATA_STREAM_LENGTH < (int)stream.length()) {
    return false;
  }

  return true;
}

bool evSmallworldRequest::write(BitStream& bs) const
{
  assert(NC_LAST_TAG > (uint)code);

  if (!(bs << setBitCount(BITCOUNT<NC_LAST_TAG - 1>::value) << (uint)code)) {
    return false;
  }

  assert(SMALLWORLD_MAX_PLAYER > (uint)idPlayer);

  if (!(bs << setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) << (uint)idPlayer)) {
    return false;
  }

  if (!(bs << time)) {
    return false;
  }

  assert(SMALLWORLD_MAX_DATA_STREAM_LENGTH > (int)stream.length());

  if (!(bs << setBitCount(BITCOUNT<SMALLWORLD_MAX_DATA_STREAM_LENGTH>::value) << stream)) {
    return false;
  }

  return true;
}

//
// evSmallworldChannel.
//

bool evSmallworldChannel::read(BitStream& bs)
{
  if (!(bs >> setBitCount(BITCOUNT<NC_LAST_TAG - 1>::value) >> (uint&)code)) {
    return false;
  }

  if (NC_LAST_TAG <= code) {
    return false;
  }

  switch (code)
  {
  case NC_PLAYER_ADD:
  case NC_PLAYER_REMOVE:
    if (!(bs >> setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) >> (uint&)idPlayer)) {
      return false;
    }
    if (SMALLWORLD_MAX_PLAYER <= idPlayer) {
      return false;
    }
    break;

  case NC_CHANGE:
    if (!(bs >> setBitCount(BITCOUNT<SMALLWORLD_MAX_CHANNEL - 1>::value) >> (uint&)iChannel)) {
      return false;
    }
    if (SMALLWORLD_MAX_CHANNEL <= iChannel) {
      return false;
    }
    break;

  default:
    return false;
  }

  return true;
}

bool evSmallworldChannel::write(BitStream& bs) const
{
  assert(NC_LAST_TAG > (uint)code);

  if (!(bs << setBitCount(BITCOUNT<NC_LAST_TAG - 1>::value) << (uint)code)) {
    return false;
  }

  switch (code)
  {
  case NC_PLAYER_ADD:
  case NC_PLAYER_REMOVE:
    assert(SMALLWORLD_MAX_PLAYER > (uint)idPlayer);
    if (!(bs << setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) << (uint)idPlayer)) {
      return false;
    }
    break;

  case NC_CHANGE:
    assert(SMALLWORLD_MAX_CHANNEL > (uint)iChannel);
    if (!(bs << setBitCount(BITCOUNT<SMALLWORLD_MAX_CHANNEL - 1>::value) << (uint)iChannel)) {
      return false;
    }
    break;

  default:
    return false;
  }

  return true;
}

//
// evSmallworldChat.
//

bool evSmallworldChat::read(BitStream& bs)
{
  if (!(bs >> setBitCount(BITCOUNT<NC_LAST_TAG - 1>::value) >> (uint&)code)) {
    return false;
  }

  if (NC_LAST_TAG <= code) {
    return false;
  }

  switch (code)
  {
  case NC_CHAT:
    if (!(bs >> msg)) {
      return false;
    }
    break;

  case NC_CHAT_FROM:
  case NC_PM_FROM:
  case NC_PM_TO:
    if (!(bs >> setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) >> (uint&)idWho)) {
      return false;
    }
    if (SMALLWORLD_MAX_PLAYER <= idWho) {
      return false;
    }
    if (!(bs >> msg)) {
      return false;
    }
    break;

  case NC_PN_NOT_FOUND:
    break;                              // Ignore.

  default:
    return false;
  }

  return true;
}

bool evSmallworldChat::write(BitStream& bs) const
{
  assert(NC_LAST_TAG > (uint)code);

  if (!(bs << setBitCount(BITCOUNT<NC_LAST_TAG - 1>::value) << (uint)code)) {
    return false;
  }

  switch (code)
  {
  case NC_CHAT:
    if (!(bs << msg)) {
      return false;
    }
    break;

  case NC_CHAT_FROM:
  case NC_PM_FROM:
  case NC_PM_TO:
    assert(SMALLWORLD_MAX_PLAYER > (uint)idWho);
    if (!(bs << setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) << (uint)idWho)) {
      return false;
    }
    if (!(bs << msg)) {
      return false;
    }
    break;

  case NC_PN_NOT_FOUND:
    break;                              // Ignore.

  default:
    return false;
  }

  return true;
}

//
// evSmallworldGame.
//

bool evSmallworldGame::read(BitStream& bs)
{
  if (!(bs >> setBitCount(BITCOUNT<NC_LAST_TAG - 1>::value) >> (uint&)code)) {
    return false;
  }

  if (NC_LAST_TAG <= code) {
    return false;
  }

  switch (code)
  {
  case NC_NEW:
  case NC_QUIT:
  case NC_GAME_NOT_FOUND:
    break;

  case NC_PLAYER_JOIN:
  case NC_PLAYER_LEAVE:
    if (!(bs >> setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) >> (uint&)idPlayer)) {
      return false;
    }
    if (SMALLWORLD_MAX_PLAYER <= idPlayer) {
      return false;
    }

    //
    // Fall through.
    //

  case NC_JOIN:
  case NC_GAME_ADD:
  case NC_GAME_REMOVE:
    if (!(bs >> setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) >> (uint&)idGame)) {
      return false;
    }
    if (SMALLWORLD_MAX_PLAYER <= idGame) {
      return false;
    }
    break;

  default:
    return false;
  }

  return true;
}

bool evSmallworldGame::write(BitStream& bs) const
{
  assert(NC_LAST_TAG > (uint)code);

  if (!(bs << setBitCount(BITCOUNT<NC_LAST_TAG - 1>::value) << (uint)code)) {
    return false;
  }

  switch (code)
  {
  case NC_NEW:
  case NC_QUIT:
  case NC_GAME_NOT_FOUND:
    break;

  case NC_PLAYER_JOIN:
  case NC_PLAYER_LEAVE:
    assert(SMALLWORLD_MAX_PLAYER > (uint)idPlayer);
    if (!(bs << setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) << (uint)idPlayer)) {
      return false;
    }

    //
    // Fall through.
    //

  case NC_JOIN:
  case NC_GAME_ADD:
  case NC_GAME_REMOVE:
    assert(SMALLWORLD_MAX_PLAYER > (uint)idGame);
    if (!(bs << setBitCount(BITCOUNT<SMALLWORLD_MAX_PLAYER - 1>::value) << (uint)idGame)) {
      return false;
    }
    break;

  default:
    return false;
  }

  return true;
}

BitStreamPacketHandler<EID_LAST_TAG> g_bph(SW2_SMALLWORLD_HEADER_MAGIC_BITS, SW2_SMALLWORLD_HEADER_MAGIC);

void initBitStreamPacketHandler()
{
  SW2_REGISTER_BITSTREAM_PACKET(g_bph, EID_NOTIFY, evSmallworldNotify);
  SW2_REGISTER_BITSTREAM_PACKET(g_bph, EID_LOGIN, evSmallworldLogin);
  SW2_REGISTER_BITSTREAM_PACKET(g_bph, EID_CHANNEL, evSmallworldChannel);
  SW2_REGISTER_BITSTREAM_PACKET(g_bph, EID_CHAT, evSmallworldChat);
  SW2_REGISTER_BITSTREAM_PACKET(g_bph, EID_GAME, evSmallworldGame);
  SW2_REGISTER_BITSTREAM_PACKET(g_bph, EID_REQUEST, evSmallworldRequest);
}

bool send(NetworkConnection* pConn, const BitStreamPacket &p)
{
  std::string buff;
  BitStream bs(buff);
  if (!g_bph.writePacket(bs, p)) {
    return false;
  }
  assert(pConn);
  return pConn->send(bs.getByteCount(), buff.data());
}

bool freePacket(const BitStreamPacket *p)
{
  return g_bph.freePacket(p);
}

bool readPacket(BitStream &bs, const BitStreamPacket **pp)
{
  return g_bph.readPacket(bs, pp);
}

} // namespace impl

bool InitializeSmallworld()
{
  if (!InitializeNetwork()) {
    return false;
  }
  impl::initBitStreamPacketHandler();
  SW2_TRACE_MESSAGE("swSmallworld initialized.");
  return true;
}

void UninitializeSmallworld()
{
  SW2_TRACE_MESSAGE("swSmallworld uninitialized.");
  UninitializeNetwork();
}

} // namespace sw2

// end of swSmallworldEv.cpp
