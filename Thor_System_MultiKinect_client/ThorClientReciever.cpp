#ifdef _DEBUG
#define _SCL_SECURE_NO_WARNINGS
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "ThorClientReciever.h"

void ThorClientReciever::RunReciever()
{
	std::thread reciever_thread(RecieverThreadStarter, this);
	std::thread sender_thread(SenderThreadStarter, this);
	std::thread decoder_thread(DecoderThreadStarter, this);

	reciever_thread.detach();
	sender_thread.detach();
	decoder_thread.detach();
}

void ThorClientReciever::CloudDecoder()
{
	// initializing decoder object
	pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA>* PointCloudDecoder;

	bool showStatistics = DEBUG_FLAG;

	// for a full list of profiles see: /io/include/pcl/compression/compression_profiles.h
	pcl::io::compression_Profiles_e compressionProfile = COMPRESSION_PROFILE;

	const pcl::io::configurationProfile_t selectedProfile = pcl::io::compressionProfiles_[compressionProfile];

	PointCloudDecoder = new pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA>(
		compressionProfile,
		showStatistics,
		selectedProfile.pointResolution,
		selectedProfile.octreeResolution,
		selectedProfile.doVoxelGridDownSampling,
		selectedProfile.iFrameRate,
		selectedProfile.doColorEncoding,
		static_cast<unsigned char>(selectedProfile.colorBitResolution)
	);

	bool is_frame_to_decompress = false;
	DatagramCollector* actual_decoding_frame = nullptr;

	while (1)
	{
		WaitForSingleObject(recieved_next_frame_event_, INFINITE);
		//cheeking if recieved new cloud
		you_shall_not_pass_.lock();
		if (next_recieved_frame_ != nullptr)
		{
			actual_decoding_frame = next_recieved_frame_;
			next_recieved_frame_ = nullptr;
			if (DEBUG_FLAG)
				printf("Otrzymano kolejny datagram!");
		}
		you_shall_not_pass_.unlock();

		//if new frame recieved start decompresing
		if (actual_decoding_frame != nullptr)
		{
			//rebuild stringstream 
			std::stringstream coded_point_cloud = actual_decoding_frame->CreateStreamFromData();
			std::string traaaa = coded_point_cloud.str();

			//decoding recieved frame
			pcl::PointCloud<pcl::PointXYZRGBA>::Ptr decompressed_cloud(new pcl::PointCloud<pcl::PointXYZRGBA>());
			PointCloudDecoder->decodePointCloud(coded_point_cloud, decompressed_cloud);


			//transfer decoded cloud to buffer
			where_was_gondor_.lock();
			next_recieved_cloud_.decompressed_cloud_s = decompressed_cloud;
			next_recieved_cloud_.new_frame_s = true;
			where_was_gondor_.unlock();
			if (DEBUG_FLAG)
				printf("przekazano kolejno ramke! \n");

			//clean the mess
			delete actual_decoding_frame;
			actual_decoding_frame = nullptr;
		}
	}

	delete PointCloudDecoder;
}

void ThorClientReciever::ClientReciever()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen,
		server_respond_timeout = 0,
		actual_recieving_frame = 0;
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
	server.sin_port = htons(recieving_port_);

	//Bind
	if (::bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	if (DEBUG_FLAG)
		puts("Bind done");

	int sock_timeout = RECIEVING_SOCKET_TIMEOUT;
	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&sock_timeout, sizeof(sock_timeout)) == SOCKET_ERROR)
	{
		closesocket(s);
		WSACleanup();
		exit(EXIT_FAILURE);
	}

	//initializing data collector
	DatagramCollector* recieved_data = nullptr;

	while (true)
	{
		if (!server_responded)
		{
			if (server_respond_timeout == 0)
			{
				ClientReplyMessage permission_message;
				permission_message.mes_type_s = RECIEVER_READY;
				permission_message.frame_number_s = actual_recieving_frame;
				permission_message.datagram_number_s = 0xFFFFFFFF;
				permission_message.sender_s = 1;

				if (DEBUG_FLAG)
					printf("message sended to sender! \n");

				one_to_rule_them_all_.lock();
				message_queue_.push(permission_message);
				one_to_rule_them_all_.unlock();
				++server_respond_timeout;
			}
			else
			{
				server_respond_timeout = (++server_respond_timeout) % 255;
			}
		}


		int Received = recvfrom(s, rec_conf, RECBUFLEN, 0, (struct sockaddr *) &si_other, &slen);
		if (Received > 0)
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
							one_to_rule_them_all_.lock();
							message_queue_.push(recieved_datagram);
							one_to_rule_them_all_.unlock();

							if (DEBUG_FLAG)
								printf("again!!! \n");
						}
					}
					else
					{
						one_to_rule_them_all_.lock();
						message_queue_.push(recieved_datagram);
						one_to_rule_them_all_.unlock();
					}
					break;
				case DATAGRAM_RECIEVED:
					if (recieved_data != nullptr && recieved_datagram.frame_number_s == actual_recieving_frame)
					{
						if (recieved_datagram.datagram_number_s >= recieved_data->NumberOfDatagrams())
							break;
						recieved_data->InstertDatagram(recieved_datagram.datagram_number_s, rec_conf, SEND_BUF_LEN);
						one_to_rule_them_all_.lock();
						message_queue_.push(recieved_datagram);
						one_to_rule_them_all_.unlock();
						if (DEBUG_FLAG)
							printf("recieved_data: \n");
						if (recieved_data->RecievedAllDatagrams())
						{
							//send message that all frame data recieved
							recieved_datagram.mes_type_s = ALL_DATTA_RECIEVED;
							recieved_datagram.frame_number_s = actual_recieving_frame;
							recieved_datagram.datagram_number_s = 0xFFFFFFFF;

							one_to_rule_them_all_.lock();
							message_queue_.push(recieved_datagram);
							one_to_rule_them_all_.unlock();
							if (DEBUG_FLAG)
								printf("recieved all frame data \n");

							//sending recieved frame to decoder
							you_shall_not_pass_.lock();
							if (next_recieved_frame_ != nullptr)
							{
								//deleting last frame if not taken
								delete next_recieved_frame_;
							}
							next_recieved_frame_ = recieved_data;
							you_shall_not_pass_.unlock();
							recieved_data = nullptr;
							SetEvent(recieved_next_frame_event_);


							actual_recieving_frame = (actual_recieving_frame + 1) % 64;
							server_responded = false;
							if (DEBUG_FLAG)
								printf("recieved all frame data \n");
						}
						else if (recieved_data == nullptr)
						{
							recieved_datagram.mes_type_s = ALL_DATTA_RECIEVED;
							recieved_datagram.frame_number_s = actual_recieving_frame;
							recieved_datagram.datagram_number_s = 0xFFFFFFFF;

							one_to_rule_them_all_.lock();
							message_queue_.push(recieved_datagram);
							one_to_rule_them_all_.unlock();
						}
						break;
					}
				}
			}
			//clear the buffer
			memset(rec_conf, '\0', BUFLEN);
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

