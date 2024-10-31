/*
	Board_connection_library
	by Sergei Grigorev 
	2024
	
	UartBoardConnector header
*/

#ifndef UART_BOARD_CONNECTOR_H
#define UART_BOARD_CONNECTOR_H

#include "Declarations.h"
#include "IBoardConnector.h"
#include "UartConnectionSettings.h"

namespace board_connect {
	
namespace uart {
	
	
/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
class UartBoardConnector : public IBoardConnector<DataType> {
private:
	const UartConnectionSettings uart_settings_;
	Handler handler_;	
	
	struct ThreadWrapper{
		thread th;
		atomic_bool join_request;
	};
	
	ThreadWrapper sender_thread_;
	ThreadWrapper receiver_thread_;
	
	atomic_bool disconnection_in_progress_flag_;
	atomic_bool connection_in_progress_flag_;
	
private:
	bool InitializeCOMPort() noexcept;
	void ReleaseCOMPort() noexcept;
	void StartSenderService();
	void StartReceiverService();
	void StopSenderService() noexcept;
	void StopReceiverService() noexcept;
	void SenderLoop() noexcept;
	void ReceiverLoop() noexcept;
	void SenderLoopErrorHandler() noexcept;
	void ReceiverLoopErrorHandler() noexcept;
	
public:
	UartBoardConnector( const IConnectionSettings& uart_settings) 
		: uart_settings_(static_cast<const UartConnectionSettings&>(uart_settings)) {}
		
	virtual ~UartBoardConnector() = default;
	
public:
	ConnectionStatus_t Connect() override;
	ConnectionStatus_t Status() noexcept override;
	ConnectionStatus_t Disconnect() noexcept override;
	
