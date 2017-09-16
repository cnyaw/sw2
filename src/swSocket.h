
//
//  TCP/IP network [Stream layer]
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/06/05 Waync created.
//

///
/// \file
/// \brief TCP/IP network [Stream layer]
///
/// Socket module is TCP/IP network stream layer, it provides basic network features:
/// - Manage connections.
/// - Non-blocking networking.
/// - Basic flow control.
///
/// Socket module is a low level module, there are many things you have to handle
/// manually. Ex: time/flow control, combine data stream, encode/decode packets, etc.
///
/// To use this moduele:
///
/// Client side:
/// - Implement event handler: inherit and implement SocketClientCallback to handle
///   client events.
/// - Get client instance: allocate a socket client instance, and access network
///   through this instance.
///
/// Server side:
/// - Implement event handler: inherit and implement SocketServerCallback to handle
///   server events.
/// - Get server instance: allocate a socket server instance, and access network
///   through this instance.
/// - Manage client connections: every client connection has a virtual connection
///   to the client, access network client through the virtual connection.
///
/// Example:
///
/// \code
/// #include "swSocket.h"
///
/// // client
/// class testClient : public SocketClientCallback
/// {
/// public:
///   testClient()
///   {
///      m_pClient = SocketClient::alloc(this); // Allocate client instance.
///   }
///   virtual ~testClient()
///   {
///      SocketClient::free(m_pClient); // Free instance.
///   }
///
///   virtual void onSocketServerReady(SocketClient *pClient)
///   { // Do something when connect to server.
///   }
///   virtual void onSocketServerLeave(SocketClient *pClient)
///   { // Do something when disconnect from server.
///   }
///   virtual void onSocketStreamReady(SocketClient *pClient, int_t len, const void* pStream)
///   { // Do something when receive data stream from server.
///   }
///
///   SocketClient* m_pClient;
/// };
///
/// // server
/// class testServer : public SocketServerCallback
/// {
/// pubic:
///   testServer()
///   {
///      m_pServer = SocketServer::alloc(this); // Allocate server instance.
///   }
///   virtual ~testServer()
///   {
///      SocketServer::free(m_pServer); // Free instance.
///   }
///
///   virtual void onSocketServerStartup(SocketServer *pServer)
///   { // Do something when server is startup.(accept new connection)
///   }
///   virtual void onSocketServerShutdown(SocketServer *pServer)
///   { // Do something when server is shutdown.(no more new connection)
///   }
///   virtual bool onSocketNewClientReady(SocketServer *pServer, SocketConnection* pNewClient)
///   { // Do something when a new connectoin is arrived.
///      return true; // Return true to accept this new client else return false to disconnect it.
///   }
///   virtual void onSocketClientLeave(SocketServer *pServer, SocketConnection* pClient)
///   { // Do something when a connection is about to close.
///   }
///   virtual void onSocketStreamReady(SocketServer *pServer, SocketConnection* pClient, int len, void const* pStream)
///   { // Do something when receive data stream from a client.
///   }
///
///   SocketServer* m_pServer;
/// };
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2006/03/15
///

#pragma once

#include <ctime>

#include "swinc.h"

namespace sw2 {

///
/// \brief Initialize socket module.
/// \return Return true if success else retur false.
///

bool InitializeSocket();

///
/// \brief Uninitialize socket module.
///

void UninitializeSocket();

///
/// Connection states.
///

enum CONNECTION_STATE
{
  CS_CONNECTED,                         ///< Connected state.
  CS_CONNECTING,                        ///< Connecting state.
  CS_DISCONNECTED,                      ///< Disconnected state.
  CS_DISCONNECTING                      ///< Disconnecting state.
};

///
/// \brief Socket client statistics.
///

struct SocketClientStats
{
  time_t startTime;                     ///< Start time.
  time_t upTime;                        ///< Up time.

  long bytesSent;                       ///< Total bytes sent.
  long bytesRecv;                       ///< Total bytes received.
};

///
/// \brief Socket server statistics.
///

struct SocketServerStats
{
  time_t startTime;                     ///< Start time.
  time_t upTime;                        ///< Up time.

  long bytesSent;                       ///< Total bytes sent.
  long bytesRecv;                       ///< Total bytes received.

  long hits;                            ///< Total hit count.
  long currOnline;                      ///< Current online count.
  long maxOnline;                       ///< Max online count.
};

class SocketClient;
class SocketServer;
class SocketConnection;

///
/// \brief Socket client event notify interface.
///

class SocketClientCallback
{
public:

  ///
  /// \brief Notify when connect to the server.
  /// \param [in] pClient The client.
  ///

  virtual void onSocketServerReady(SocketClient *pClient)
  {
  }

  ///
  /// \brief Notify when disconnect from the server.
  /// \param [in] pClient The client.
  /// \note If never get a SocketClientCallback::onSocketServerReady notify then
  ///       won't get this notify.
  ///

  virtual void onSocketServerLeave(SocketClient *pClient)
  {
  }

  ///
  /// \brief Notify when received data stream from server.
  /// \param [in] pClient The client.
  /// \param [in] len Data length(in byte).
  /// \param [in] pStream Data stream.
  /// \note The received data stream may be part of sent data, application should
  ///       detect it and combine data parts manually.
  ///

