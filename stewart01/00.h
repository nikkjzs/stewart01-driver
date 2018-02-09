#pragma once



#include "mystruct.h"
#include <boost/lockfree/stack.hpp>
#include <boost/thread/thread.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/container/list.hpp>
#include <boost/thread/mutex.hpp>
#include <iostream>



class cBaseUpper
{
public:
	cBaseUpper(boost::asio::io_context& ioContext,boost::mutex& g_logContex):r_io_context(ioContext),logMutex(g_logContex)
	{
		last_new_data.timeStamp = -1;
	}


	void processBefore(char* recv_buffer)
	{
		discardobsoleteData(recv_buffer);
	}


	void processAfter()
	{
		process0();
	}


	void discardobsoleteData(char* recv_buffer)
	{
		DATA_TO_DRIVER l_recv_data = *(DATA_TO_DRIVER*)recv_buffer;

		if (l_recv_data.timeStamp > last_new_data.timeStamp)
		{
			ready_process_data_stack.push(l_recv_data);
			last_new_data = l_recv_data;
		}
	}



	void process0()
	{
		if (ready_process_data_stack.empty() == true)
		{
			return;
		}

		//取出待处理数据集合中最新的那条数据
		DATA_TO_DRIVER newer_data = { 0 };
		DATA_TO_DRIVER tmp_data = { 0 };
		for (;;)
		{
			if (ready_process_data_stack.pop(tmp_data) == false)
			{
				break;//emtpy
			}

			if (tmp_data.timeStamp > newer_data.timeStamp)
			{
				newer_data = tmp_data;
			}
		}

		//处理数据
		process01(newer_data);

		logMutex.lock();
		std::cout << boost::this_thread::get_id() << "===" << newer_data.timeStamp << std::endl;
		logMutex.unlock();
		return;
	}


	virtual void process01(DATA_TO_DRIVER newer_data) = 0
	{

	}


	size_t GetDoneListSize()
	{
		outCacheSetMutex.lock();
		size_t s = done_process_data_list.size();
		outCacheSetMutex.unlock();

		return s;
	}


	struct nodeForSrot {
		bool operator()(const DATA_TO_DRIVER& t1, const DATA_TO_DRIVER& t2) {
			return t1.timeStamp < t2.timeStamp;    //会产生升序排序,若改为>,则变为降序
		}
	};


	DATA_TO_DRIVER GetAndDelMin()//tmp
	{
		outCacheSetMutex.lock();
		done_process_data_list.sort(nodeForSrot());
		DATA_TO_DRIVER minData = done_process_data_list.front();
		done_process_data_list.erase(done_process_data_list.begin());
		outCacheSetMutex.unlock();
		return minData;
	}


	DATA_TO_DRIVER GetMAxAndClearAll()//tmp
	{
		outCacheSetMutex.lock();
		done_process_data_list.sort(nodeForSrot());
		DATA_TO_DRIVER minData = done_process_data_list.back();
		done_process_data_list.clear();
		outCacheSetMutex.unlock();
		return minData;
	}
	


	boost::lockfree::stack<DATA_TO_DRIVER> ready_process_data_stack;
	boost::container::list<DATA_TO_DRIVER> done_process_data_list;//ys

	DATA_TO_DRIVER last_new_data;
	boost::asio::io_context& r_io_context;
	boost::mutex outCacheSetMutex;
	boost::mutex& logMutex;
};
