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

typedef struct
{
	BYTE sMark;     //标志位（默认为55）   
	BYTE sComd;     //上位机命令 
	BYTE sRepa[2];  //（默认为0） 
	float sAtti[6]; //六个姿态（α，β，γ，X，Y，Z） 
	float sVelo[6]; //（默认值为0） 
	float sAcce[3]; //加速度（默认值为0） 
}DataToVice;

typedef struct
{
	BYTE rMark;       //标志位 
	BYTE rComd;         //下位机状态 
	BYTE rPara[2];      //（默认为0） rPara[0]为DI输入。
	float rAtti[6];     //下位机实际姿态 
	float rRese[6];     //下位机状态 
	float rMoto[6];      //电机码值 
}DataToMain;




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
	sComd0 = 0,      //正常发送运动参数 
	sComd2 = 2,      //控制平台回到中立位 
	sComd4 = 4,      //握手协议：平台切换到工作态 
	sComd6 = 6,      //开机命令：平台由底位上升到中立位 
	sComd7 = 7,      //关机命令：平台由中立位回到底位 
	sComd8 = 8,       //保留 
};





std::string make_daytime_string()
{
	return "justmakestring";
}


class udp_server
{
public:
	udp_server(boost::asio::io_context& io_context)
		: socket_(io_context, udp::endpoint(udp::v4(), 14)), r_io_context(io_context), tar_endpoint_
		(boost::asio::ip::address_v4::from_string("192.168.2.151"),13)
	{
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

			DataToVice recvdata = { 0 };
			recvdata = *(DataToVice*)recv_buffer_;

			statmutex.lock();
			DEVICE_STATUS changedStat = changestatus(dev_stat_, recvdata.sComd);
			dev_stat_ = changedStat;
			statmutex.unlock();

			start_receive();
		}
	}


	void start_send_entry()
	{
		start_send();
		r_io_context.run();
	}


	void start_send()
	{
		for (;;)
		{
			static DataToMain senddata = { 0 };
			*(DataToMain*)send_buffer_ = senddata;
			statmutex.lock();
			senddata.rComd = dev_stat_;
			int ret = socket_.send_to(boost::asio::buffer(send_buffer_), tar_endpoint_);
			statmutex.unlock();

			int err = GetLastError();
		}
	}


	void handle_send()
	{
		start_send();
	}


	DEVICE_STATUS changestatus(int curStat, int cmd)
	{
		DEVICE_STATUS outStat = *(DEVICE_STATUS*)&curStat;
		switch (curStat)
		{
		case status0://平台停止，等待开机
			if (cmd == sComd6)
				outStat = status6;
			break;
		case status1://平台正在回中位，此时不响应上位机命令 
			if (cmd == sComd2)
				outStat = status2;
			break;
		case status2://平台在中位 
			if (cmd == sComd7)
				outStat = status7;
			else if (cmd == sComd4)
				outStat = status3;
			break;
		case status3://平台在工作态，可正常接受运动参数 
			if (cmd == sComd7)
				outStat = status7;
			else if (cmd == sComd2)
				outStat = status1;
			break;
		case status6://平台正在开机
			if (cmd == sComd6)
				outStat = status2;
			break;
		case status7://平台正在关机
			if (cmd == sComd7)
				outStat = status7;
			break;
		case status8://平台故障
			break;
		case status99://custom default value
			break;
		default:
			std::cout << "non-status" << std::endl;
			break;
		}

		std::cout << curStat << std::endl;
		return outStat;
	}

	udp::socket socket_;
	udp::endpoint remote_endpoint_;

	udp::endpoint tar_endpoint_;

	char recv_buffer_[255] = {0};
	char send_buffer_[255] = {0};
	boost::asio::io_context& r_io_context;

	DEVICE_STATUS dev_stat_ = status0;
};


int main()
{
	try
	{
		boost::thread_group group;

		boost::asio::io_context io_context;
		udp_server server(io_context);

		group.create_thread(boost::bind(&udp_server::start_send_entry, &server));
		group.create_thread(boost::bind(&udp_server::start_receive_entry, &server));

		group.join_all();
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

	return 0;
}



