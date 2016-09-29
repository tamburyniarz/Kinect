#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "ClientFunctions.h"

void MagicTransmisionClient::RunClient()
{
	std::thread adiszynal(ThreadStarter,this);
	adiszynal.detach();
	ClientReciever();
	return;
}

void MagicTransmisionClient::ClientReciever()//std::queue<ClientReplyMessage> &message_queue, std::mutex &one_to_rule_them_all)
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen,
		server_respond_timeout=0,
		actual_recieving_frame=0;
	char rec_conf[RECBUFLEN];
	WSADATA wsa;
	bool server_responded = false;

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
	if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	if (DEBUG_FLAG)
		printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(RECIEVING_PORT);

	//Bind
	if (::bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	if (DEBUG_FLAG)
		puts("Bind done");

	int sock_timeout = 10;
	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&sock_timeout, sizeof(sock_timeout)) == SOCKET_ERROR)
	{
		closesocket(s);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	//initializing data collector
	DatagramCollector *recieved_data = nullptr;

	while (1)
	{
		if (!server_responded)
		{
			if (server_respond_timeout == 0)
			{
				ClientReplyMessage permission_message;
				permission_message.mes_type_s = RECIEVER_READY;
				permission_message.frame_number_s = actual_recieving_frame;
				permission_message.datagram_number_s = 0xFFFF;
				permission_message.sender_s = 1;

				if (DEBUG_FLAG)
					printf("message sended to sender! \n");

				one_to_rule_them_all.lock();
				message_queue.push(permission_message);
				one_to_rule_them_all.unlock();
				++server_respond_timeout;
			}
			else
			{
				server_respond_timeout = (++server_respond_timeout) % 255;
			}
		}

			
		int Received = recvfrom(s, rec_conf, RECBUFLEN, 0, (struct sockaddr *) &si_other, &slen);
		if (Received>0)
		{
			if (Received >= 2)
			{
				if (DEBUG_FLAG)
					printf("datagram recieved \n");
				ClientReplyMessage recieved_datagram = DatagramRecognition(rec_conf);
				switch (recieved_datagram.mes_type_s)
				{
				case RECIEVED_NUMBER_OF_DATAGRAM:
					if (recieved_data == nullptr)
					{
						if (recieved_datagram.frame_number_s == actual_recieving_frame)
						{
							recieved_data = new DatagramCollector(recieved_datagram.datagram_number_s, recieved_datagram.frame_number_s, recieved_datagram.sender_s);
							server_responded = true;
							server_respond_timeout = 0;
							one_to_rule_them_all.lock();
							message_queue.push(recieved_datagram);
							one_to_rule_them_all.unlock();

							if (DEBUG_FLAG)
								printf("again!!! \n");
						}
					}
					else
					{
						one_to_rule_them_all.lock();
						message_queue.push(recieved_datagram);
						one_to_rule_them_all.unlock();
					}
					break;
				case DATAGRAM_RECIEVED:
					if (recieved_data != nullptr && recieved_datagram.frame_number_s == actual_recieving_frame)
					{
						if (recieved_datagram.datagram_number_s >= recieved_data->NumberOfDatagrams())
							break;
						recieved_data->InstertDatagram(recieved_datagram.datagram_number_s, rec_conf, SENDER_BUF_LEN);
						one_to_rule_them_all.lock();
						message_queue.push(recieved_datagram);
						one_to_rule_them_all.unlock();
						if (DEBUG_FLAG)
							printf("recieved_data: \n");
						if (recieved_data->RecievedAllDatagrams())
						{
							recieved_datagram.mes_type_s = ALL_DATTA_RECIEVED;
							recieved_datagram.frame_number_s = actual_recieving_frame;
							recieved_datagram.datagram_number_s = 0xFFFF;
							one_to_rule_them_all.lock();
							message_queue.push(recieved_datagram);
							one_to_rule_them_all.unlock();
							printf("!\n");
							std::stringstream recievedcloud = recieved_data->CreateStreamFromData();
							std::string rerere = recievedcloud.str();
							//initializing all components needet to decoded and save recieved Point Cloud
							pcl::PointCloud<pcl::PointXYZRGBA> writeCloud;
							pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA>* PointCloudDecoder;
							PointCloudDecoder = new pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA>();
							// output pointcloud
							pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloudOut(new pcl::PointCloud<pcl::PointXYZRGBA>());

							PointCloudDecoder->decodePointCloud(recievedcloud, cloudOut);

							pcl::PointCloud<pcl::PointXYZRGBA> cloud = *cloudOut;
							pcl::io::savePCDFileASCII("test_pcd.pcd", cloud);

							delete recieved_data;
							recieved_data = nullptr;
							actual_recieving_frame = (actual_recieving_frame+1)%16;
							server_responded = false;
							if (DEBUG_FLAG)
								printf("recieved all data \n");
						}
						else if (recieved_data == nullptr)
						{
							recieved_datagram.mes_type_s = ALL_DATTA_RECIEVED;
							recieved_datagram.frame_number_s = actual_recieving_frame;
							recieved_datagram.datagram_number_s = 0xFFFF;
							one_to_rule_them_all.lock();
							message_queue.push(recieved_datagram);
							one_to_rule_them_all.unlock();
						}
						break;
					}
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

void MagicTransmisionClient::ClientSender()//std::queue<ClientReplyMessage> &message_queue, std::mutex &one_to_rule_them_all)
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other), rec_num_pack_counter = 0;
	char message[BUFLEN];
	WSADATA wsa;

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
	si_other.sin_addr.S_un.S_addr = inet_addr(SERVER);

	//start communication
	while (1)
	{
		//check recieved messages
		one_to_rule_them_all.lock();
		bool ocs = message_queue.empty();
		one_to_rule_them_all.unlock();

		if (!ocs)
		{
			memset(message, '\0', BUFLEN);
			one_to_rule_them_all.lock();
			ClientReplyMessage actual_message = message_queue.front();
			message_queue.pop();
			one_to_rule_them_all.unlock();
			CreateMessageToSend(message, actual_message);
			if (sendto(s, message, BUFLEN, 0, (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
			{
				printf("sendto() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			if (DEBUG_FLAG)
				printf("Message sended \n");
		}
		else
		{
			
		}
		//Sleep(1);
	}

	closesocket(s);
	WSACleanup();

	return;
}

void CreateMessageToSend(char *buffer, ClientReplyMessage client_reciever_message)
{
	//message type
	buffer[0] = static_cast<char>((client_reciever_message.mes_type_s & 0xFF00)>> 8);
	buffer[1] = static_cast<char>(client_reciever_message.mes_type_s & 0x00FF);
	buffer[2] = static_cast<char>((client_reciever_message.datagram_number_s & 0x03FC)>> 2);
	buffer[3] = (static_cast<char>(client_reciever_message.datagram_number_s & 0x0003) << 6) + (static_cast<char>(client_reciever_message.frame_number_s) << 2) + static_cast<char>(client_reciever_message.sender_s);
	return;
}

ClientReplyMessage MagicTransmisionClient::DatagramRecognition(char *buffer)
{
	ClientReplyMessage recieved_message;
	if (buffer[0] == 'N' && buffer[1] == 'U' && buffer[2] == 'M' && buffer[3] == ':')
	{
		recieved_message.mes_type_s = RECIEVED_NUMBER_OF_DATAGRAM;
		recieved_message.frame_number_s =  (((unsigned int)((unsigned char)buffer[5])) & 0x003C) >> 2;
		recieved_message.datagram_number_s = (((unsigned int)((unsigned char)buffer[4])) << 2) + ((((unsigned int)((unsigned char)buffer[5])) & 0x00C0) >> 6);
		recieved_message.sender_s = ((unsigned int)((unsigned char)buffer[5])) & 0x0003;
		return recieved_message;
	}
	recieved_message.frame_number_s = (((unsigned int)((unsigned char)buffer[1])) & 0x003C) >> 2;
	recieved_message.datagram_number_s = (((unsigned int)((unsigned char)buffer[0])) << 2) + ((((unsigned int)((unsigned char)buffer[1])) & 0x00C0) >> 6);
	recieved_message.sender_s = ((unsigned int)((unsigned char)buffer[1])) & 0x0003;
	recieved_message.mes_type_s = DATAGRAM_RECIEVED;
	if (DEBUG_FLAG)
		printf("Recieved data: %d , frame: %d \n", recieved_message.datagram_number_s, recieved_message.frame_number_s);
	return recieved_message;
}