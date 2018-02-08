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
	unsigned char sMark;     //��־λ��Ĭ��Ϊ55��   
	unsigned char sComd;     //��λ������ 
	unsigned char sRepa[2];  //��Ĭ��Ϊ0�� 
	float sAtti[6]; //������̬�������£��ã�X��Y��Z�� 
	float sVelo[6]; //��Ĭ��ֵΪ0�� 
	float sAcce[3]; //���ٶȣ�Ĭ��ֵΪ0�� 
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
	sComd0 = 0,
	sComd2 = 2,
	sComd4 = 4,
	sComd6 = 6,
	sComd7 = 7,
	sComd8 = 8,
	sComd99 = 99,
};


