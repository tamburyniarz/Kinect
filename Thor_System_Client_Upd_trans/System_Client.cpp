#ifdef _DEBUG
#define _SCL_SECURE_NO_WARNINGS
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "System_Client.h"


INT_PTR CALLBACK ThorClientSystem::xWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_MOUSEWHEEL:
		if (GET_KEYSTATE_WPARAM(wParam) == MK_RBUTTON)
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			{
				viewer_distance_ += SCROLLING_VELOCITY;
			}
			else
			{
				viewer_distance_ -= SCROLLING_VELOCITY;
			}
		}
		break;
	case WM_RBUTTONUP:
		right_was_clicked_ = false;
		break;
	case WM_RBUTTONDOWN:
		prev_mouse_y_ = HIWORD(lParam);
		right_was_clicked_ = true;
		break;
	case WM_LBUTTONUP:
		left_was_clicked_ = false;
		break;
	case WM_LBUTTONDOWN:
		prev_mouse_x_ = LOWORD(lParam);
		left_was_clicked_ = true;
		break;
	case WM_MOUSEMOVE:
		if (left_was_clicked_)
		{
			short len_x = abs(LOWORD(lParam)) - abs(prev_mouse_x_);
			rotation_y_ -= Y_ROTATION_VELOCITY*ceil(static_cast<double>(len_x) / 10.0);
			prev_mouse_x_ = LOWORD(lParam);
		}
		if (right_was_clicked_)
		{
			short len_y = abs(HIWORD(lParam)) - abs(prev_mouse_y_);
			rotation_x_ += X_ROTATION_VELOCITY*ceil(static_cast<double>(len_y) / 10.0);
			prev_mouse_y_ = HIWORD(lParam);
		}
		break;
	case WM_MOUSELEAVE:
		left_was_clicked_ = false;
		right_was_clicked_ = false;
		break;
	case WM_SIZE:
		xResizeGLScene(LOWORD(lParam), HIWORD(lParam));
		return xDrawGLScene();
	case WM_PAINT:
		return xDrawGLScene();
	}
	return cWindow::xWindowProc(hDlg, message, wParam, lParam);
}

unsigned char* ThorClientSystem::ReadBmpFromFile(char* szFileName, int &riWidth, int &riHeight)
{
	BITMAPFILEHEADER     bfh;
	BITMAPINFOHEADER     bih;

	int                i, j, h, v, lev, l, ls;
	unsigned char*     buff = NULL;

	unsigned char* p_palette = NULL;
	unsigned short n_colors = 0;

	unsigned char* pRGBBuffer = nullptr;

	FILE* hfile = fopen(szFileName, "rb");

	if (hfile != NULL)
	{
		fread(&bfh, sizeof(bfh), 1, hfile);
		if (!(bfh.bfType != 0x4d42 || (bfh.bfOffBits < (sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)))))
		{
			fread(&bih, sizeof(bih), 1, hfile);
			v = bih.biWidth;
			h = bih.biHeight;
			lev = bih.biBitCount;

			riWidth = v;
			riHeight = h;
			pRGBBuffer = new unsigned char[riWidth*riHeight * 3]; //Zaalokowanie odpowiedniego buffora obrazu

																  //Za³aduj Palete barw jesli jest
			if ((lev == 1) || (lev == 4) || (lev == 8))
			{
				n_colors = 1 << lev;
				p_palette = new unsigned char[4 * n_colors];
				fread(p_palette, 4 * n_colors, 1, hfile);
			}

			fseek(hfile, bfh.bfOffBits, SEEK_SET);

			buff = new unsigned char[v * 4];

			switch (lev)
			{
			case 1:
				//Nie obs³ugiwane
				break;
			case 4:
				//nie Obs³ugiwane
				break;
			case 8: //Skala szaroœci
				ls = (v + 3) & 0xFFFFFFFC;
				for (j = (h - 1); j >= 0; j--)
				{
					fread(buff, ls, 1, hfile);
					for (i = 0, l = 0; i<v; i++)
					{
						pRGBBuffer[((j*riWidth) + i) * 3 + 2] = p_palette[(buff[i] << 2) + 2];//R
						pRGBBuffer[((j*riWidth) + i) * 3 + 1] = p_palette[(buff[i] << 2) + 1];//G
						pRGBBuffer[((j*riWidth) + i) * 3 + 0] = p_palette[(buff[i] << 2) + 0];//B
					}
				};
				break;
			case 24:
				//bitmapa RGB
				ls = (v * 3 + 3) & 0xFFFFFFFC;
				for (j = (h - 1); j >= 0; j--)
				{
					//x_fread(hfile,buff,ls);
					fread(buff, ls, 1, hfile);
					for (i = 0, l = 0; i<v; i++, l += 3)
					{
						pRGBBuffer[((j*riWidth) + i) * 3 + 0] = buff[l + 0];
						pRGBBuffer[((j*riWidth) + i) * 3 + 1] = buff[l + 1];
						pRGBBuffer[((j*riWidth) + i) * 3 + 2] = buff[l + 2];
					};
				};
				break;
			case 32:
				// RGBA bitmap 
				for (j = (h - 1); j >= 0; j--)
				{
					fread(buff, v * 4, 1, hfile);
					for (i = 0, l = 0; i<v; i++, l += 4)
					{
						pRGBBuffer[((j*riWidth) + i) * 3 + 0] = buff[l + 0];
						pRGBBuffer[((j*riWidth) + i) * 3 + 1] = buff[l + 1];
						pRGBBuffer[((j*riWidth) + i) * 3 + 2] = buff[l + 2];
					}
				};
				break;
			};
			delete buff;
			if (p_palette) delete p_palette;

		}
	}
	return pRGBBuffer;
}

