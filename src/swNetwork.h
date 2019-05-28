
//
//  TCP/IP network [Packet layer]
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/08/02 Waync created.
//

///
/// \file
/// \brief TCP/IP network [Packet layer]
///
/// Network module is based on Socket module, provides higher level network
/// features. Include:
/// - Disconnection detect.
/// - Full data stream control.
/// - Formated network data packet.
///
/// The usage of Network module is similar to Socket module:
///
/// Client side:
/// - Provide and implement event callback: inherit NetworkClientCallback and
///   implement the event callback interface to handle client events. Ex: connect
///   to server event, received data from server event, etc.
/// - Allocate a client instance: allocate a network client instance then use
///   the instance to do network operations. Ex: connect to server, send data
///   to server, etc.
///
/// Server side:
/// - Provide and implement event callback: inherit NetworkServerCallback and
///   implement the event callback interface to handle server events. Ex: handle
///   new connection request event, client disconnect event or receive data from
///   client event, etc.
/// - Allocate a server instance: allocate a network server instance then use the
///   instance to do network operations.
/// - Manage/manipulate client connection: in the server, every client is related
///   to a connection object. Use the interface to handle client operations.
///
/// Example:
///
/// \code
/// #include "swNetwork.h"
///
/// //
/// // User define packet.
/// //
///
/// class MyPacket : public NetworkPacket
/// {
/// public:
///
///   //
///   // Declare packet class and it's unique ID.
///   //
///
///   SW2_DECLARE_PACKET(ID_MYPACKET, MyPacket)
///
///   //
///   // Implement formated data read/write functions.
///   //
///
///   virtual bool read(BitStream& bs) { ... }
///   virtual bool write(BitStream& bs) { ... }
/// };
///
/// //
/// // Register and implement MyPacket, this line should place in the source file.
/// //
///
/// SW2_IMPLEMENT_PACKET(ID_MYPACKET, MyPacket)
///
/// //
/// // Client.
/// //
///
/// class testClient : public NetworkClientCallback
/// {
/// public:
///   testClient()
///   {
///      m_pClient = NetworkClient::alloc(this); // Allocate client instance.
///   }
///   virtual ~testClient()
///   {
///      NetworkClient::free(m_pClient); // Release client instance.
///   }
///
///   virtual void onNetworkServerReady(NetworkClient* pClient)
///   { // Do something when connect to server.
///   }
///   virtual void onNetworkServerLeave(NetworkClient* pClient)
///   { // Do something when disconnect from server.
///   }
///   virtual void onNetworkStreamReady(NetworkClient* pClient, int len, void const* pStream)
///   { // Do something when receive a data stream from server.
///   }
///   void onNetworkPacketReady(NetworkClient* pClient, NetworkPacket const& p)
///   { // Do something when receive a data packet from server.
///      if (ID_MYPACKET == p.getId())
///      { // This is a MyPacket packet.
///      }
///   }
///
///   NetworkClient* m_pClient;
/// };
///
/// //
/// // Server.
/// //
///
/// class testServer : public NetworkServerCallback
/// {
/// pubic:
///   testServer()
///   {
///      m_pServer = NetworkServer::alloc(this); // Allocate server instance.
///   }
///   virtual ~testServer()
///   {
///      NetworkServer::free(m_pServer); // Release server instance.
///   }
///
///   virtual void onNetworkServerStartup(NetworkServer* pServer)
///   { // Do something when server is startup.(Ready to accept new connection.)
///   }
///   virtual void onNetworkServerShutdown(NetworkServer* pServer)
///   { // Do something server is shutdown.(No more new connection is allowed.)
///   }
///   virtual bool onNetworkNewClientReady(NetworkServer* pServer, NetworkConnection* pNewClient)
///   { // Do something when a new client is arrived.
///      return true; // Return true to accpet this new request, else return false to reject it.
///   }
///   virtual void onNetworkClientLeave(NetworkServer* pServer, NetworkConnection* pClient)
///   { // Do something when a client is disconnect.
///   }
///   virtual void onNetworkStreamReady(NetworkServer* pServer, NetworkConnection* pClient, int len, void const* pStream)
///   { // Do something when received a data stream from a client.
///   }
///   void onNetworkPacketReady(NetworkServer* pServer, NetworkConnection* pClient, NetworkPacket const& p)
///   { // Do something when received a data packet from a client.
///      if (ID_MYPACKET == p.getId())
///      { // This is a MyPacket packet.
///      }
///   }
///
///   NetworkServer* m_pServer;
/// };
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2006/03/15
///

#pragma once

#include "swBitStream.h"
#include "swSocket.h"