	bool Send(const DataType data) override;
	std::optional<DataType> Receive() override;
	
};	


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnectorFactory
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
class UartBoardConnectorFactory : public IBoardConnectorFactory<DataType> {
public:
	~UartBoardConnectorFactory() override {}
public:
	IBoardConnector_up<DataType> MakeBoardConnector( const IConnectionSettings& connection_settings) const override;
};


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector methods
			UartBoardConnector::InitializeCOMPort
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
bool UartBoardConnector<DataType>::InitializeCOMPort() noexcept {
	try	{
		LPCSTR port_name = uart::ConvertPortNameToStr(uart_settings_.port).data();
		
		/*  - - - -  - - -  - */
		//Get handle
		handler_ = CreateFile(	port_name, 
														GENERIC_READ | GENERIC_WRITE, 
														0, 
														0, 
														OPEN_EXISTING, 
														FILE_ATTRIBUTE_NORMAL, 
														0);
														
		if(handler_ == INVALID_HANDLE_VALUE)	{
			if(GetLastError()==ERROR_FILE_NOT_FOUND) {
				throw std::runtime_error("Serial port does not exist");
			}
			else {
				throw std::runtime_error("Error during COM port initialization");
			}
		}	
		
		/*  - - - -  - - -  - */
		//Set Communication timeouts	
		COMMTIMEOUTS commtimeouts = uart_settings_.winapi_commtimeouts;
		
		if(!SetCommTimeouts(handler_, &commtimeouts)){
			throw std::runtime_error("Error when setting timeouts");
		}
		
		/* - - - - - -  */
		//setting serial port params
		DCB dcbSerialParams = {0};
		dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
		if (!GetCommState(handler_, &dcbSerialParams))	{
			throw std::runtime_error("Error when getting dcbSerialParams");
		}
		dcbSerialParams.BaudRate = uart_settings_.baud;
		dcbSerialParams.ByteSize = uart_settings_.length;
		dcbSerialParams.StopBits = uart_settings_.stop_bits;
		dcbSerialParams.Parity = uart_settings_.parity;
		if(!SetCommState(handler_, &dcbSerialParams)) {
			throw std::runtime_error("Error when setting serial params");
		}
		
	}
	catch(std::runtime_error& err){
		cout<<err.what()<<endl;
		ReleaseCOMPort();
		return false;
	}
	return true;
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::ReleaseCOMPort
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
void UartBoardConnector<DataType>::ReleaseCOMPort() noexcept {
	if(handler_ != INVALID_HANDLE_VALUE) {
		CloseHandle(handler_);	
	}
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::Connect
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
ConnectionStatus_t UartBoardConnector<DataType>::Connect() {

	if(this->current_state_ == ConnectionStatus_t::CONNECTED_OK) 
		Disconnect();
	
	bool COM_initialized = InitializeCOMPort();
	if(!COM_initialized){
		return this->current_state_ = ConnectionStatus_t::CONNECTION_ERROR;
	}
	
	try {
		StartSenderService();
		StartReceiverService();
	}
	catch(std::exception& err) {
		Disconnect();
		cout<<"Unable to start sender / receiver services. Connection cancelled"<<endl;
		return this->current_state_ = ConnectionStatus_t::OTHER_ERROR;
	}
	
	return this->current_state_ = ConnectionStatus_t::CONNECTED_OK;
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::Status
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
ConnectionStatus_t UartBoardConnector<DataType>::Status() noexcept {
	
	switch(this->current_state_){
		
	case ConnectionStatus_t::CONNECTED_OK:
		if(handler_ == INVALID_HANDLE_VALUE){
			this->current_state_ = ConnectionStatus_t::CONNECTION_LOST;
		}
		break;
		
	default:
		break;
	}
	
	//in other cases just return current state
	return this->current_state_;
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::Disconnect
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
ConnectionStatus_t UartBoardConnector<DataType>::Disconnect() noexcept {
	StopSenderService();
	StopReceiverService();
	ReleaseCOMPort();
	return ConnectionStatus_t::DISCONNECTED_OK;
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::Send
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
bool UartBoardConnector<DataType>::Send(const DataType data) {
	if(this->current_state_ != ConnectionStatus_t::CONNECTED_OK)
		return false;
	return this->send_buffer_.Store(data);
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::Receive
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
optional<DataType> UartBoardConnector<DataType>::Receive() {
	auto rx_data = this->receive_buffer_.Load();
	if(rx_data != nullopt) {
		this->receive_buffer_.ConfirmReception();
	}
	return rx_data;
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::SenderLoop
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
void UartBoardConnector<DataType>::SenderLoop() noexcept {
	auto& stop_request_atomic = sender_thread_.join_request;
	while(!stop_request_atomic.load(std::memory_order_relaxed)) {
		/*	sender loop routine	*/
		
		try{
			auto send_parcel = this->send_buffer_.Load();
	
			if(send_parcel){
				const char* raw_str = Data(*send_parcel);
				const int len = std::strlen(raw_str);
				DWORD bytes_written{};
				bool send_result = WriteFile(handler_, raw_str, len, &bytes_written, nullptr);
				if(send_result == false){
					/* error handling */
					throw std::runtime_error("Error during sending parcel");
				}
				else {
					/* sended OK */
					this->send_buffer_.ConfirmReception();
					continue;
				}
			}	//if(send_parcel)
		
		}		//try
		catch(...){
			SenderLoopErrorHandler();
		}
		
		/* 	end of sender loop routine	*/
		std::this_thread::sleep_for(uart_settings_.send_loop_period);
	}
	cout<<"Sender loop finished"<<endl;
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::ReceiverLoop
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
void UartBoardConnector<DataType>::ReceiverLoop() noexcept {
	auto& stop_request_atomic = receiver_thread_.join_request;
	
	DWORD actually_received;
	const DWORD max_bytes_to_read = uart_settings_.max_bytes_to_read_at_once;
	char rx_buffer[max_bytes_to_read];
	
	while(!stop_request_atomic.load(std::memory_order_relaxed)) {
		/*	receiver loop routine	*/
		
		try{
			
			bool read_result = ReadFile(handler_, rx_buffer, max_bytes_to_read, &actually_received, nullptr);
			if(!read_result) {
				throw std::runtime_error("Error during reading COM port");
			}
			if(actually_received > 0){
				this->receive_buffer_.Store(Data(rx_buffer, actually_received));
				continue;
			}
		}	//try
		catch(...){
			ReceiverLoopErrorHandler();
		}
		
		/* 	end of receiver loop routine	*/
		std::this_thread::sleep_for(uart_settings_.receive_loop_period);
	}
	cout<<"Receiver loop finished"<<endl;
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::SenderLoopErrorHandler
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
void UartBoardConnector<DataType>::SenderLoopErrorHandler(){
	cout<<"Error in sender loop. Disconnection"<<endl;
	
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::StartSenderService
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
void UartBoardConnector<DataType>::StartSenderService() {
	//..
	sender_thread_.join_request.store(false, std::memory_order_relaxed);
	
	//exception may be thrown if's impossible to create a new thread
	sender_thread_.th = thread{&UartBoardConnector<DataType>::SenderLoop, this};
	cout<<"SenderLoop started"<<endl;
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::StartReceiverService
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
void UartBoardConnector<DataType>::StartReceiverService() {
	//..
	receiver_thread_.join_request.store(false, std::memory_order_relaxed);
	
	//exception may be thrown if's impossible to create a new thread
	receiver_thread_.th = thread{&UartBoardConnector<DataType>::ReceiverLoop, this};
	cout<<"Receiver Loop started"<<endl;
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::StopSenderService
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
void UartBoardConnector<DataType>::StopSenderService() noexcept{
	sender_thread_.join_request.store(true, std::memory_order_relaxed);
	if(sender_thread_.th.joinable()) {
		sender_thread_.th.join();
	}
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnector::StopReceiverService
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
void UartBoardConnector<DataType>::StopReceiverService() noexcept{
	receiver_thread_.join_request.store(true, std::memory_order_relaxed);
	if(receiver_thread_.th.joinable()) {
		receiver_thread_.th.join();
	}
}


/*	--------------------------------------------------------------------------------------------------------------------
			UartBoardConnectorFactory::MakeBoardConnector
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
IBoardConnector_up<DataType> UartBoardConnectorFactory<DataType>::MakeBoardConnector(const IConnectionSettings& connection_settings) const {
	return std::make_unique<UartBoardConnector<DataType>>(connection_settings);
}
	
} //uart

}	//board_connect

#endif