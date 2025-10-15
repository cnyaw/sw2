
//
// TCP/IP network [Framework layer]
//
// Copyright (c) 2015 Waync Cheng.
// All Rights Reserved.
//
// 2015/2/2 Waync created.
//

///
/// \file
/// \brief TCP/IP network [Framework layer]
/// \author Waync Cheng
/// \date 2015/2/2
///

#pragma once

#include "swIni.h"
#include "swNetwork.h"

namespace sw2 {

///
/// \brief Initialize bigworld module.
/// \return Return true if success else return false.
///

bool InitializeBigworld();

///
/// \brief Uninitialize bigworld module.
///

void UninitializeBigworld();

class BigworldNode;                     // Forward decl.

///
/// \brief Bigworld event notify interface.
///

class BigworldCallback
{
public:

  ///
  /// \brief Notify when a new bigworld node is connected.
  /// \param [in] pInstNode The instance node.
  /// \param [in] pNewNode New bigworld node.
  /// \note The new node is always a child node of pInstNode.
  ///

  virtual void onBigworldNewNodeReady(BigworldNode *pInstNode, BigworldNode *pNewNode)
  {
  }

  ///
  /// \brief Notify when a bigworld node is disconnected.
  /// \param [in] pInstNode The instance node.
  /// \param [in] pNode The bigworld node.
  ///

  virtual void onBigworldNodeClose(BigworldNode *pInstNode, BigworldNode *pNode)
  {
  }

  ///
  /// \brief Notify when a data stream is ready from a bigworld node.
  /// \param [in] pInstNode The instance node.
  /// \param [in] pNode The sender bigworld node.
  /// \param [in] len Data length(in byte)
  /// \param [in] pStream Data stream.
  ///

  virtual void onBigworldStreamReady(BigworldNode *pInstNode, BigworldNode *pNode, int len, void const* pStream)
  {
  }
};

///
/// \brief Bigworld node.
///

class BigworldNode
{
public:

  ///
  /// \brief Allocate a bigworld node instance.
  /// \param [in] pCallback Bigworld callback.
  /// \return If success return an interface pointer else return 0.
  ///

  static BigworldNode* alloc(BigworldCallback *pCallback);

  ///
  /// \brief Release a unused bigworld node instance.
  /// \param [in] pNode Instance to free.
  ///

  static void free(BigworldNode *pNode);

  ///
  /// \brief Get unique ID of the bigworld node.
  /// \return Return the unique ID of the bigworld node.
  /// \note The ID of a bigworld node is not force to be unique, but if not unique
  ///       may cause ambiguous problem.
  ///

  virtual std::string getId() const=0;

  ///
  /// \brief Get address.
  /// \return Return address, format: ip:port.
  ///

  virtual std::string getAddr() const=0;

  ///
  /// Get statistics.
  /// \return Return statistics.
  ///

  virtual NetworkClientStats getNetStats() const=0;

  //
  // \brief Check is this node ready.
  // \return Return true if this node is ready.
  //

  virtual bool isReady() const=0;

  ///
  /// \brief Startup a bigworld node.
  /// \param [in] ini The INI confs of the bigworld node.
  /// \param [in] id The section name as the ID in the conf.
  /// \return Return true if startup success else return false.
  /// \note The network of a bigworld node is setup by the conf. Following is
  ///       a sample network:\n
  /// \n
  /// [Login1]\n
  /// Id=login1                         ; Default is the section name Login1. Id is also treated as node type.\n
  /// AddrNode=localhost:2888           ; The addr used for other node to connect to this node.\n
  /// Depex=Db1 Game1 Game2             ; Will connect to node Db1, Game1 and Game2.\n
  /// \n
  /// [Db1]\n
  /// AddrNode=localhost:1234\n
  /// \n
  /// [Game1]\n
  /// AddrNode=localhost:5678\n
  /// Depex=Db1                         ; Will connecto to node Db1.\n
  /// [Game2]\n
  /// AddrNode=localhost:5679\n
  /// Depex=Db1                         ; Will connecto to node Db1.\n
  /// WebSocket=1                       ; This server supports WebSocket.\n
  /// \n
  /// [Client]\n
  /// Depex=Login1\n
  /// KeepConnected=0                   ; Do not keep connected automatically. Default is 1(true).\n
  /// \n
  /// \note The relationship of the sample network:\n
  /// - Child node of Login1: Client\n
  /// - Parent node of Login1: Db1 Game1\n
  /// - Child node of Game1: Login1\n
  /// - Parent node of Game1: Db1\n
  /// - Child node of Db1: Login1 Game1\n
  /// - Parent node of Db1: none\n
  /// - Child node of Client: none\n
  /// - Parent node of Client: Login1\n
  ///

  virtual bool startup(Ini const &ini, std::string const &id)=0;

  ///
  /// \brief Shutdown the bigworld node.
  ///

  virtual void shutdown()=0;

  ///
  /// \brief Trigger bigworld module.
  /// \note Application should call trigger periodically to make module works
  ///       properly.
  ///

  virtual void trigger()=0;

  ///
  /// \brief Send a data stream to this bigworld node.
  /// \param [in] len Data length(in byte)
  /// \param [in] pStream Data stream.
  /// \return Return true if success else return false.
  ///

  virtual bool send(int len, void const* pStream)=0;

  ///
  /// \brief Add a new depex node.
  /// \param [in] ini The INI confs of the bigworld node.
  /// \param [in] ids The section name as the ID in the conf.
  /// \return Return true if add new depex success else return false.
  /// \note See BigworldNode::startup for more info of conf format.
  /// \note If a depex node is alreay existed then it is ignored.
  ///

  virtual bool addDepex(Ini const &ini, const std::vector<std::string> &ids)=0;

  ///
  /// \brief Get first child node.
  /// \return Return the first child node.
  /// \note A child node is a node connected to this node.
  ///

  virtual BigworldNode* getFirstChild()=0;

  ///
  /// \brief Get next child node.
  /// \param [in] pNode Current bigworld node.
  /// \return Return the next child node.
  /// \note A child node is a node connected to this node.
  ///

  virtual BigworldNode* getNextChild(BigworldNode *pNode)=0;

  ///
  /// \brief Get first depex node.
  /// \return Return the first depex node.
  /// \note A depex node is a node this node connected to.
  ///

  virtual BigworldNode* getFirstDepex()=0;

  ///
  /// \brief Get next depex node.
  /// \param [in] pNode Current bigworld node.
  /// \return Return the next depex node.
  /// \note A depex node is a node this node connected to.
  ///

  virtual BigworldNode* getNextDepex(BigworldNode *pNode)=0;

  uint_ptr userData;                    ///< User define data.
};

} // namespace sw2

// end of swBigworld.h
