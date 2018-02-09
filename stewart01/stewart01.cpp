// stewart01.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "01.h"
#include "02.h"
#include "device.h"

#include "mystruct.h"

#include <iostream>
#include <string>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lockfree/stack.hpp>



using boost::asio::ip::udp;

std::string make_daytime_string()
{
	return "a";
}

class udp_server
{
public:
	udp_server(boost::asio::io_context& io_context, boost::mutex& g_logMutex)
		: socket_(io_context, udp::endpoint(udp::v4(), 13)), r_io_context(io_context),
		up_(io_context, g_logMutex),
		cc_(io_context, g_logMutex),
		commuDevice_(io_context, g_logMutex)
	{
		tar_dev_endpoint_ = udp::endpoint(boost::asio::ip::address_v4::from_string("125.125.122.83"),14);
		tar_upper_endpoint_ = udp::endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"), 14);
	}

	void entry_recv_send()
	{
		start_receive();

		start_send();

		isProducerReady = true;

		r_io_context.run();
	}

	void entry_worker()
	{
		r_io_context.run();
	}

	void entry_realtimer_send()
	{
		boost::asio::io_context io;

		for (;;)
		{
			boost::asio::deadline_timer t(io, boost::posix_time::millisec(10));
			t.wait();

			if (up_.GetDoneListSize() != 0)
			{
				DATA_TO_DRIVER tmp = up_.GetMAxAndClearAll();
				*(DataToVice*)send_buffer_ = changeUpperData2ViceData(tmp);
				socket_.send_to(boost::asio::buffer(send_buffer_), tar_dev_endpoint_);
			}
			else
			{
				socket_.send_to(boost::asio::buffer(send_buffer_), tar_dev_endpoint_);
			}
		}
	}


	void start_receive()
	{
		socket_.async_receive_from(
			boost::asio::buffer(recv_buffer_), remote_endpoint_,
			boost::bind(&udp_server::handle_receive, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

	void handle_receive(const boost::system::error_code& error,
		std::size_t /*bytes_transferred*/)
	{
		if (!error || error == boost::asio::error::message_size)
		{
			branchWorkflow();
		}
	}


	void start_send()
	{
		DATA_TO_DRIVER tmp = { 0 };
		statmutex.lock();
		tmp.dev_stat = commuDevice_.vice_status_;
		*(DATA_TO_DRIVER*)send_buffer_ = tmp;
		statmutex.unlock();
		socket_.async_send_to(boost::asio::buffer(send_buffer_), tar_upper_endpoint_,
			boost::bind(&udp_server::handle_send, this));
	}

	void handle_send()
	{
		start_send();
	}

private:
	void branchWorkflow()
	{
		udp::endpoint l_remote_endpoint = remote_endpoint_;
		if (l_remote_endpoint.address().to_string() == "127.0.0.1")
		{
			up_.processBefore(recv_buffer_);
			start_receive();
			up_.processAfter();
		}
		else if (l_remote_endpoint.address().to_string() == "125.125.122.83")//lower monitor
		{
			commuDevice_.processBefore(recv_buffer_);
			start_receive();
			commuDevice_.processAfter();
		}
		else
		{
		}
	}


	DataToVice changeUpperData2ViceData(DATA_TO_DRIVER data2driver)
	{
		DataToVice data2vice = { 0 };
		data2vice.sMark = 55;
		data2vice.sComd = data2driver.upper_cmd;
		data2vice.sAtti[0] = data2driver.pitch;
		data2vice.sAtti[1] = data2driver.roll;
		data2vice.sAtti[2] = data2driver.yaw;
		data2vice.sAtti[3] = data2driver.x;
		data2vice.sAtti[4] = data2driver.z;
		data2vice.sAtti[5] = data2driver.y;
		return data2vice;
	}


public:
	bool IsProducerReady()
	{
		return isProducerReady;
	}

private:
	udp::socket socket_;
	udp::endpoint remote_endpoint_;
	udp::endpoint tar_upper_endpoint_;
	udp::endpoint tar_dev_endpoint_;
	char send_buffer_[256];
	char recv_buffer_[256];
	boost::asio::io_context& r_io_context;

	cUpper up_;
	cControlCenter cc_;
	cCummuDevice commuDevice_;

	bool isProducerReady = false;

	boost::mutex io_mutex;
	boost::mutex statmutex;
};


int main()
{
	try
	{
		boost::mutex g_logMutex;

		boost::asio::io_context io_context;

		udp_server server(io_context,g_logMutex);

		boost::thread_group group;

		for (int num = 0; num < 4; ++num)
		{
			if (num == 0)
				group.create_thread(boost::bind(&udp_server::entry_recv_send,&server));
			else if (num == 1)
				group.create_thread(boost::bind(&udp_server::entry_realtimer_send, &server));
			else
				group.create_thread(boost::bind(&udp_server::entry_worker, &server));

			for (;;)
			{
				if (server.IsProducerReady())
				{
					break;
				}
			}
		}
		group.join_all(); //等待所有线程执行结束
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}
