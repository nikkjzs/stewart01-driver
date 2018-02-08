#pragma once
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
	unsigned char sMark;     //标志位（默认为55）   
	unsigned char sComd;     //上位机命令 
	unsigned char sRepa[2];  //（默认为0） 
	float sAtti[6]; //六个姿态（α，β，γ，X，Y，Z） 
	float sVelo[6]; //（默认值为0） 
	float sAcce[3]; //加速度（默认值为0） 
}DataToVice;


typedef struct 
{
	unsigned char rMark;
	unsigned char rComd;
	unsigned char rPara[2];
	float rAtti[6];
	float rRese[6];
	float rMoto[6];
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
	sComd0 = 0,
	sComd2 = 2,
	sComd4 = 4,
	sComd6 = 6,
	sComd7 = 7,
	sComd8 = 8,
	sComd99 = 99,
};