void ThorClientReciever::ClientSender()
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
	si_other.sin_port = htons(sending_port_);
	si_other.sin_addr.S_un.S_addr = inet_addr(server_IP_);

	//start communication
	while (1)
	{
		//check recieved messages
		one_to_rule_them_all_.lock();
		bool ocs = message_queue_.empty();
		one_to_rule_them_all_.unlock();

		if (!ocs)
		{
			memset(message, '\0', BUFLEN);
			one_to_rule_them_all_.lock();
			ClientReplyMessage actual_message = message_queue_.front();
			message_queue_.pop();
			one_to_rule_them_all_.unlock();
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

void ThorClientReciever::CreateMessageToSend(char* buffer, ThorClientReciever::ClientReplyMessage client_reciever_message)
{
	//message type
	buffer[0] = static_cast<char>((client_reciever_message.mes_type_s & 0xFF00) >> 8);
	buffer[1] = static_cast<char>(client_reciever_message.mes_type_s & 0x00FF);
	buffer[2] = static_cast<char>((client_reciever_message.datagram_number_s & 0xFF000000) >> 24);
	buffer[3] = static_cast<char>((client_reciever_message.datagram_number_s & 0x00FF0000) >> 16);
	buffer[4] = static_cast<char>((client_reciever_message.datagram_number_s & 0x0000FF00) >> 8);
	buffer[5] = static_cast<char>((client_reciever_message.datagram_number_s & 0x000000FF));
	buffer[6] = (static_cast<char>(client_reciever_message.frame_number_s) << 2) + static_cast<char>(client_reciever_message.sender_s);
	return;
}

ThorClientReciever::ClientReplyMessage ThorClientReciever::DatagramRecognition(char* buffer)
{
	ClientReplyMessage recieved_message;
	if (buffer[0] == 'N' && buffer[1] == 'U' && buffer[2] == 'M' && buffer[3] == ':')
	{
		recieved_message.mes_type_s = RECIEVED_NUMBER_OF_DATAGRAM;
		recieved_message.frame_number_s = (((unsigned int)((unsigned char)buffer[8])) & 0x00FC) >> 2;
		recieved_message.datagram_number_s = (((unsigned int)((unsigned char)buffer[4])) << 24)
			+ (((unsigned int)((unsigned char)buffer[5])) << 16)
			+ (((unsigned int)((unsigned char)buffer[6])) << 8)
			+ ((unsigned int)((unsigned char)buffer[7]));
		recieved_message.sender_s = ((unsigned int)((unsigned char)buffer[8])) & 0x0003;
		return recieved_message;
	}
	recieved_message.frame_number_s = (((unsigned int)((unsigned char)buffer[4])) & 0x00FC) >> 2;
	recieved_message.datagram_number_s = (((unsigned int)((unsigned char)buffer[0])) << 24)
		+ (((unsigned int)((unsigned char)buffer[1])) << 16)
		+ (((unsigned int)((unsigned char)buffer[2])) << 8)
		+ ((unsigned int)((unsigned char)buffer[3]));
	recieved_message.sender_s = ((unsigned int)((unsigned char)buffer[4])) & 0x0003;
	recieved_message.mes_type_s = DATAGRAM_RECIEVED;
	if (DEBUG_FLAG)
		printf("Recieved data: %d , frame: %d \n", recieved_message.datagram_number_s, recieved_message.frame_number_s);
	return recieved_message;
}


ThorClientReciever::ThorClientReciever(int sending_port, int recieving_port, char* server_IP):
	sending_port_(sending_port),
	recieving_port_(recieving_port),
	server_IP_(server_IP)
{
	next_recieved_frame_ = nullptr;
	recieved_next_frame_event_ = CreateEvent(NULL, TRUE, FALSE, NULL);
}

ThorClientReciever::~ThorClientReciever()
{
	if (next_recieved_frame_ != nullptr)
	{
		delete next_recieved_frame_;
	}
	delete[] server_IP_;
	CloseHandle(recieved_next_frame_event_);
}