namespace sw2 {

///
/// \brief Initialize network module.
/// \return Return true if success else return false.
///

bool InitializeNetwork();

///
/// \brief Uninitialize network module.
///

void UninitializeNetwork();

///
/// \brief Network client statistics.
///

struct NetworkClientStats : public SocketClientStats
{
  long packetsSent;                     ///< Total packets sent.
  long packetsRecv;                     ///< Total packets received.
};

///
/// \brief Network server statistics.
///

struct NetworkServerStats : public SocketServerStats
{
  long packetsSent;                     ///< Total packets sent.
  long packetsRecv;                     ///< Total packets received.
};

///
/// \brief Network formated packet.
///
/// Base class of network packet, encapsulate data stream with bit stream.
///

class NetworkPacket
{
public:
  typedef NetworkPacket* (*StaticCreatePacket)();

  virtual ~NetworkPacket()
  {
  }

  ///
  /// \brief Get packet ID.
  /// \return Return packet ID.
  /// \note Every packet has a unique ID, this function is implemented by
  ///       SW2_DECL_PACKET macro, no need to implement it again.
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

class NetworkClient;
class NetworkServer;
class NetworkConnection;

///
/// \brief Network client event notify interface.
///

class NetworkClientCallback
{
public:

  ///
  /// \brief Notify when connect to server.
  /// \param [in] pClient The client.
  ///

  virtual void onNetworkServerReady(NetworkClient* pClient)
  {
  }

  ///
  /// \brief Notify when disconnect from server.
  /// \param [in] pClient The client.
  /// \note If never get a NetworkClientCallback::onNetworkServerReady notify,
  ///       then will not get this notify.
  ///

  virtual void onNetworkServerLeave(NetworkClient* pClient)
  {
  }

  ///
  /// \brief Notify when a data stream is ready from server.
  /// \param [in] pClient The client.
  /// \param [in] len Data length(in byte)
  /// \param [in] pStream Data stream.
  ///

  virtual void onNetworkStreamReady(NetworkClient* pClient, int len, void const* pStream)
  {
  }

  ///
  /// \brief Notify when a formated packet is ready from server.
  /// \param [in] pClient The client.
  /// \param [in] p Data packet.
  ///

  virtual void onNetworkPacketReady(NetworkClient* pClient, NetworkPacket const& p)
  {
  }
};

///
/// \brief Network server event notify interface.
///

class NetworkServerCallback
{
public:

  ///
  /// \brief Notify when server startup.
  /// \param [in] pServer The server.
  ///

  virtual void onNetworkServerStartup(NetworkServer* pServer)
  {
  }

  ///
  /// \brief Notify when server shutdown.
  /// \param [in] pServer The server.
  /// \note If never get a NetworkServerCallback::onNetworkServerStartup notify,
  ///       then will not get this notify.
  /// \note After get this notify, existing connections will still keep connected.
  ///

  virtual void onNetworkServerShutdown(NetworkServer* pServer)
  {
  }

  ///
  /// \brief Notify when a new client is arrived.
  /// \param [in] pServer The server.
  /// \param [in] pNewClient New client.
  /// \return Return true to accept this new client else return false to reject
  ///         the request and server will disconnect it.
  ///

  virtual bool onNetworkNewClientReady(NetworkServer* pServer, NetworkConnection* pNewClient)
  {
    return true;
  }

  ///
  /// \brief Notify when a client is disconnect from server.
  /// \param [in] pServer The server.
  /// \param [in] pClient The client.
  /// \note If never get a NetworkServerCallback::onNetworkNewClientReady and
  ///       return true then will not get this notify.
  ///

  virtual void onNetworkClientLeave(NetworkServer* pServer, NetworkConnection* pClient)
  {
  }

  ///
  /// \brief Notify when a data stream is ready from a client.
  /// \param [in] pServer The server.
  /// \param [in] pClient The sender client.
  /// \param [in] len Data length(in byte)
  /// \param [in] pStream Data stream.
  ///

  virtual void onNetworkStreamReady(NetworkServer* pServer, NetworkConnection* pClient, int len, void const* pStream)
  {
  }

  ///
  /// \brief Notify when a formated packet is ready from a client.
  /// \param [in] pServer The server.
  /// \param [in] pClient The sender client.
  /// \param [in] p Data packet.
  ///

  virtual void onNetworkPacketReady(NetworkServer* pServer, NetworkConnection* pClient, NetworkPacket const& p)
  {
  }
};

///
/// \brief Network client connection.
///

class NetworkConnection
{
public:

  ///
  /// \brief Disconnect the connection.
  ///

  virtual void disconnect()=0;

  ///
  /// \brief Get connection state.
  /// \return See CONNECTION_STATE(Socket).
  ///

  virtual int getConnectionState() const=0;

  ///
  /// \brief Get address.
  /// \return Return address, format: ip:port.
  ///

  virtual std::string getAddr() const=0;

  ///
  /// \brief Get statistics.
  /// \return Return statistics.
  ///

  virtual NetworkClientStats getNetStats()=0;

  ///
  /// \brief Send a data stream to remote client.
  /// \param [in] len Data length(in byte).
  /// \param [in] pStream Data stream.
  /// \return Return true if success else return false.
  /// \note Return true doesn't mean the data is sent right away. It is possible
  ///       queued and sent later.
  ///

