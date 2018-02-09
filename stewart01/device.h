#pragma once
#include "mystruct.h"
#include <boost/lockfree/stack.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/container/list.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>

class cCummuDevice
{
public:
	cCummuDevice(boost::asio::io_context& io_context, boost::mutex& g_logMutex) : r_io_context(io_context), logMutex(g_logMutex)
	{
	}

	void processBefore(char* recv_buffer)
	{
		updataDeviceStatus(recv_buffer);
	}

	void updataDeviceStatus(char* recv_buffer)
	{
		DEVICE_STATUS l_vice_stat = (DEVICE_STATUS)(*(DataToMain*)recv_buffer).rComd;
		vice_status_ = l_vice_stat;
	}


	void processAfter()
	{
		logMutex.lock();
		std::cout << vice_status_ << std::endl;
		logMutex.unlock();
	}


	DEVICE_STATUS GetCurStat()
	{
		return vice_status_;
	}

	DEVICE_STATUS vice_status_ = status0;
	boost::mutex& logMutex;
	boost::asio::io_context& r_io_context;
};