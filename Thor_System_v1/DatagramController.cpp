#define _CRT_SECURE_NO_WARNINGS

#include "DatagramController.h"


int DatagramController::RetNumDAta()
{
	return number_of_datagrams_;
}

int DatagramController::ReturnActualFrame()
{
	return frame_number_;
}

bool DatagramController::SendedTooMuch()
{
	if (number_of_sended_ < max_possible_send_)
	{
		return false;
	}
	return true;
}

bool DatagramController::RecieverKnowDataNum()
{
	return reciever_know_number_;
}

bool DatagramController::AbleToTransmit()
{
	return able_to_transmit_;
}

void DatagramController::SetNumOfDataConfirmation()
{
	reciever_know_number_ = true;
	return;
}

void DatagramController::SetTransmitionPermission()
{
	able_to_transmit_ = true;
	return;
}

void DatagramController::ConfimDatagram(int number_of_datagram)
{
	if (number_of_datagram < number_of_datagrams_)
	{
		if (datagram_list_[number_of_datagram].confirmed_s == false)
		{
			datagram_list_[number_of_datagram].confirmed_s = true;
			++confirmed_datagrams_;
			--number_of_sended_;
		}
		return;
	}
	printf("Wrong number of datagram! \n");
	return;
}

void DatagramController::CheckTimeout()
{;
	for (int i = 0; i < number_of_datagrams_; ++i)
	{
		if (datagram_list_[i].sended_s && !datagram_list_[i].confirmed_s && datagram_list_[i].timeout_s>=timeout_)
		{
			//printf("flag changed for : %d \n", i);
			datagram_list_[i].timeout_s = 0;
			datagram_list_[i].sended_s = false;
			--number_of_sended_;
		}
	}
	return;
}

void DatagramController::IncreaseWaitTime()
{
	for (int i = 0; i < number_of_datagrams_; ++i)
	{
		if (datagram_list_[i].sended_s && !datagram_list_[i].confirmed_s)
		{
			//printf("increased for %d \n",i);
			++(datagram_list_[i].timeout_s);
		}
	}
	return;
}

void DatagramController::ReturnFrameDatgram(int number_of_demand_datagram, char *buffer, int buffer_length)
{
	if (buffer_length < buffer_length_)
	{
		printf("Bufor is too small for data!");
		return;
	}
	datagram_list_[number_of_demand_datagram].data_s.copy(buffer, buffer_length_);
	datagram_list_[number_of_demand_datagram].sended_s = true;
	++number_of_sended_;
	return;
}

void DatagramController::ReturnFrameInfoDatagram(char *buffer, int buffer_length)
{
	if (buffer_length < buffer_length_)
	{
		printf("Bufor is too small for data!");
		return;
	}
	buffer[0] = 'N';
	buffer[1] = 'U';
	buffer[2] = 'M';
	buffer[3] = ':';
	buffer[4] = (number_of_datagrams_ & 0x03FC) >> 2;
	buffer[5] = static_cast<char>(((number_of_datagrams_ & 0x0003) << 6) + ((frame_number_ & 0x000F) << 2) + (sender_number_ & 0x0003));

	return;
}

int DatagramController::ReturnNextDatagramNumber()
{
	if (confirmed_datagrams_ == number_of_datagrams_)
	{
		//printf("All datagram allready sended and confirmed");
		return 0xFFFF;
	}
	if (number_of_next_ < number_of_datagrams_)
	{
		return number_of_next_++;
	}
	else
	{
		//printf("Szukam zgubionych \n");
		for(int i = 0; i<number_of_datagrams_;++i)
		{
			if (!(datagram_list_[i].sended_s))
				return i;
		}
	}
	return 0xFFFF;
}

DatagramController::DatagramController(std::stringstream &coded_cloud, int buffer_length, int sender_number, int frame_number, int timeout, int max_possible_send) :
	number_of_next_(0),
	buffer_length_(buffer_length),
	sender_number_(sender_number),
	frame_number_(frame_number),
	confirmed_datagrams_(0),
	timeout_(timeout),
	max_possible_send_(max_possible_send),
	number_of_sended_(0),
	reciever_know_number_(false), 
	able_to_transmit_(false)
{
	std::string coded_cloud_string = coded_cloud.str();
	// calculating datagram number
	number_of_datagrams_ = static_cast<int>(ceil(static_cast<float>(coded_cloud_string.size()) / static_cast<float>(buffer_length-2)));

	printf("num of symbols: %d \n", coded_cloud_string.size());
	printf("num of datagrams: %d \n", number_of_datagrams_);
	
	//FILE *file_ptr;
	//file_ptr = fopen("test.txt", "wb");

	//fwrite(coded_cloud_string.c_str(), sizeof(char), coded_cloud_string.size(), file_ptr);

	//fclose(file_ptr);

	// creating structure for data
	datagram_list_ = new DatagramData[number_of_datagrams_];

	int data_size = buffer_length - 2,
		part_od_second = ((frame_number & 0x000F) << 2) + (sender_number & 0x0003);
	
	//header 
	char first = '\0',
		second = '\0';
	
	//Spliting data in datagrams
	for (int i = 0; i < number_of_datagrams_; ++i)
	{
		// Creating header
		first = static_cast<char>((i & 0x03FC) >> 2);
		second = static_cast<char>(((i & 0x0003) << 6) + part_od_second);
		datagram_list_[i].data_s += first;
		datagram_list_[i].data_s += second;
		std::string temp;
		if (i == (number_of_datagrams_ - 1))
		{
			temp = coded_cloud_string.substr((i) * data_size);
		}
		else
		{
			temp = coded_cloud_string.substr((i) * data_size, data_size);;
		}
		for (int j = 0; j < temp.size(); ++j)
			datagram_list_[i].data_s += temp[j];
		datagram_list_[i].timeout_s = 0;
		datagram_list_[i].sended_s = false;
		datagram_list_[i].confirmed_s = false;
	}
}

DatagramController::~DatagramController()
{
	delete[] datagram_list_;
}