  virtual bool send(int len, void const* pStream)=0;

  ///
  /// \brief Send a formated packet to remote client.
  /// \param [in] p Data packet.
  /// \return Return true if success else return false.
  /// \note Return true doesn't mean the data is sent right away. It is possible
  ///       queued and sent later.
  ///

  virtual bool send(NetworkPacket const& p)=0;

  uint_ptr userData;                    ///< User define data.
};

///
/// \brief Network client.
///

class NetworkClient : public NetworkConnection
{
public:

  ///
  /// \brief Allocate a client instance.
  /// \param [in] pCallback Client callback.
  /// \return If success return an interface pointer else return 0.
  ///

  static NetworkClient* alloc(NetworkClientCallback* pCallback);

  ///
  /// \brief Release a unused client instance.
  /// \param [in] pItf Instance to free.
  ///

  static void free(NetworkClient* pItf);

  ///
  /// \brief Connect to server.
  /// \param [in] svrAddr Address of server, format: ip:port or hostname:port.
  /// \return Return true if success else return false.
  /// \note It may not connect to the server right away if return true, to make
  ///       sure it is connected to the server you should get a notify of
  ///       NetworkClient::onNetworkServerReady or use NetworkClient::getConnectionState
  ///       to check state.
  ///

  virtual bool connect(std::string const& svrAddr)=0;

  ///
  /// \brief Trigger network.
  /// \note Application should call trigger periodically to make module works
  ///       properly.
  ///

  virtual void trigger()=0;

  ///
  /// \brief Get trigger frequency.
  /// \return Return trigger frequency setting.
  /// \note A trigger frequency is used to control the data flow of send/recv
  ///       process of a connection. Higher frequency can handle more data in
  ///       a second, lower frequency can handle less data in a second.
  ///

  virtual int getTriggerFrequency() const=0;

  ///
  /// \brief Set trigger frequency.
  /// \param [in] freq New trigger frequency.
  ///

  virtual void setTriggerFrequency(int freq)=0;
};

///
/// \brief Network server.
///

class NetworkServer
{
public:

  ///
  /// \brief Allocate a server instance.
  /// \param [in] pCallback Server callback.
  /// \return If success return an interface pointer else return 0.
  ///

  static NetworkServer* alloc(NetworkServerCallback* pCallback);

  ///
  /// \brief Release a unused server instance.
  /// \param [in] pItf Instance to free.
  ///

  static void free(NetworkServer* pItf);

  ///
  /// \brief Startup server and begin to accept new connection.
  /// \param [in] addr Listen port, format: ip:port, hostname:port or port.
  /// \return Return true if success else return false.
  ///

  virtual bool startup(std::string const& addr)=0;

  ///
  /// \brief Stop to accept new connection, existing connections will still keep
  ///        connected.
  ///

  virtual void shutdown()=0;

  ///
  /// \brief Trigger network.
  /// \note Application should call trigger periodically to make module works
  ///       properly.
  ///

  virtual void trigger()=0;

  ///
  /// \brief Get server address.
  /// \return Return address, format: ip:port.
  ///

  virtual std::string getAddr() const=0;

  ///
  /// \brief Get statistics.
  /// \return Return statistics.
  ///

  virtual NetworkServerStats getNetStats()=0;

  ///
  /// \brief Get first connection.
  /// \return Return first connection.
  ///

  virtual NetworkConnection* getFirstConnection() const=0;

  ///
  /// \brief Get next connection.
  /// \param [in] pClient Current connection.
  /// \return Return next connection.
  ///

  virtual NetworkConnection* getNextConnection(NetworkConnection* pClient) const=0;

  ///
  /// \brief Get trigger frequency.
  /// \return Return trigger frequency setting.
  /// \note A trigger frequency is used to control the data flow of send/recv
  ///       process of a connection. Higher frequency can handle more data in
  ///       a second, lower frequency can handle less data in a second.
  ///

  virtual int getTriggerFrequency() const=0;

  ///
  /// \brief Set trigger frequency.
  /// \param [in] freq New trigger frequency.
  ///

  virtual void setTriggerFrequency(int freq)=0;

  uint_ptr userData;                    ///< User define data.
};

///
/// \brief Declare packet class.
///

#define SW2_DECLARE_PACKET(id, cls) \
public:\
  static NetworkPacket* staticCreatePacket() { return new cls; }\
  virtual int getId() const { return id; }

//
// Internal register net packet class.
//

struct NetworkPacketRegister
{
  NetworkPacketRegister(uint, NetworkPacket::StaticCreatePacket, char const*);
};

///
/// \brief Implement packet class.
///

#define SW2_IMPLEMENT_PACKET(id, cls) NetworkPacketRegister cls##NetworkPacketRegister(id, &cls::staticCreatePacket, #cls);

} // namespace sw2

// end of swNetwork.h
