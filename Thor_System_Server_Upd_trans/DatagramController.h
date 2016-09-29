#pragma once
#ifndef _DatagramControlerH_
#define _DatagramControlerH_

#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <fstream>

class DatagramController
{
public:
	DatagramController(std::stringstream &coded_cloud, int buffer_length, int sender_number, int frame_number, int timeout, int max_possble_send);
	~DatagramController();
	int ReturnNextDatagramNumber();
	void ReturnFrameInfoDatagram(char *buffer, int buffer_length);
	void ReturnFrameDatgram(int number_of_demand_datagram, char *buffer, int buffer_length);
	void IncreaseWaitTime();
	void CheckTimeout();
	void ConfimDatagram(int number_of_datagram);
	void SetTransmitionPermission();
	void SetNumOfDataConfirmation();
	bool AbleToTransmit();
	bool RecieverKnowDataNum();
	bool SendedTooMuch();
	int ReturnActualFrame();


	int RetNumDAta();
private:
	struct DatagramData
	{
		std::string data_s;
		int timeout_s;
		bool sended_s,
			 confirmed_s;
	};
	unsigned int number_of_datagrams_;
	int buffer_length_, 
		sender_number_, 
		frame_number_, 
		number_of_next_, 
		confirmed_datagrams_,
		timeout_,
		max_possible_send_,
		number_of_sended_;
	bool able_to_transmit_,
		reciever_know_number_;
	DatagramData *datagram_list_;
};

#endif // !_DatagramControlerH_