  virtual void onSocketStreamReady(SocketClient *pClient, int len, void const* pStream)
  {
  }
};

///
/// \brief Socket server event notify interface.
///

class SocketServerCallback
{
public:

  ///
  /// \brief Notify when server startup.
  /// \param [in] pServer The server.
  ///

  virtual void onSocketServerStartup(SocketServer *pServer)
  {
  }

  ///
  /// \brief Notify when server shutdown.
  /// \param [in] pServer The server.
  /// \note If never get a SocketServerCallback::onSocketServerStartup notify
  ///       then won't get this notify.
  /// \note Existing connections will still keep connected.
  ///

  virtual void onSocketServerShutdown(SocketServer *pServer)
  {
  }

  ///
  /// \brief Notify when there is a new connection.
  /// \param [in] pServer The server.
  /// \param [in] pNewClient New client.
  /// \return Return true to accept this new connection else return false to
  ///         reject this connection and disconnect it.
  ///

  virtual bool onSocketNewClientReady(SocketServer *pServer, SocketConnection* pNewClient)
  {
    return true;
  }

  ///
  /// \brief Notify when a client is about to disconnect.
  /// \param [in] pServer The server.
  /// \param [in] pClient The client.
  /// \note If never get a SocketServerCallback::onSocketNewClientReady notify
  ///       and return true then won't get this notify.
  ///

  virtual void onSocketClientLeave(SocketServer *pServer, SocketConnection* pClient)
  {
  }

  ///
  /// \brief Notify when received a data stream from client.
  /// \param [in] pServer The server.
  /// \param [in] pClient The client.
  /// \param [in] len Data length(in byte)
  /// \param [in] pStream Data stream.
  /// \note The received data stream may be part of sent data, application should
  ///       detect it and combine data parts manually.
  ///

  virtual void onSocketStreamReady(SocketServer *pServer, SocketConnection* pClient, int len, void const* pStream)
  {
  }
};

///
/// \brief Socket client connection.
///

class SocketConnection
{
public:

  ///
  /// \brief Disconnect the connection.
  ///

  virtual void disconnect()=0;

  ///
  /// \brief Get connection state.
  /// \return See CONNECTION_STATE.
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

  virtual SocketClientStats getNetStats()=0;

  ///
  /// \brief Send a data stream to remote client.
  /// \param [in] len Data length(in byte).
  /// \param [in] pStream Data stream.
  /// \return Return true if success else return false.
  /// \note Return true doesn't mean the data is sent right away. It is possible
  ///       queued and sent later.
  /// \note The data may be sliced to several parts, receiver should combine them.
  ///

  virtual bool send(int len, void const* pStream)=0;

  uint_ptr userData;                    ///< User define data.
};

///
/// \brief Socket client.
///

class SocketClient : public SocketConnection
{
public:

  ///
  /// \brief Allocate a client instance.
  /// \param [in] pCallback Client callback interface.
  /// \return If success return an interface pointer else return 0.
  ///

  static SocketClient* alloc(SocketClientCallback* pCallback);

  ///
  /// \brief Release a unused client instance.
  /// \param [in] pClient Instance to free.
  ///

  static void free(SocketClient* pClient);

  ///
  /// \brief Connect to server.
  /// \param [in] svrAddr Address of server, format: ip:port or hostname:port.
  /// \return Return true if success else return false.
  /// \note It may not connect to the server right away if return true. To make
  ///       sure it is connected to the server you should get a notify of
  ///       SocketClient::getConnectionState or use SocketClient::getConnectionState
  ///       to check state.
  ///

  virtual bool connect(std::string const& svrAddr)=0;

  ///
  /// \brief Trigger socket.
  /// \note Application should call trigger periodically to make module works
  ///       properly.
  ///

  virtual void trigger()=0;
};

///
/// \brief Socket server.
///

class SocketServer
{
public:

  ///
  /// \brief Allocate a server instance.
  /// \param [in] pCallback Server callback.
  /// \return If success return an interface pointer else return 0.
  ///

  static SocketServer* alloc(SocketServerCallback* pCallback);

  ///
  /// \brief Release a unused server instance.
  /// \param [in] pServer Instance to free.
  ///

  static void free(SocketServer* pServer);

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
  /// \brief Trigger socket.
  /// \note Application should call trigger periodically to make module works
  ///       properly.
  ///

  virtual void trigger()=0;

  ///
  /// \brief Get statistics.
  /// \return Return statistics.
  ///

  virtual SocketServerStats getNetStats()=0;

  ///
  /// \brief Get first connection.
  /// \return Return first connection.
  ///

  virtual SocketConnection* getFirstConnection() const=0;

  ///
  /// \brief Get next connection.
  /// \param [in] pClient Current connection.
  /// \return Return next connection.
  ///

  virtual SocketConnection* getNextConnection(SocketConnection* pClient) const=0;

  ///
  /// \brief Get trigger frequency.
  /// \return Return trigger frequency setting.
  /// \note A trigger frequency is used to control the data flow of send/recv
  ///       process of a connection. Higher frequency can handle more data in
  ///       a second, lower frequency can handle less data in a second.
  ///

  virtual int getTriggerFrequency() const=0;

  ///
  /// \brief Get trigger frequency.
  /// \param [in] freq New trigger frequency.
  ///

  virtual void setTriggerFrequency(int freq)=0;

  uint_ptr userData;                    ///< User define data.
};

} // namespace sw2

// end of swSocket.h
