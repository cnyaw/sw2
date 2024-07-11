
//
// Bit stream packet.
//
// Copyright (c) 2021 Waync Cheng.
// All Rights Reserved.
//
// 2021/11/25 Waync created.
//

///
/// \file
/// \brief Bit stream packet.
/// \author Waync Cheng
/// \date 2021/11/25
///

#pragma once

#include <list>

#include "swBitStream.h"
#include "swUtil.h"

namespace sw2 {

///
/// \brief Formatted packet.
///
/// Base class of stream packet, encapsulate data with bit stream.
///

class BitStreamPacket
{
public:
  typedef BitStreamPacket* (*StaticCreatePacket)();

  virtual ~BitStreamPacket()
  {
  }

  ///
  /// \brief Get packet ID.
  /// \return Return packet ID.
  /// \note Every packet has a unique ID, this function is implemented by
  ///       SW2_DECLARE_BITSTREAM_PACKET macro. No need to implement it again.
  ///

  virtual int getId() const=0;

  ///
  /// \brief Read data from bit stream.
  /// \param [in] bs Bit stream.
  /// \return Return true if read data success else return false.
  ///

  virtual bool read(BitStream &bs)=0;

  ///
  /// \brief Write data to bit stream.
  /// \param [out] bs Bit stream.
  /// \return Return true if write data success else return false.
  ///

  virtual bool write(BitStream &bs) const=0;
};

//
// \brief Read/write bit stream packets and manager packet cache.
//

template<int MAX_ID>
class BitStreamPacketHandler
{
public:

  //
  // \brief Constructor.
  // \param [in] bitMagic Number bits of magic ID header of packet.
  // \param [in] magic The magic ID header of packet.
  //

  explicit BitStreamPacketHandler(unsigned int bitsMagic = 0, unsigned int magic = 0) : m_bitsMagic(bitsMagic), m_magic(magic)
  {
  }

  //
  // \brief Release handled packet.
  // \param [in] p A packet returned by readPacket.
  // \return True if p is a valid packet else return false.
  //

  bool freePacket(const BitStreamPacket *p)
  {
    int id = p->getId();
    if (0 <= id && MAX_ID > id) {
      m_rt[id].freePacket((BitStreamPacket*)p);
      return true;
    }
    return false;
  }

  //
  // \brief Read and decode a packet from bit stream.
  // \param [in] bs A bit stream to read packet data from.
  // \param [out] pp A pointer to obtain a decoded packet.
  // \return True is a valid packet is decoded else return false.
  // \note A valid packet should
  //       1, match magic ID header. (Skip if magic ID is not assigned)
  //       2, match packet type ID.
  //       3, match packet format. (User define)
  // \note After the packet is handled, use freePacket to release the packet.
  //

  bool readPacket(BitStream &bs, const BitStreamPacket **pp)
  {
    if (0 < m_bitsMagic) {
      if (bs.isOutOfRange(m_bitsMagic)) {
        return false;
      }
      unsigned int magic;
      if (!(bs >> sw2::setBitCount(m_bitsMagic) >> magic)) {
        return false;
      }
      if (magic != m_magic) {
        return false;
      }
    }
    unsigned int id = (unsigned int)-1;
    if (!bs.isOutOfRange(getIdBitCount()) && !(bs >> setBitCount(getIdBitCount()) >> id)) {
      return false;
    }
    BitStreamPacket *p = allocPacket(id);
    if (0 == p) {
      return false;
    }
    if (!p->read(bs)) {
      freePacket(p);
      return false;
    }
    *pp = p;
    return true;
  }

  //
  // \brief Register a packet type for this handler.
  // \param [in] id This type ID of packet.
  // \param [in] pf A function to allocate a new object of this type of packet.
  // \param [in] pn Type name of packet.
  // \return True if register is success else return false.
  // \note Please use SW2_REGISTER_BITSTREAM_PACKET to do registration.
  //

  bool registerPacket(int id, BitStreamPacket::StaticCreatePacket pf, const char *pn)
  {
    if (0 > id || MAX_ID <= id) {
      SW2_TRACE_ERROR("registerPacket [%s:%d] invalid ID.", pn, id);
      return false;
    }
    if (0 != m_rt[id].pf) {
      SW2_TRACE_ERROR("registerPacket [%s:%d] already registerd.", pn, id);
      return false;
    }
    m_rt[id].pf = pf;
    return true;
  }

  //
  // \brief Encode and write a packet to bit stream.
  // \param [in] bs A bit stream to write packet data to.
  // \param [in] p A packet to encode and write to bs.
  // \return True if write is success else return false.
  //

  bool writePacket(BitStream &bs, const BitStreamPacket &p) const
  {
    if (0 < m_bitsMagic) {
      if (!(bs << sw2::setBitCount(m_bitsMagic) << m_magic)) {
        return false;
      }
    }
    if (!(bs << sw2::setBitCount(getIdBitCount()) << (unsigned int)p.getId())) {
      return false;
    }
    return p.write(bs);
  }

protected:
  BitStreamPacket* allocPacket(int id)
  {
    if(0 > id || MAX_ID <= id) {
      return 0;
    }
    return m_rt[id].allocPacket();
  }

  int getIdBitCount() const
  {
    return BITCOUNT<MAX_ID>::value;
  }

private:
  class BitStreamPacketRuntime
  {
  public:
    BitStreamPacket::StaticCreatePacket pf;
    std::list<BitStreamPacket*> cache;

    BitStreamPacketRuntime() : pf(0)
    {
    }

    ~BitStreamPacketRuntime()
    {
      std::list<BitStreamPacket*>::iterator it = cache.begin();
      for (; cache.end() != it; it++) {
        delete *it;
      }
    }

    BitStreamPacket* allocPacket()
    {
      if (!cache.empty()) {
        BitStreamPacket *p = cache.front();
        cache.pop_front();
        return p;
      }
      if (pf) {
        BitStreamPacket *p = pf();
        return p;
      }
      return 0;
    }

    void freePacket(BitStreamPacket *p)
    {
      cache.push_back(p);
    }
  };
  BitStreamPacketRuntime m_rt[MAX_ID];
  unsigned int m_bitsMagic, m_magic;
};

//
// \brief Declare a bit stream packet type.
// \note See BitStreamPacket::getId.
//

#define SW2_DECLARE_BITSTREAM_PACKET(id, cls) \
public:\
  static BitStreamPacket* staticCreatePacket() { return new cls; }\
  virtual int getId() const { return id; }

//
// \brief Register a bit stream packet type.
// \note See BitStreamPacketHandler::registerPacket.
//

#define SW2_REGISTER_BITSTREAM_PACKET(mgr, id, cls) mgr.registerPacket(id, &cls::staticCreatePacket, #cls);

} // namespace sw2

// end of swBitStreamPacket.h
