/*
  Board_connection_library
  by Sergei Grigorev 
  2024
  
  header BoardConnect
  
*/

#ifndef BOARD_CONNECT_H
#define BOARD_CONNECT_H


#include "Declarations.h"
#include "Board.h"

#include "UartConnectionSettings.h"
#include "UartBoardConnector.h"



namespace board_connect{


template <typename DataType = DefaultDataType>
Board<DataType> MakeUartBoard(const uart::UartConnectionSettings& uart_settings = uart::UartConnectionSettings()) { 
  return Board<DataType>(  uart_settings, uart::UartBoardConnectorFactory<DataType>()); 
};



}  //board_connect

#endif  //BOARD_CONNECT_H