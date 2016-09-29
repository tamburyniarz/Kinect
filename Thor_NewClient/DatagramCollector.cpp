#include "DatagramCollector.h"

int DatagramCollector::NumberOfDatagrams()
{
	return number_of_datagrams_;
}

std::stringstream DatagramCollector::CreateStreamFromData()
{
	std::stringstream rebuilded_stream;
	if (RecievedAllDatagrams())
	{
		for (int i = 0; i < number_of_datagrams_; ++i)
		{
			rebuilded_stream << datagram_list_[i].data_s;
		}
	}
	else
	{
		printf("Some datagram missing, unable to rebuild binary stream! \n");
	}
	return rebuilded_stream;
}

bool DatagramCollector::RecievedAllDatagrams()
{
	if (number_of_recieved_datagrams_ == number_of_datagrams_)
	{
		return true;
	}
	return false;
}

void DatagramCollector::InstertDatagram(int number_of_datagram, char *buffer, int buffer_length)
{
	if (datagram_list_[number_of_datagram].is_recieved_s)
	{
		//printf("Allready recieved!!! \n");
		return;
	}
	if (number_of_datagram >= number_of_datagrams_)
	{
		//printf("Unexpected datagram number! \n");
		return;
	}
	for (int i = 2; i < buffer_length;++i)
		datagram_list_[number_of_datagram].data_s += buffer[i];
	datagram_list_[number_of_datagram].is_recieved_s = true;
	++number_of_recieved_datagrams_;
	if (datagram_list_[number_of_datagram].data_s.size() != (buffer_length-2))
	{
		//printf("Magic cosmic cow had eaten som of symbols!! \n");
	}
	return;
}

DatagramCollector::DatagramCollector(int number_of_datagrams, int number_of_frame, int sender_number):
	number_of_datagrams_(number_of_datagrams), number_of_frame_(number_of_frame),
	sender_number_(sender_number), number_of_recieved_datagrams_(0)
{
	printf("number of datagrams: %d \n", number_of_datagrams_);
	datagram_list_ = new RecievedData[number_of_datagrams_];
}

DatagramCollector::~DatagramCollector()
{
	delete[] datagram_list_;
}