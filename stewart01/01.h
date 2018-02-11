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
		process001(newer_data);
	}


	bool averagefiltering(DATA_TO_DRIVER newer_data, DATA_TO_DRIVER& filted_data)
	{
		int filtrange = 10;
		static int curIdxRange = 0;
		static float averagevalue[6] = { 0 };

		if (curIdxRange >= filtrange)
		{
			averagevalue[0] = (averagevalue[0] * (filtrange - 1) + newer_data.x)	/	filtrange;
			averagevalue[1] = (averagevalue[1] * (filtrange - 1) + newer_data.y)	/	filtrange;
			averagevalue[2] = (averagevalue[2] * (filtrange - 1) + newer_data.z)	/	filtrange;
			averagevalue[3] = (averagevalue[3] * (filtrange - 1) + newer_data.pitch)/	filtrange;
			averagevalue[4] = (averagevalue[4] * (filtrange - 1) + newer_data.yaw)	/	filtrange;
			averagevalue[5] = (averagevalue[5] * (filtrange - 1) + newer_data.roll)	/	filtrange;

			filted_data.x		= averagevalue[0];
			filted_data.y		= averagevalue[1];
			filted_data.z		= averagevalue[2];
			filted_data.pitch	= averagevalue[3];
			filted_data.yaw		= averagevalue[4];
			filted_data.roll	= averagevalue[5];

			filted_data.timeStamp = newer_data.timeStamp;
			filted_data.upper_cmd = newer_data.upper_cmd;
			filted_data.reset = newer_data.reset;
			return true;
		}
		else
		{
			averagevalue[0] = (averagevalue[0] * (curIdxRange) + newer_data.x)	/ (curIdxRange + 1);
			averagevalue[1] = (averagevalue[1] * (curIdxRange) + newer_data.y)	/ (curIdxRange + 1);
			averagevalue[2] = (averagevalue[2] * (curIdxRange) + newer_data.z)	/ (curIdxRange + 1);
			averagevalue[3] = (averagevalue[3] * (curIdxRange) + newer_data.pitch)	/ (curIdxRange + 1);
			averagevalue[4] = (averagevalue[4] * (curIdxRange) + newer_data.yaw)	/ (curIdxRange + 1);
			averagevalue[5] = (averagevalue[5] * (curIdxRange) + newer_data.roll)	/ (curIdxRange + 1);
			curIdxRange++;

			return false;
		}
	}


	void process001(DATA_TO_DRIVER data2driver)
	{
		outCacheSetMutex.lock();
		DATA_TO_DRIVER filted_data = { 0 };
		if (averagefiltering(data2driver, filted_data) == true)
		{
			done_process_data_list.push_back(filted_data);
		}
		outCacheSetMutex.unlock();
	}
};
