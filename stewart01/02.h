#pragma once

#include "00.h"

#include "mystruct.h"
#include <boost/lockfree/stack.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <iostream>

class cControlCenter : public cBaseUpper
{
public:
	cControlCenter(boost::asio::io_context& ioContext,boost::mutex& g_logMutex) : cBaseUpper(ioContext, g_logMutex)
	{

	}

	virtual void process01(DATA_TO_DRIVER newer_data)
	{

	}
};