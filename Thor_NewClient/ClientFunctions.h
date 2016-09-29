#pragma once
#ifndef _ClientFunctionsH_
#define _ClientFunctionsH_

#include "DatagramCollector.h"
#include <mutex>
#include <queue>
#include <thread>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/compression/octree_pointcloud_compression.h>
#include <winsock2.h>
//Winsock Library
#pragma comment(lib,"ws2_32.lib")

#define SERVER "172.20.128.13"		//ip address od server
#define BUFLEN 10					//Max length of buffer
#define SENDER_BUF_LEN 1000
#define RECBUFLEN 1024				// Length of recieving bufer
#define SENDING_PORT 8889			//The port o which to send data
#define RECIEVING_PORT 8888			//The port on whichto listen for incoming data
#define DEBUG_FLAG 1

enum reply_data_message_type
{
	RECIEVER_READY = 0xFF0F,
	RECIEVED_NUMBER_OF_DATAGRAM = 0x0FF0,
	ALL_DATTA_RECIEVED = 0xFFFF,
	DATA_RECIEVED_DAMANGED = 0xF00F,
	DATAGRAM_RECIEVED = 0x8001,
	UNKNOWN_DATAGRAM = 0x0000
};
struct ClientReplyMessage
{
	reply_data_message_type mes_type_s;
	int datagram_number_s,
		frame_number_s,
		sender_s;
};

void CreateMessageToSend(char *buffer, ClientReplyMessage client_reciever_message);

class MagicTransmisionClient
{
public:
	void RunClient();
private:
	std::mutex one_to_rule_them_all;
	std::queue<ClientReplyMessage> message_queue;

	ClientReplyMessage DatagramRecognition(char *buffer);
	
	void ClientSender();
	void ClientReciever();

	static void ThreadStarter(MagicTransmisionClient *my_client) { return my_client->ClientSender(); }
};

#endif // !_ClientFunctionsH_