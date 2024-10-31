/*
	Board_connection_library
	by Sergei Grigorev 
	2024
	
	class Board
	
*/

/*
	TODO: 
		- add multithreading
		- tests
*/

#ifndef BOARD_H
#define BOARD_H

#include <memory>
#include <optional>
#include <iostream>

#include "Declarations.h"


namespace board_connect{


using std::cout;
using std::endl;
using std::nullopt;

/*	--------------------------------------------------------------------------------------------------------------------
			class Board declaration
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
class Board{
	
	IBoardConnector_up<DataType> connector_;

public:
	//ctor
	Board(	const IConnectionSettings& settings, 
					const IBoardConnectorFactory<DataType>& connector_factory );
	
	Board(const Board& oth) = delete;
	Board(const Board&& oth);
	Board& operator=(const Board& oth) = delete;
	Board& operator=(const Board&& oth);
	virtual ~Board() = default;
	
public:
	//API

	virtual ConnectionStatus Connect();
	virtual ConnectionStatus Status() const;
	virtual ConnectionStatus Disconnect();

	//Send or operator>> for convenience
	virtual bool Send(const DataType data);
	virtual bool operator<<(const DataType data) { return Send(std::move(data)); }
	
	virtual std::optional<DataType> Receive();
	virtual void operator>>(std::optional<DataType>& target) { target = Receive(); }
};

/*	--------------------------------------------------------------------------------------------------------------------
			Board:: constructor
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
Board<DataType>::Board(	const IConnectionSettings& settings, 
												const IBoardConnectorFactory<DataType>& connector_factory ) {
	connector_ = connector_factory.MakeBoardConnector(settings);
}


/*	--------------------------------------------------------------------------------------------------------------------
			Board:: move constructor
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
Board<DataType>::Board(const Board<DataType>&& oth) {
	connector_ = std::move(oth.connector_);
}

/*	--------------------------------------------------------------------------------------------------------------------
			Board:: move assignment
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
Board<DataType>& Board<DataType>::operator=(const Board<DataType>&& oth) {
	if(this == &oth) { return *this; }
	connector_ = std::move(oth.connector_);
}


/*	--------------------------------------------------------------------------------------------------------------------
			Definitions of Board:: methods
		--------------------------------------------------------------------------------------------------------------------
*/
/*	--------------------------------------------------------------------------------------------------------------------
				Board::Connect
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
ConnectionStatus Board<DataType>::Connect(){
	return connector_->Connect();
}


/*	--------------------------------------------------------------------------------------------------------------------
				Board::Status
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
ConnectionStatus Board<DataType>::Status() const{
	return connector_->Status();
}


/*	--------------------------------------------------------------------------------------------------------------------
				Board::Disconnect
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
ConnectionStatus Board<DataType>::Disconnect(){
	return connector_->Disconnect();
}


/*	--------------------------------------------------------------------------------------------------------------------
				Board::Send
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
bool Board<DataType>::Send(const DataType data){
	return connector_->Send(data);
}


/*	--------------------------------------------------------------------------------------------------------------------
				Board::Receive
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
std::optional<DataType> Board<DataType>::Receive(){
	return connector_->Receive();
}	


}	//namespace BoardConnect

#endif 		//BOARD_H