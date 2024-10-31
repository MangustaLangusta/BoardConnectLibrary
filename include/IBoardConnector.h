/*
  Board_connection_library
  by Sergei Grigorev 
  2024
  
  IBoardConnector header
*/

#ifndef I_BOARD_CONNECTOR_H
#define I_BOARD_CONNECTOR_H

#include "Declarations.h"
#include "BoardConnectBuffer.h"

namespace board_connect{

template <typename DataType>
class IBoardConnector{


protected:
  Buffer<DataType> send_buffer_;
  Buffer<DataType> receive_buffer_;

  ConnectionStatus_t current_state_ = ConnectionStatus_t::UNDEFINED;

//ctor / dtor  
public:
  IBoardConnector() {};
  IBoardConnector(const IBoardConnector& oth) = delete;
  IBoardConnector(IBoardConnector &&oth) = default;
  IBoardConnector& operator=(const IBoardConnector& oth) = delete;
  IBoardConnector& operator=(IBoardConnector&& oth) = default;
  
  virtual ~IBoardConnector() = default;
  
// methods
public:
  virtual ConnectionStatus_t Connect() = 0;
  virtual ConnectionStatus_t Status() = 0;
  virtual ConnectionStatus_t Disconnect() = 0;
  
  virtual bool Send(const DataType data) = 0;
  virtual std::optional<DataType> Receive() = 0;

};


/*  --------------------------------------------------------------------------------------------------------------------
      IBoardConnectorFactory
    --------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
class IBoardConnectorFactory{
public:
  virtual ~IBoardConnectorFactory() = default;

public:
  virtual IBoardConnector_up<DataType> MakeBoardConnector( const IConnectionSettings& connection_settings) const = 0;  
};

}  //board_connect

#endif  //I_BOARD_CONNECTOR_H