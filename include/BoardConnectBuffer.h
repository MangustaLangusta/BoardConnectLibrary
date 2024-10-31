/*
	Board_connection_library
	by Sergei Grigorev 
	2024
	
	Buffer header
*/

#ifndef BOARD_CONNECT_BUFFER_H
#define BOARD_CONNECT_BUFFER_H

#include <list>
#include <optional>
#include "Declarations.h"

namespace board_connect {

using std::list;
using std::mutex;
using std::lock_guard;
using std::optional;
using std::nullopt;
	
	
template <typename DataType = DefaultDataType>
class Buffer {
	
	list<DataType> storage_;
	mutex storage_mutex_;
	bool await_load_confirmation_ = false;
	
public:

	virtual 										~Buffer() = default;
	virtual bool								Store(const DataType& data);
	virtual optional<DataType>	Load();												//Returns oldest parcel in queue. After this, need to call ConfirmReception().
	virtual bool								ConfirmReception();						//Removes oldest parcel from storage (this is to prevent lost of data in case of errors on caller's side)
																														//returns true if parcel is successfully erased. Consequental call of Load() will return new parcel.
};


/*	--------------------------------------------------------------------------------------------------------------------
			Buffer::Store()
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
bool Buffer<DataType>::Store(const DataType& data) {
		//std::mutex may throw exception, in which case it's not locked
		//if data throws anything (during copying), push_back has no effect. 
		//rethrow exception in both cases
	try {	
		const lock_guard<mutex> lock(storage_mutex_);
		storage_.push_back(data);	 
	}
	catch(...) {
		throw;	
	}
	return true;
}


/*	--------------------------------------------------------------------------------------------------------------------
			Buffer::Load()
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
optional<DataType> Buffer<DataType>::Load(){
	try {	//std::mutex may throw exception, in which case it's not locked. rethrow exception
		const lock_guard<mutex> lock(storage_mutex_);
	}
	catch(...) {
		throw;	
	}
	
	if(storage_.empty())
		return nullopt;
	
	await_load_confirmation_ = true;
	return storage_.front();
}


/*	--------------------------------------------------------------------------------------------------------------------
			Buffer::ConfirmReception()
		--------------------------------------------------------------------------------------------------------------------
*/
template <typename DataType>
bool Buffer<DataType>::ConfirmReception(){
	try {	//std::mutex may throw exception, in which case it's not locked. rethrow exception
		const lock_guard<mutex> lock(storage_mutex_);
	}
	catch(...) {
		throw;	
	}
	
	if(await_load_confirmation_ && !storage_.empty() ){
		storage_.pop_front();
		await_load_confirmation_ = false;
		return true;
	}
	return await_load_confirmation_ = false;
}


}	//board connect

#endif	//BOARD_CONNECT_BUFFER_H