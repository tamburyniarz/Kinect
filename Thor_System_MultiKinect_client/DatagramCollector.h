#pragma once
#ifndef _DatagramCollectorH_
#define _DatagramCollectorH_

#include <iostream>
#include <string>
#include <stdio.h>
#include <sstream>

struct RecievedData
{
	std::string data_s;
	bool is_recieved_s;
	RecievedData() { is_recieved_s = false; }
};

class DatagramCollector
{
public:
	DatagramCollector(int number_of_datagrams, int number_of_frame,int sender_number);
	~DatagramCollector();
	void InstertDatagram(int number_of_datagram, char *buffer, int buffer_length);
	bool RecievedAllDatagrams();
	int NumberOfDatagrams();
	std::stringstream CreateStreamFromData();
private:
	unsigned int number_of_datagrams_;
	int number_of_frame_,
		sender_number_,
		number_of_recieved_datagrams_;
	RecievedData *datagram_list_;
};

#endif // !_DatagramCollectorH_