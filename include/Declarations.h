/*
  Board_connection_library
  by Sergei Grigorev 
  2024
  
  Declarations header
*/

#ifndef DECLARATIONS_H
#define DECLARATIONS_H

#include <string>
#include <cstring>
#include <set>
#include <memory>
#include <limits>
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>

#include <cassert>

//#define NDEBUG

#include <winsock2.h>
#include <windows.h>

//#pragma comment(lib, "Ws2_32.lib")
//GCC does not support this. Use  g++ ... -lWs2_32


namespace board_connect{
  
//For debug and testing
using std::cout;
using std::endl;
using std::thread;
using std::atomic_bool;
  
using DefaultDataType = std::string;

using Handler = HANDLE;

template <typename DataType> class IBoardConnector;
class IConnectionSettings;

template <typename DataType> class IBoardConnectorFactory;

using IConnectionSettings_up = std::unique_ptr<const IConnectionSettings>;

template <typename DataType>
using IBoardConnector_up = std::unique_ptr< IBoardConnector<DataType> >;

enum class ConnectionStatus_t { UNDEFINED, CONNECTED_OK, DISCONNECTED_OK, CONNECTION_LOST, CONNECTION_ERROR, OTHER_ERROR, CONNECTION_IN_PROGRESS, DISCONNECTION_IN_PROGRESS };


/*  --------------------------------------------------------------------------------------------------------------------
      constants for WinApi
    --------------------------------------------------------------------------------------------------------------------
*/
const COMMTIMEOUTS DEFAULT_COMMTIMEOUTS{ 
  MAXDWORD,    // ReadIntervalTimeout
  0,          // ReadTotalTimeoutMultiplier
  0,          // ReadTotalTimeoutConstant
  0,          // WriteTotalTimeoutMultiplier
  0            // WriteTotalTimeoutConstant
};


/*  --------------------------------------------------------------------------------------------------------------------
      ConnectionStatus
    --------------------------------------------------------------------------------------------------------------------
*/
struct ConnectionStatus{
  
  //fields
  const ConnectionStatus_t value;
  
  operator int() const { return static_cast<int>(value);  }
  bool operator==(const ConnectionStatus_t oth) { return (value == oth); }

  //ctor / dtor
  ConnectionStatus(ConnectionStatus_t new_val) : value(new_val) {}
  virtual ~ConnectionStatus() = default;
};


/*  --------------------------------------------------------------------------------------------------------------------
      IConnectionSettings
    --------------------------------------------------------------------------------------------------------------------
*/
class IConnectionSettings{
  
public:
 virtual ~IConnectionSettings() = default;
 
public:
  virtual void Dump() const = 0;
};


/*  --------------------------------------------------------------------------------------------------------------------
      Getting data from DataType (string by default)
    --------------------------------------------------------------------------------------------------------------------
*/
char* Data(DefaultDataType obj){
  return obj.data();
}

/*  --------------------------------------------------------------------------------------------------------------------
      convert raw data (array of char) into DataType (to string by default)
    --------------------------------------------------------------------------------------------------------------------
*/
DefaultDataType Data(char* raw_arr, int size){
  return DefaultDataType(raw_arr, size);
}


}  //board_connect

#endif  //DECLARATIONS_H