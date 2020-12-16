#pragma once
#include <string>
#include "CSerialPort/SerialPort.h"
#include "CSerialPort/SerialPortInfo.h"

using namespace itas109;


class CPublic
{
public:
	CPublic();
	virtual ~CPublic();

public:
	static bool test1;
	//�ж��Ƿ񵽴�
	static bool ArriveFlag;
	//�ж�ʶ���Ƿ����
	static bool IdentifyDone;
	//��ʼִ��ʶ��
	static bool ExecuteIdentify;

	static std::string Port;
	static int BaudRate;
	static int Parity;
	static int DataBits;
	static int Stop;


	//�ڴ�й©
	//static CSerialPort m_SerialPort;//About CSerialPort
public:
	////����CRC16��ȫ�ֺ���
	//static unsigned short CRC16(unsigned char* puchMsg, unsigned short usDataLen);
	//
	////���ͺ���
	//static void SendData(int CommTypeIn, WORD DownAdd, DWORD DownData);

};



