#pragma once
#ifndef _ServerFunctionsH_
#define _ServerFunctionsH_

#include "DatagramController.h"
#include <mutex>
#include <queue>
#include <thread>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/compression/octree_pointcloud_compression.h>
#include <winsock2.h>
//Winsock Library
#pragma comment(lib,"ws2_32.lib")



#define CLIENT "192.168.1.76"		//ip address of udp client
#define BUFLEN 245					//Max length of buffer
#define RECBUFLEN 256				// Length of reeceiveing bufer
#define SENDING_PORT 8888			//The port on which to send data
#define RECIEVING_PORT 8889			//The port on which to listen for incoming data
#define NUM_OF_SENDED 10			//Number of possible datagram sended without confirmation
#define CONF_TIMEOUT 3000
#define RR_TIMEOUT 255000
#define DEBUG_FLAG 0

// Contains all possible messages
enum rec_data_message_type
{
	RECIEVER_READY = 0xFF0F,
	RECIEVED_NUMBER_OF_DATAGRAMS = 0x0FF0,
	ALL_DATA_RECIEVED = 0xFFFF,
	DATA_RECIEVED_DAMAGED = 0xF00F,
	DATAGRAM_RECIEVED = 0x8001,
	UNKNOWN_MESSAGE = 0x0000
};

struct RecievedMessage
{
	rec_data_message_type mes_type_s;
	int datagram_number_s,
		frame_number_s;
};

class MagicTransmisionServer 
{
public:
	void RunServer();
private:
	std::mutex one_to_rule_them_all;
	std::queue<RecievedMessage> message_queue;

	RecievedMessage TranstaleRecievedMessage(char *buffer);
	void ReadPointCloud(std::string &outputData);
	void ServerSender();
	void ServerReciever();

	static void ThreadStarter(MagicTransmisionServer *my_server) { return my_server->ServerReciever(); }
};

#endif // !_ServerFunctionsH_
