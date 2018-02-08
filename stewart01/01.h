#pragma once


#include "00.h"
#include "mystruct.h"
#include <boost/lockfree/stack.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>



class cUpper : public cBaseUpper
{
public:
	cUpper(boost::asio::io_context& ioContext, boost::mutex& g_logContex) : cBaseUpper(ioContext,g_logContex)
	{

	}
	

	virtual void process01(DATA_TO_DRIVER newer_data)
	{
		//chuli
		//r_io_context.post(boost::bind(&cUpper::process001, this));

		//DataToMain data2main = {0};//tmp
		//DATA_TO_DRIVER data = { 0 };
		process001(newer_data);
		
		
	}

	/*void process001(DataToVice data2vice)
	{
		outCacheSetMutex.lock();
		done_process_data_list.push_back(data2vice);
		outCacheSetMutex.unlock();
	}*/

	void process001(DATA_TO_DRIVER data2vice)
	{
		outCacheSetMutex.lock();
		done_process_data_list.push_back(data2vice);
		outCacheSetMutex.unlock();
	}
};
