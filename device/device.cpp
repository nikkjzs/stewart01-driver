// asio_client_01.cpp : �������̨Ӧ�ó������ڵ㡣
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

//git
typedef struct
{
	BYTE sMark;     //��־λ��Ĭ��Ϊ55��   
	BYTE sComd;     //��λ������ 
	BYTE sRepa[2];  //��Ĭ��Ϊ0�� 
	float sAtti[6]; //������̬�������£��ã�X��Y��Z�� 
	float sVelo[6]; //��Ĭ��ֵΪ0�� 
	float sAcce[3]; //���ٶȣ�Ĭ��ֵΪ0�� 
}DataToVice;

typedef struct
{
	BYTE rMark;       //��־λ 
	BYTE rComd;         //��λ��״̬ 
	BYTE rPara[2];      //��Ĭ��Ϊ0�� rPara[0]ΪDI���롣
	float rAtti[6];     //��λ��ʵ����̬ 
	float rRese[6];     //��λ��״̬ 
	float rMoto[6];      //�����ֵ 
}DataToMain;




enum DEVICE_STATUS
{
	status0 = 0,        //ƽֹ̨ͣ���ȴ�����
	status1 = 1,        //ƽ̨���ڻ���λ����ʱ����Ӧ��λ������ 
	status2 = 2,        //ƽ̨����λ 
	status3 = 3,        //ƽ̨�ڹ���̬�������������˶����� 
	status6 = 6,        //ƽ̨���ڿ���
	status7 = 7,        //ƽ̨���ڹػ�
	status8 = 8,        //ƽ̨����
	status99 = 99,      //custom default value
};


enum S_CMD
{
	sComd0 = 0,      //���������˶����� 
	sComd2 = 2,      //����ƽ̨�ص�����λ 
	sComd4 = 4,      //����Э�飺ƽ̨�л�������̬ 
	sComd6 = 6,      //�������ƽ̨�ɵ�λ����������λ 
	sComd7 = 7,      //�ػ����ƽ̨������λ�ص���λ 
	sComd8 = 8,       //���� 
};


//sComd = 0,      //���������˶����� 
//sComd = 2,      //����ƽ̨�ص�����λ 
//sComd = 4,      //����Э�飺ƽ̨�л�������̬ 
//sComd = 6,      //�������ƽ̨�ɵ�λ����������λ 
//sComd = 7,      //�ػ����ƽ̨������λ�ص���λ 
//sComd = 8       //���� 




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
		: socket_(io_context, udp::endpoint(udp::v4(), 15)), r_io_context(io_context), tar_endpoint_
		//(boost::asio::ip::address_v4::from_string("192.168.2.151"),13)
		(boost::asio::ip::address_v4::from_string("127.0.0.1"), 14)
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

			DataToVice recvdata = { 0 };
			recvdata = *(DataToVice*)recv_buffer_;

			statmutex.lock();
			DEVICE_STATUS changedStat = changestatus(dev_stat_, recvdata.sComd);
			dev_stat_ = changedStat;
			statmutex.unlock();

			//std::cout << recvdata.timeStamp << std::endl;


			start_receive();

			/*DATA_TO_DRIVER recvdata = { 0 };
			recvdata = *(DATA_TO_DRIVER*)recv_buffer_;
			
			statmutex.lock();
			DEVICE_STATUS changedStat = changestatus(dev_stat_, recvdata.upper_cmd);
			dev_stat_ = changedStat;
			statmutex.unlock();

			std::cout << recvdata.timeStamp << std::endl;


			start_receive();*/
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


			//socket_.async_send_to(boost::asio::buffer(send_buffer_), remote_endpoint_, boost::bind(&udp_server::handle_send, this));
			static DataToMain senddata = { 0 };
			//senddata.timeStamp++;
			*(DataToMain*)send_buffer_ = senddata;
			statmutex.lock();
			senddata.rComd = dev_stat_;
			int ret = socket_.send_to(boost::asio::buffer(send_buffer_), tar_endpoint_);// , boost::bind(&udp_server::handle_send, this));
			statmutex.unlock();

			int err = GetLastError();
		}


		//for (;;)
		//{
		//	

		//	//socket_.async_send_to(boost::asio::buffer(send_buffer_), remote_endpoint_, boost::bind(&udp_server::handle_send, this));
		//	static DATA_TO_DRIVER senddata = { 0 };
		//	senddata.timeStamp++;
		//	*(DATA_TO_DRIVER*)send_buffer_ = senddata;
		//	statmutex.lock();
		//	senddata.dev_stat = dev_stat_;
		//	int ret = socket_.send_to(boost::asio::buffer(send_buffer_), tar_endpoint_);// , boost::bind(&udp_server::handle_send, this));
		//	statmutex.unlock();

		//	int err = GetLastError();
		//}
	}

	//void handle_send(boost::shared_ptr<std::string> /*message*/,
	//	const boost::system::error_code& /*error*/,
	//	std::size_t /*bytes_transferred*/)
	void handle_send()
	{
		start_send();
	}


	DEVICE_STATUS changestatus(int curStat, int cmd)
	{
		DEVICE_STATUS outStat = *(DEVICE_STATUS*)&curStat;
		switch (curStat)
		{
		case status0://ƽֹ̨ͣ���ȴ�����
			if (cmd == sComd6)
				outStat = status6;
			break;
		case status1://ƽ̨���ڻ���λ����ʱ����Ӧ��λ������ 
			if (cmd == sComd2)
				outStat = status2;
			break;
		case status2://ƽ̨����λ 
			if (cmd == sComd7)
				outStat = status7;
			else if (cmd == sComd4)
				outStat = status3;
			break;
		case status3://ƽ̨�ڹ���̬�������������˶����� 
			if (cmd == sComd7)
				outStat = status7;
			else if (cmd == sComd2)
				outStat = status1;
			break;
		case status6://ƽ̨���ڿ���
			if (cmd == sComd6)
				outStat = status2;
			break;
		case status7://ƽ̨���ڹػ�
			if (cmd == sComd7)
				outStat = status7;
			break;
		case status8://ƽ̨����
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
	//boost::array<char, 1> recv_buffer_;

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

