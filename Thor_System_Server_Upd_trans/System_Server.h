#pragma once
#ifndef _ServerSystemH_
#define _ServerSystemH_


#include "DatagramController.h"
#include "DatagramCollector.h"
#include <mutex>
#include <queue>
#include <thread>

#include <pcl/io/pcd_io.h>
#include <pcl/compression/octree_pointcloud_compression.h>
#include "kinect2_grabber.h"
#include <winsock2.h>

#define CLIENT "172.20.2.208"		//ip address of udp client
#define BUFLEN 1000					//Max length of buffer
#define RECBUFLEN 14				// Length of reeceiveing bufer
#define SENDING_PORT 8890			//The port on which to send data
#define RECIEVING_PORT 8891			//The port on which to listen for incoming data
#define NUM_OF_SENDED 5			//Number of possible datagram sended without confirmation
#define CONF_TIMEOUT 3000
#define RR_TIMEOUT 2
#define DEBUG_FLAG 0
#define RECIEVING_SOCKET_TIMEOUT 1

#define COMPRESSION_PROFILE pcl::io::LOW_RES_ONLINE_COMPRESSION_WITH_COLOR
#define SHOW_COMPR_STATS false

class ThorServerSystem
{
public:

	ThorServerSystem();
	~ThorServerSystem();

	void RunSystem();

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
		unsigned int datagram_number_s;
		int frame_number_s;
	};

	struct NextCloudBuffer
	{
		pcl::PointCloud<pcl::PointXYZ> cloud_to_compress_s;
		bool new_frame_s = false;
	};

	mutable std::mutex you_shall_not_pass_;									//mutex for communication between coder and sender
	std::stringstream *next_coded_frame_;									//datagrams containig codded frames

private:
	mutable std::mutex one_to_rule_them_all_;								//mutex for servver sender and reciever
	std::queue<RecievedMessage> message_queue_;								//queue for communication between sender and reciever

	char client_ip_[16];


	void LoadIPFromFile(char * file_name = "client_IP.txt");
	void GrabFrames();
	void ServerSender();
	void ServerReciever();
	RecievedMessage TranstaleRecievedMessage(char *buffer);

	static void GraberThreadStarter(ThorServerSystem *my_server) { return my_server->GrabFrames(); }
	static void SenderThreadStarter(ThorServerSystem *my_server) { return my_server->ServerSender(); }
	static void RecieverThreadStarter(ThorServerSystem *my_server) { return my_server->ServerReciever(); }
};

#endif // !_ServerSystemH_