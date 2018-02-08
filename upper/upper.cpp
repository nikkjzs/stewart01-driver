// asio_client_01.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include <boost/array.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

using boost::asio::ip::udp;
boost::mutex srmutex;
boost::mutex statmutex;

typedef struct
{
	float x;
	float y;
	float z;
	float pitch;
	float yaw;
	float roll;
	float reserve1;
	float reserve2;
	float reserve3;
	float timeStamp;
	int upper_cmd;
	int dev_stat;
	bool reset;
}DATA_TO_DRIVER, *PDATA_TO_DRIVER;


enum DEVICE_STATUS
{
	status0 = 0,        //平台停止，等待开机
	status1 = 1,        //平台正在回中位，此时不响应上位机命令 
	status2 = 2,        //平台在中位 
	status3 = 3,        //平台在工作态，可正常接受运动参数 
	status6 = 6,        //平台正在开机
	status7 = 7,        //平台正在关机
	status8 = 8,        //平台故障
	status99 = 99,      //custom default value
};


enum S_CMD
{
	sComd0 = 0,
	sComd2 = 2,
	sComd4 = 4,
	sComd6 = 6,
	sComd7 = 7,
	sComd8 = 8,
	sComd99 = 99,
};



std::string make_daytime_string()
{
	//using namespace std; // For time_t, time and ctime;
	//time_t now = time(0);
	return "justmakestring";
}

class udp_server
{
public:
	udp_server(boost::asio::io_context& io_context)
		: socket_(io_context, udp::endpoint(udp::v4(), 14)), r_io_context(io_context), tar_endpoint_
		(boost::asio::ip::address_v4::from_string("127.0.0.1"),13)
	{
		//start_receive();
	}

	void start_receive_entry()
	{
		start_receive();
		r_io_context.run();
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

			DATA_TO_DRIVER recvdata = { 0 };
			recvdata = *(DATA_TO_DRIVER*)recv_buffer_;
			statmutex.lock();
			dev_stat_ = *(DEVICE_STATUS*)&recvdata.dev_stat;
			statmutex.unlock();
			std::cout << recvdata.timeStamp << std::endl;



			/*static DATA_TO_DRIVER senddata = { 0 };
			senddata.timeStamp++;
			*(DATA_TO_DRIVER*)send_buffer_ = senddata;

			socket_.async_send_to(boost::asio::buffer(send_buffer_), remote_endpoint_,boost::bind(&udp_server::handle_send, this));*/

			start_receive();
		}
	}

	void start_send()
	{
		for (;;)
		{
			static DATA_TO_DRIVER senddata = { 0 };
			

			statmutex.lock();
			senddata.timeStamp++;
			senddata.upper_cmd = orderfunc(dev_stat_);
			*(DATA_TO_DRIVER*)send_buffer_ = senddata;
			int ret = socket_.send_to(boost::asio::buffer(send_buffer_), tar_endpoint_);// , boost::bind(&udp_server::handle_send, this));
			statmutex.unlock();
		}
	}



	/*void handle_send()
	{
		start_send();
	}*/


	S_CMD orderfunc(DEVICE_STATUS curStat)
	{
		if (curStat == 4 || curStat == 5 || curStat > 8)
		{
			bool breakp = false;
		}

		S_CMD outCmd = sComd99;
		switch (curStat)
		{
		case status0:
			outCmd = sComd6;
			break;
		case status1:
			outCmd = sComd6;
			break;
		case status2:
			outCmd = sComd4;
			break;
		case status3:
			outCmd = sComd0;
			break;
		case status6:
			outCmd = sComd6;
			break;
		case status7:
			break;
		case status8:
			break;
		default:
			std::cout << "default cmd" << std::endl;
			break;
		}

		return outCmd;
	}

	udp::socket socket_;
	udp::endpoint remote_endpoint_;

	udp::endpoint tar_endpoint_;

	char recv_buffer_[255] = {0};
	char send_buffer_[255] = {0};
	boost::asio::io_context& r_io_context;
	DEVICE_STATUS dev_stat_ = status0;
	//boost::array<char, 1> recv_buffer_;
};

int main()
{
	try
	{
		boost::thread_group group;

		

		boost::asio::io_context io_context;
		udp_server server(io_context);

		group.create_thread(boost::bind(&udp_server::start_send, &server));
		group.create_thread(boost::bind(&udp_server::start_receive_entry, &server));
		

		//io_context.run();

		group.join_all();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}




//int main(int argc, char* argv[])
//{
//	try
//	{
//		if (argc != 2)
//		{
//			std::cerr << "Usage: client <host>" << std::endl;
//			return 1;
//		}
//
//		boost::asio::io_context io_context;
//
//		//udp::resolver resolver(io_context);
//		//udp::endpoint receiver_endpoint =
//		//	*resolver.resolve(udp::v4(), argv[1], "daytime").begin();
//
//		udp::endpoint receiver_endpoint(boost::asio::ip::address_v4::from_string("127.0.0.1"),13);
//		udp::socket socket(io_context);
//		socket.open(udp::v4());
//
//		boost::array<char, 1> send_buf = { { 0 } };
//		for (;;)
//		{
//			socket.send_to(boost::asio::buffer(send_buf), receiver_endpoint);
//			int err = GetLastError();
//		}
//
//		boost::array<char, 128> recv_buf;
//		udp::endpoint sender_endpoint;
//		size_t len = socket.receive_from(
//			boost::asio::buffer(recv_buf), sender_endpoint);
//
//		std::cout.write(recv_buf.data(), len);
//	}
//	catch (std::exception& e)
//	{
//		std::cerr << e.what() << std::endl;
//	}
//}
//

