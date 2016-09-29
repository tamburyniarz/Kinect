#ifdef _DEBUG
#define _SCL_SECURE_NO_WARNINGS
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "System_Server.h"

void ThorServerSystem::RunSystem()
{
	LoadIPFromFile();
	//std::thread graber_thread(GraberThreadStarter, this);
	std::thread reciever_thread(RecieverThreadStarter, this);
	std::thread sender_thread(SenderThreadStarter, this);
	//graber_thread.detach();
	reciever_thread.detach();
	sender_thread.detach();
	GrabFrames();
}

void ThorServerSystem::ServerReciever()
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
		WSACleanup();
		printf("Could not create socket : %d", WSAGetLastError());
	}
	if (DEBUG_FLAG)
		printf("Socket created.\n");

	//Prepare the sockaddr_in 
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(recieving_port_);

	//Bind
	if (::bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		closesocket(s);
		WSACleanup();
		exit(EXIT_FAILURE);
	}
	if (DEBUG_FLAG)
		puts("Bind done");

	//sets socket to stop blocking port after timeout
	//int sock_timeout = RECIEVING_SOCKET_TIMEOUT;
	//if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&sock_timeout, sizeof(sock_timeout)) == SOCKET_ERROR)
	//{
	//	printf("Socket timeout failed with error code : %d", WSAGetLastError());
	//	closesocket(s);
	//	WSACleanup();
	//	exit(EXIT_FAILURE);
	//}

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
					one_to_rule_them_all_.lock();
					message_queue_.push(actual_message);
					one_to_rule_them_all_.unlock();
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

