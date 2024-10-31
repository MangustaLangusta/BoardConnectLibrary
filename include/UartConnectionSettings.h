/*
	Board_connection_library
	by Sergei Grigorev 
	2024
	
	UartConnectionSettings header
*/

#ifndef UART_CONNECTION_SETTINGS_H
#define UART_CONNECTION_SETTINGS_H

#include <map>

#include "Declarations.h"

namespace board_connect {
	
namespace uart {
	
using BaudRate_t = long long;

enum DataLength_t { FIVE_BITS = 5, SIX_BITS = 6, SEVEN_BITS = 7, EIGHT_BITS = 8 }; 
enum Parity_t { UART_PARITY_NONE = 0, UART_PARITY_ODD = 1, UART_PARITY_EVEN = 2 };
enum StopBits_t { STOP_ONE = 1, STOP_TWO = 2 };
enum PortName_t { COM0, COM1, COM2, COM3, COM4, COM5, COM6 };

using Duration_t = std::chrono::duration<long long, std::milli>;
using std::chrono::operator""ms;


const std::map<PortName_t, const std::string> PortNameMap = { 	{ COM0, "COM0" }, 
																																{ COM1, "COM1" },
																																{ COM2, "COM2" }, 
																																{ COM3, "COM3" },
																																{ COM4, "COM4" }, 
																																{ COM5, "COM5" },
																																{ COM6, "COM6" } };
																																	
const std::string ConvertPortNameToStr(PortName_t pn) noexcept { 
	assert(PortNameMap.find(pn) != PortNameMap.end());
	return PortNameMap.at(pn); 
}



struct UartConnectionSettings : IConnectionSettings {
	
	
protected: 
	constexpr static PortName_t DEFAULT_PORT_NAME = COM1;
	constexpr static BaudRate_t DEFAULT_BAUD_RATE = 9600;
	constexpr static DataLength_t DEFAULT_DATA_LENGTH = EIGHT_BITS;
	constexpr static Parity_t DEFAULT_PARITY = UART_PARITY_NONE;
	constexpr static StopBits_t DEFAULT_STOP_BITS = STOP_ONE;
	
	constexpr static Duration_t DEFAULT_RECEIVE_LOOP_PERIOD = 200ms;
	constexpr static Duration_t DEFAULT_SEND_LOOP_PERIOD = 200ms;
	constexpr static int DEFAULT_MAX_BYTES_TO_READ_AT_ONCE = 100;
public:
	const PortName_t port;
	const BaudRate_t baud;
	const DataLength_t length;
	const Parity_t parity;
	const StopBits_t stop_bits;
	
	Duration_t receive_loop_period = DEFAULT_RECEIVE_LOOP_PERIOD;
	Duration_t send_loop_period = DEFAULT_SEND_LOOP_PERIOD;
	int max_bytes_to_read_at_once = DEFAULT_MAX_BYTES_TO_READ_AT_ONCE;
	
//for WinApi
public:
	COMMTIMEOUTS winapi_commtimeouts = DEFAULT_COMMTIMEOUTS;

public:
	UartConnectionSettings(	PortName_t pn		= DEFAULT_PORT_NAME,
													BaudRate_t b 		= DEFAULT_BAUD_RATE, 
													DataLength_t l 	= DEFAULT_DATA_LENGTH, 
													Parity_t p 			= DEFAULT_PARITY, 
													StopBits_t s 		= DEFAULT_STOP_BITS) noexcept :
													port(pn), baud(b), length(l), parity(p), stop_bits(s) {	cout<<"UartConnectionSettings created "<<endl; };
													
	virtual ~UartConnectionSettings() { cout<<"~UartConnectionSettings"<<endl; };
	
public:
	void Dump() const override  {
		cout<<"PortName = "<<ConvertPortNameToStr(port)<<endl;	
		cout<<"BaudRate = "<<baud<<endl;
		cout<<"Length = "<<length<<" bits"<<endl;
		cout<<"Parity = "<<parity<<endl;
		cout<<"StopBits = "<<stop_bits<<" bits"<<endl;
	};
};	


}	//uart


}	//board_connect

#endif	//UART_CONNECTION_SETTINGS_H