bool ThorClientSystem::xLoadTexture()
{
	bool Status = FALSE;
	unsigned char *pp_logo = nullptr;
	int logo_width = 0,
		logo_height = 0;
	if(pp_logo = ReadBmpFromFile("pp_logo.bmp", logo_width, logo_height))
	{
		Status = TRUE;

		glGenTextures(1, &texture[0]);
		glBindTexture(GL_TEXTURE_2D, texture[0]);

		// Generate The Texture
		glTexImage2D(GL_TEXTURE_2D, 0, 3, logo_width, logo_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pp_logo);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
	}
	if (pp_logo)                            // If Texture Exists
	{
		delete[] pp_logo;
	}

	return Status;
}

void ThorClientSystem::RunSystem()
{
	std::thread reciever_thread(RecieverThreadStarter, this);
	std::thread sender_thread(SenderThreadStarter, this);
	std::thread decoder_thread(DecoderThreadStarter, this);

	reciever_thread.detach();
	sender_thread.detach();
	decoder_thread.detach();
}

bool  ThorClientSystem::xInitGLScene()
{
	cSaveRestoreRC c(m_hDC, m_hRC);

	if (!xLoadTexture())                          // Jump To Texture Loading Routine ( NEW )
	{
		return FALSE;                           // If Texture Didn't Load Return FALSE ( NEW )
	}

	glEnable(GL_TEXTURE_2D);                        // Enable Texture Mapping ( NEW )
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);							                  // ustawienie bufora g³ebi
	glEnable(GL_DEPTH_TEST);					                  // w³¹czenie testowania g³êbi
	glDepthFunc(GL_LEQUAL);					                      // ustawienie typu testowania
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glPointSize(3.0);

	return TRUE;
}

bool ThorClientSystem::xDrawGLScene()
{
	cSaveRestoreRC c(m_hDC, m_hRC);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);           // czyœæ ekran
	glLoadIdentity();                                             // zresetuj macierz modeli

	where_was_gondor_.lock();
	if (next_recieved_cloud_.new_frame_s)
	{
		actual_displaying_frame = next_recieved_cloud_.decompressed_cloud_s;
		next_recieved_cloud_.new_frame_s = false;
	}
	where_was_gondor_.unlock();
	glTranslatef(0.0f, 0.0f, viewer_distance_);
	glColor3f(1.0, 1.0,1.0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3d(-(0.95), (0.95), -1.0 - viewer_distance_);	//upper left
	glTexCoord2f(1.0f, 0.0f);
	glVertex3d(-(0.70), (0.95), -1.0 - viewer_distance_);	// upper right
	glTexCoord2f(1.0f, 1.0f);
	glVertex3d(-(0.70), (0.70), -1.0 - viewer_distance_);	// down right
	glTexCoord2f(0.0f, 1.0f);
	glVertex3d(-(0.95), (0.70), -1.0 - viewer_distance_);	// down left
	glEnd();

                            // Porusz 1.5 w lewo i 6.0 wg³¹b

	glRotatef(170.0 + rotation_y_, 0.0f, 1.0f, 0.0f);
	glRotatef( rotation_x_ , 1.0f, 0.0f, .0f);



	if (!(actual_displaying_frame == nullptr))
	{
		glBegin(GL_POINTS);

		for (size_t i = 0; i < actual_displaying_frame->points.size(); ++i)
		{
			glColor4f(actual_displaying_frame->points[i].r / 255.0, actual_displaying_frame->points[i].g / 255.0, actual_displaying_frame->points[i].b / 255.0, actual_displaying_frame->points[i].a / 255.0);
			glVertex3f(actual_displaying_frame->points[i].x, actual_displaying_frame->points[i].y, actual_displaying_frame->points[i].z);
		}

		glEnd();
	}

	//cOpenGLWindow::xDrawGLScene();

	SwapBuffers(m_hDC);
	//rtri += 0.05f;
	return TRUE;         // Wszytko posz³o ok
}

void ThorClientSystem::ClientReciever()
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
	server.sin_port = htons(RECIEVING_PORT);

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
							if(DEBUG_FLAG)
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

void ThorClientSystem::ClientSender()
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

void ThorClientSystem::CreateMessageToSend(char *buffer, ThorClientSystem::ClientReplyMessage client_reciever_message)
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

ThorClientSystem::ClientReplyMessage ThorClientSystem::DatagramRecognition(char *buffer)
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

void ThorClientSystem::CloudDecoder()
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
	DatagramCollector *actual_decoding_frame = nullptr;
	
	while (1)
	{
		//cheeking if recieved new cloud
		you_shall_not_pass_.lock();
		if (next_recieved_frame_ != nullptr)
		{
			actual_decoding_frame = next_recieved_frame_;
			next_recieved_frame_ = nullptr;
			if(DEBUG_FLAG)
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
			if(DEBUG_FLAG)
				printf("przekazano kolejno ramke! \n");

			//clean the mess
			delete actual_decoding_frame;
			actual_decoding_frame = nullptr;
		}
	}

	delete PointCloudDecoder;
}

ThorClientSystem::ThorClientSystem(int iResourceID, char* sWindowName) : cOpenGLWindow(iResourceID, sWindowName)
{
	next_recieved_frame_ = nullptr;
	rotation_x_ = 0.0f;
	rotation_y_ = 0.0f;
	viewer_distance_ = 0.0;
	left_was_clicked_ = false;
	right_was_clicked_ = false;
	xInitGLScene();
}

ThorClientSystem::~ThorClientSystem()
{
	if (next_recieved_frame_ != nullptr)
		delete next_recieved_frame_;
	prev_mouse_y_ = 0;
	prev_mouse_y_ = 0;
	xDestroyGLScene();
}