void ThorServerSystem::ServerSender()
{
	struct sockaddr_in si_other;
	int s, slen = sizeof(si_other), rec_num_pack_counter = 0;
	char message[BUFLEN];
	WSADATA wsa;

	// Initializing datagrams container
	DatagramController *datagrams_service = nullptr;

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
		WSACleanup();
		printf("socket() failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//setup address structure
	memset((char *)&si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(sending_port_);
	si_other.sin_addr.S_un.S_addr = inet_addr(client_ip_);

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
					if(DEBUG_FLAG)
						printf("returned: %d \n", next_data_num);
					if (next_data_num != 0xFFFFFFFF)
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
		one_to_rule_them_all_.lock();
		if (!message_queue_.empty())
		{
			are_messages = message_queue_.empty();
			actual_message = message_queue_.front();
			message_queue_.pop();
		}
		one_to_rule_them_all_.unlock();

		if (!are_messages)
		{
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
					you_shall_not_pass_.lock();
					if (next_coded_frame_ == nullptr)
					{
						printf("NO frames coddded");
					}
					else
					{
						datagrams_service = new DatagramController(*next_coded_frame_, BUFLEN, 1, actual_message.frame_number_s, CONF_TIMEOUT, NUM_OF_SENDED);
						delete next_coded_frame_;
						next_coded_frame_ = nullptr;
						datagrams_service->SetTransmitionPermission();
					}
					you_shall_not_pass_.unlock();

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
		}
		if (datagrams_service != nullptr && datagrams_service->RecieverKnowDataNum())
		{
			datagrams_service->IncreaseWaitTime();
			datagrams_service->CheckTimeout();
		}
		Sleep(1);
	}

	closesocket(s);
	WSACleanup();

	return;
}

// Message translator
ThorServerSystem::RecievedMessage ThorServerSystem::TranstaleRecievedMessage(char *buffer)
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
		actual_message.datagram_number_s = 0xFFFFFFFF;
		actual_message.frame_number_s = (((unsigned int)((unsigned char)buffer[6])) & 0x00FC) >> 2;
		return actual_message;
	case ALL_DATA_RECIEVED:
		if (DEBUG_FLAG)
			printf("ALL_DATA_RECIEVED \n");
		actual_message.mes_type_s = ALL_DATA_RECIEVED;
		actual_message.datagram_number_s = 0xFFFFFFFF;
		actual_message.frame_number_s = (((unsigned int)((unsigned char)buffer[6])) & 0x00FC) >> 2;
		return actual_message;
	case RECIEVED_NUMBER_OF_DATAGRAMS:
		if (DEBUG_FLAG)
			printf("RECIEVED_NUMBER_OF_DATAGRAMS \n");
		actual_message.mes_type_s = RECIEVED_NUMBER_OF_DATAGRAMS;
		actual_message.datagram_number_s = 0xFFFFFFFF;
		actual_message.frame_number_s = (((unsigned int)((unsigned char)buffer[6])) & 0x00FC) >> 2;
		return actual_message;
	case DATA_RECIEVED_DAMAGED:
		if (DEBUG_FLAG)
			printf("DATA_RECIEVED_DAMAGED \n");
		actual_message.datagram_number_s = (((unsigned int)((unsigned char)buffer[2])) << 24) 
			+ (((unsigned int)((unsigned char)buffer[3])) << 16)
			+ (((unsigned int)((unsigned char)buffer[4])) << 8)
			+ ((unsigned int)((unsigned char)buffer[5]));
		actual_message.frame_number_s = (((unsigned int)((unsigned char)buffer[6])) & 0x00FC) >> 2;
		actual_message.mes_type_s = DATA_RECIEVED_DAMAGED;
		return actual_message;
	case DATAGRAM_RECIEVED:
		if (DEBUG_FLAG)
			printf("DATAGRAM_RECIEVED \n");
		actual_message.datagram_number_s = (((unsigned int)((unsigned char)buffer[2])) << 24)
			+ (((unsigned int)((unsigned char)buffer[3])) << 16)
			+ (((unsigned int)((unsigned char)buffer[4])) << 8)
			+ ((unsigned int)((unsigned char)buffer[5]));
		actual_message.frame_number_s = (((unsigned int)((unsigned char)buffer[6])) & 0x00FC) >> 2;
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

void ThorServerSystem::LoadIPFromFile(char * file_name)
{
	std::ifstream file(file_name);
	char prefix[20] = "";
	// Client ip
	file >> prefix;
	if (prefix[0] == '#')
	{
		std::cout << prefix << " ";
		file >> client_ip_;
		std::cout << client_ip_ << "\n";
	}
	// Sending port
	file >> prefix;
	if (prefix[0] == '#')
	{
		std::cout << prefix << " ";
		file >> sending_port_;
		std::cout << sending_port_ << "\n";
	}
	// Receiving port
	file >> prefix;
	if (prefix[0] == '#')
	{
		std::cout << prefix << " ";
		file >> recieving_port_;
		std::cout << recieving_port_ << "\n";
	}
	file.close();
}

//kinect frames grabber
void ThorServerSystem::GrabFrames()
{

	pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA>* PointCloudEncoder;
	bool showStatistics = DEBUG_FLAG;

	// for a full list of profiles see: /io/include/pcl/compression/compression_profiles.h
	pcl::io::compression_Profiles_e compressionProfile = COMPRESSION_PROFILE;

	const pcl::io::configurationProfile_t selectedProfile = pcl::io::compressionProfiles_[compressionProfile];

	// instantiate point cloud compression for encoding and decoding
	PointCloudEncoder = new pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA>(
		compressionProfile,
		showStatistics,
		selectedProfile.pointResolution,
		selectedProfile.octreeResolution, 
		selectedProfile.doVoxelGridDownSampling,
		selectedProfile.iFrameRate,
		selectedProfile.doColorEncoding,
		static_cast<unsigned char>(selectedProfile.colorBitResolution)
		);


	ThorServerSystem* thor = this;

	int counter = 0;
	// Point Cloud
	pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr cloud;

	// Retrieved Point Cloud Callback Function
	boost::mutex mutex;
	boost::function<void(const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr&)> function =
		[&cloud,&mutex,&thor,&PointCloudEncoder, &counter](const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr& ptr) {
		boost::mutex::scoped_lock lock(mutex);

		std::stringstream tererere;

		thor->you_shall_not_pass_.lock();
		if (thor->next_coded_frame_ == nullptr)
		{
			thor->next_coded_frame_ = new std::stringstream;

			// compress point cloud
			PointCloudEncoder->encodePointCloud(ptr, *(thor->next_coded_frame_));
			
		}
		else
		{
			//printf("nothing to do! ");
		}
		thor->you_shall_not_pass_.unlock();

		if (DEBUG_FLAG)
		{
			//printf("Grabbed next frame! \n");
		}
	};
	HANDLE g_myThreadEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

	// Kinect2Grabber
	boost::shared_ptr<pcl::Grabber> grabber = boost::make_shared<pcl::Kinect2Grabber>();

	// Register Callback Function
	boost::signals2::connection connection = grabber->registerCallback(function);

	// Start Grabber
	grabber->start();

	WaitForSingleObject(g_myThreadEvent, INFINITE);
	CloseHandle(g_myThreadEvent);
	// Stop Grabber
	grabber->stop();

	// Disconnect Callback Function
	if (connection.connected()) 
	{
		connection.disconnect();
	}
}

ThorServerSystem::ThorServerSystem()
{
	next_coded_frame_ = nullptr;
}

ThorServerSystem::~ThorServerSystem()
{
	delete next_coded_frame_;
}