#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "ServerFunctions.h"

void MagicTransmisionServer::RunServer()
{
	std::thread adiszynal(ThreadStarter, this);
	adiszynal.detach();
	ServerSender();
}

void MagicTransmisionServer::ServerReciever()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen;
	char rec_conf[RECBUFLEN];
	WSADATA wsa;

	slen = sizeof(si_other);

	//Initialise winsock
	if (DEBUG_FLAG)
		printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	if (DEBUG_FLAG)
		printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	if (DEBUG_FLAG)
		printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(RECIEVING_PORT);
	//fcntl
	//Bind
	if (::bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	if (DEBUG_FLAG)
		puts("Bind done");

	while (1)
	{
		int Received = recv(s, rec_conf, RECBUFLEN, 0);
		if (Received>0)
		{
			if (Received >= 2)
			{
				RecievedMessage actual_message = TranstaleRecievedMessage(rec_conf);
				if (actual_message.mes_type_s == UNKNOWN_MESSAGE)
				{
					continue;
				}
				else
				{
					one_to_rule_them_all.lock();
					message_queue.push(actual_message);
					one_to_rule_them_all.unlock();
				}
			}
		}
		else
		{
			Sleep(0);
		}
	}

	closesocket(s);
	WSACleanup();

	return;
}

void MagicTransmisionServer::ServerSender()
{
	std::string outputData;

	//Reading Point Cloud, compression and saving output data to string
	ReadPointCloud(outputData);

	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other), rec_num_pack_counter = 0;
	char message[BUFLEN];
	WSADATA wsa;

	// Initializing datagrams container
	DatagramController *datagrams_service = nullptr;

	//Initialise winsock
	if(DEBUG_FLAG)
		printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	if (DEBUG_FLAG)
		printf("Initialised.\n");

	//create socket
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
	{
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//setup address structure
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(SENDING_PORT);
	si_other.sin_addr.S_un.S_addr = inet_addr(CLIENT);

	//start communication
	while (1)
	{
		if (datagrams_service != nullptr)
		{
			if (datagrams_service->RecieverKnowDataNum())
			{
				//cheek if possible to send next datagram
				if (!datagrams_service->SendedTooMuch())
				{
					//datagrams_service->ShNumSEND();
					//Sending next taken datagram
					int next_data_num = datagrams_service->ReturnNextDatagramNumber();
					if (next_data_num != 0xFFFF)
					{
						memset(message, '\0', BUFLEN);
						datagrams_service->ReturnFrameDatgram(next_data_num, message, BUFLEN);
						if (sendto(s, message, BUFLEN, 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
						{
							printf("sendto() failed with error code : %d", WSAGetLastError());
							exit(EXIT_FAILURE);
						}
						if (DEBUG_FLAG)
							printf("Message sended \n");
					}
					
				}
			}
			else
			{
				if (rec_num_pack_counter == 0)
				{
					//retransmiting number of frames
					memset(message, '\0', BUFLEN);
					datagrams_service->ReturnFrameInfoDatagram(message, BUFLEN);
					if (sendto(s, message, BUFLEN, 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
					{
						printf("sendto() failed with error code : %d", WSAGetLastError());
						exit(EXIT_FAILURE);
					}
					++rec_num_pack_counter;
					if (DEBUG_FLAG)
						printf("Num of pack sended \n");
				}
				else
				{
					rec_num_pack_counter = (++rec_num_pack_counter) % RR_TIMEOUT;
				}
			}
		}
		else
		{
			Sleep(0);
		}

		//int number_of_messages=0;
		RecievedMessage actual_message;
		bool are_messages = true;
		//check recieved messages
		one_to_rule_them_all.lock();
		if (!message_queue.empty())
		{
			are_messages = message_queue.empty();
			actual_message = message_queue.front();
			message_queue.pop();
		}
		one_to_rule_them_all.unlock();

		if (!are_messages)
		{
			//number_of_messages = temporary_queue.size();
			//for (int i = 0; i < number_of_messages; ++i)
			//{
				switch (actual_message.mes_type_s)
				{
				case RECIEVER_READY:
					if (datagrams_service != nullptr)
					{
						if (datagrams_service->AbleToTransmit() && actual_message.frame_number_s != datagrams_service->ReturnActualFrame())
						{
							delete datagrams_service;
							datagrams_service = nullptr;
						}
					}
					else
					{
						datagrams_service = new DatagramController(outputData, BUFLEN, 1, actual_message.frame_number_s, CONF_TIMEOUT, NUM_OF_SENDED);
						datagrams_service->SetTransmitionPermission();
					}
					break;
				case ALL_DATA_RECIEVED:
					if (datagrams_service != nullptr && actual_message.frame_number_s == datagrams_service->ReturnActualFrame())
					{
						delete datagrams_service;
						datagrams_service = nullptr;
						
					}
					break;
				case RECIEVED_NUMBER_OF_DATAGRAMS:
					if (datagrams_service != nullptr && actual_message.frame_number_s == datagrams_service->ReturnActualFrame())
					{
						datagrams_service->SetNumOfDataConfirmation();
					}
					break;
				case  DATA_RECIEVED_DAMAGED:
					// case of dmg 
					break;
				case DATAGRAM_RECIEVED:
					if (datagrams_service != nullptr &&  actual_message.frame_number_s == datagrams_service->ReturnActualFrame())
					{
						if (DEBUG_FLAG)
							printf("confirmed recieving od datagram : %d \n", actual_message.datagram_number_s);
						datagrams_service->ConfimDatagram(actual_message.datagram_number_s);
					}
					break;
				default:
					if (DEBUG_FLAG)
						printf("takie cus: %d   data:   %d    frame: %d", actual_message.mes_type_s, actual_message.datagram_number_s, actual_message.frame_number_s);
				}
				//temporary_queue.pop();
				if (DEBUG_FLAG)
					printf("Message translated \n");
			//}
		}
		if (datagrams_service != nullptr && datagrams_service->RecieverKnowDataNum())
		{
			datagrams_service->IncreaseWaitTime();
			datagrams_service->CheckTimeout();
		}
	}

	closesocket(s);
	WSACleanup();

	return;
}


void MagicTransmisionServer::ReadPointCloud(std::string &outputData)
{
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZRGBA>);
	//frame_after_decopresion
	if (pcl::io::loadPCDFile<pcl::PointXYZRGBA>("original_frame.pcd", *cloud) == -1) //* load the file
	{
		PCL_ERROR("Couldn't read file common.pcd \n");
		getchar();
		return;
	}

	bool showStatistics = true;
	//Instances for coder 
	pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA>* PointCloudEncoder;

	// for a full list of profiles see: /io/include/pcl/compression/compression_profiles.h
	pcl::io::compression_Profiles_e compressionProfile = pcl::io::LOW_RES_ONLINE_COMPRESSION_WITH_COLOR;

	// instantiate point cloud compression for encoding and decoding
	PointCloudEncoder = new pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA>(compressionProfile, showStatistics);

	// stringstream to store compressed point cloud
	std::stringstream compressedData;

	// compress point cloud
	PointCloudEncoder->encodePointCloud(cloud, compressedData);

	outputData = compressedData.str();
}

// Message translator
RecievedMessage MagicTransmisionServer::TranstaleRecievedMessage(char *buffer)
{
	RecievedMessage actual_message;
	//unsigned char tmp = (unsigned char)buffer[0];
	int recieved_message = (((unsigned int)((unsigned char)buffer[0])) << 8) + ((unsigned int)((unsigned char)buffer[1]));
	switch (recieved_message)
	{
	case RECIEVER_READY:
		if (DEBUG_FLAG)
			printf("RECIEVER_READY \n");
		actual_message.mes_type_s = RECIEVER_READY;
		actual_message.datagram_number_s = 0xFFFF;
		actual_message.frame_number_s = (((unsigned int)((unsigned char)buffer[3])) & 0x003C) >> 2;
		return actual_message;
	case ALL_DATA_RECIEVED:
		if (DEBUG_FLAG)
			printf("ALL_DATA_RECIEVED \n");
		actual_message.mes_type_s = ALL_DATA_RECIEVED;
		actual_message.datagram_number_s = 0xFFFF;
		actual_message.frame_number_s = (((unsigned int)((unsigned char)buffer[3])) & 0x003C) >>2;
		return actual_message;
	case RECIEVED_NUMBER_OF_DATAGRAMS:
		if (DEBUG_FLAG)
			printf("RECIEVED_NUMBER_OF_DATAGRAMS \n");
		actual_message.mes_type_s = RECIEVED_NUMBER_OF_DATAGRAMS;
		actual_message.datagram_number_s = 0xFFFF;
		actual_message.frame_number_s = (((unsigned int)((unsigned char)buffer[3])) & 0x003C) >> 2;
		return actual_message;
	case DATA_RECIEVED_DAMAGED:
		if (DEBUG_FLAG)
			printf("DATA_RECIEVED_DAMAGED \n");
		actual_message.datagram_number_s = (((unsigned int)((unsigned char)buffer[2])) << 2) + ((((unsigned int)((unsigned char)buffer[3])) & 0x00C0) >> 6);
		actual_message.frame_number_s = (((unsigned int)((unsigned char)buffer[3])) & 0x003C) >> 2;
		actual_message.mes_type_s = DATA_RECIEVED_DAMAGED;
		return actual_message;
	case DATAGRAM_RECIEVED:
		if (DEBUG_FLAG)
			printf("DATAGRAM_RECIEVED \n");
		actual_message.datagram_number_s = (((unsigned int)((unsigned char)buffer[2])) << 2) + ((((unsigned int)((unsigned char)buffer[3])) & 0x00C0) >> 6);
		actual_message.frame_number_s = (((unsigned int)((unsigned char)buffer[3])) & 0x003C) >> 2;
		actual_message.mes_type_s = DATAGRAM_RECIEVED;
		return actual_message;
	}
	if (DEBUG_FLAG)
		printf("UNKNOWN_MESSAGE \n");
	actual_message.frame_number_s = 0;
	actual_message.datagram_number_s = 0;
	actual_message.mes_type_s = UNKNOWN_MESSAGE;
	return actual_message;
}