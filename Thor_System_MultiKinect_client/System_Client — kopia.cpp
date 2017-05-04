#ifdef _DEBUG
#define _SCL_SECURE_NO_WARNINGS
#endif

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include "System_Client.h"


INT_PTR CALLBACK ThorClientSystem::xWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_UP:
			viewer_distance_ += SCROLLING_VELOCITY;
			break;
		case VK_DOWN:
			viewer_distance_ -= SCROLLING_VELOCITY;
			break;
		case VK_LEFT:
			x_axis_pos -= SCROLLING_VELOCITY;
			break;
		case VK_RIGHT:
			x_axis_pos += SCROLLING_VELOCITY;
			break;
		case VK_SPACE:
			if (actual_displaying_frame_1_ != nullptr)
				std::cout << "Cloud 1 size: " << actual_displaying_frame_1_->size() << "\n";
			else
				std::cout << "Cloud 1 size: NULL\n";
			if (actual_displaying_frame_2_ != nullptr)
				std::cout << "Cloud 2 size: " << actual_displaying_frame_2_->size() << "\n";
			else
				std::cout << "Cloud 2 size: NULL\n";
			if (actual_displaying_frame_final_ != nullptr)
				std::cout << "Cloud final size: " << actual_displaying_frame_final_->size() << "\n";
			else
				std::cout << "Cloud final size: NULL\n";
			aligner.DisplaySizes();
			break;
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
			rotation_y_ -= Y_ROTATION_VELOCITY * ceil(static_cast<double>(len_x) / 10.0);
			prev_mouse_x_ = LOWORD(lParam);
		}
		if (right_was_clicked_)
		{
			short len_y = abs(HIWORD(lParam)) - abs(prev_mouse_y_);
			rotation_x_ += X_ROTATION_VELOCITY * ceil(static_cast<double>(len_y) / 10.0);
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

unsigned char* ThorClientSystem::ReadBmpFromFile(char* szFileName, int& riWidth, int& riHeight)
{
	BITMAPFILEHEADER bfh;
	BITMAPINFOHEADER bih;

	int i, j, h, v, lev, l, ls;
	unsigned char* buff = NULL;

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
			pRGBBuffer = new unsigned char[riWidth * riHeight * 3]; //Zaalokowanie odpowiedniego buffora obrazu

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
					for (i = 0 , l = 0; i < v; i++)
					{
						pRGBBuffer[((j * riWidth) + i) * 3 + 2] = p_palette[(buff[i] << 2) + 2];//R
						pRGBBuffer[((j * riWidth) + i) * 3 + 1] = p_palette[(buff[i] << 2) + 1];//G
						pRGBBuffer[((j * riWidth) + i) * 3 + 0] = p_palette[(buff[i] << 2) + 0];//B
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
					for (i = 0 , l = 0; i < v; i++ , l += 3)
					{
						pRGBBuffer[((j * riWidth) + i) * 3 + 0] = buff[l + 0];
						pRGBBuffer[((j * riWidth) + i) * 3 + 1] = buff[l + 1];
						pRGBBuffer[((j * riWidth) + i) * 3 + 2] = buff[l + 2];
					};
				};
				break;
			case 32:
				// RGBA bitmap 
				for (j = (h - 1); j >= 0; j--)
				{
					fread(buff, v * 4, 1, hfile);
					for (i = 0 , l = 0; i < v; i++ , l += 4)
					{
						pRGBBuffer[((j * riWidth) + i) * 3 + 0] = buff[l + 0];
						pRGBBuffer[((j * riWidth) + i) * 3 + 1] = buff[l + 1];
						pRGBBuffer[((j * riWidth) + i) * 3 + 2] = buff[l + 2];
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

void ThorClientSystem::LoadIPFromFile(char* file_name)
{
	std::ifstream file(file_name);
	char prefix[20] = "";
	// Server 1
	file >> prefix;
	if (prefix[0] == '#')
	{
		std::cout << "\n" << prefix << " ";
		file >> server_1_;
		std::cout << server_1_ << "\n";
	}
	// Sending port 1
	file >> prefix;
	if (prefix[0] == '#')
	{
		std::cout << prefix << " ";
		file >> sending_port_1_;
		std::cout << sending_port_1_ << "\n";
	}
	// Receiving port 1
	file >> prefix;
	if (prefix[0] == '#')
	{
		std::cout << prefix << " ";
		file >> recieving_port_1_;
		std::cout << recieving_port_1_ << "\n\n";
	}
	// Server 2
	file >> prefix;
	if (prefix[0] == '#')
	{
		std::cout << prefix << " ";
		file >> server_2_;
		std::cout << server_2_ << "\n";
	}
	// Sending port 2
	file >> prefix;
	if (prefix[0] == '#')
	{
		std::cout << prefix << " ";
		file >> sending_port_2_;
		std::cout << sending_port_2_ << "\n";
	}
	// Receiving port 2
	file >> prefix;
	if (prefix[0] == '#')
	{
		std::cout << prefix << " ";
		file >> recieving_port_2_;
		std::cout << recieving_port_2_ << "\n";
	}
	file.close();
}

bool ThorClientSystem::xLoadTexture()
{
	bool Status = FALSE;
	unsigned char* pp_logo = nullptr;
	int logo_width = 0,
		logo_height = 0;
	if (pp_logo = ReadBmpFromFile("pp_logo.bmp", logo_width, logo_height))
	{
		Status = TRUE;

		glGenTextures(1, &texture[0]);
		glBindTexture(GL_TEXTURE_2D, texture[0]);

		// Generate The Texture
		glTexImage2D(GL_TEXTURE_2D, 0, 3, logo_width, logo_height, 0, GL_RGB, GL_UNSIGNED_BYTE, pp_logo);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // Linear Filtering
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Linear Filtering
	}
	if (pp_logo) // If Texture Exists
	{
		delete[] pp_logo;
	}

	return Status;
}

void ThorClientSystem::RunSystem()
{
	for (int i = 0; i < num_of_servers_; ++i)
	{
		client_recievers_[i]->RunReciever();
	}
}

bool ThorClientSystem::xInitGLScene()
{
	cSaveRestoreRC c(m_hDC, m_hRC);

	if (!xLoadTexture()) // Jump To Texture Loading Routine ( NEW )
	{
		return FALSE; // If Texture Didn't Load Return FALSE ( NEW )
	}

	glEnable(GL_TEXTURE_2D); // Enable Texture Mapping ( NEW )
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f); // ustawienie bufora g³ebi
	glEnable(GL_DEPTH_TEST); // w³¹czenie testowania g³êbi
	glDepthFunc(GL_LEQUAL); // ustawienie typu testowania
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glPointSize(3.0);

	return TRUE;
}

bool ThorClientSystem::xDrawGLScene()
{
	cSaveRestoreRC c(m_hDC, m_hRC);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // czyœæ ekran
	glLoadIdentity(); // zresetuj macierz modeli

	client_recievers_[0]->where_was_gondor_.lock();
	if (client_recievers_[0]->next_recieved_cloud_.new_frame_s)
	{
		actual_displaying_frame_1_ = client_recievers_[0]->next_recieved_cloud_.decompressed_cloud_s;
		client_recievers_[0]->next_recieved_cloud_.new_frame_s = false;
	}
	client_recievers_[0]->where_was_gondor_.unlock();

	client_recievers_[1]->where_was_gondor_.lock();
	if (client_recievers_[1]->next_recieved_cloud_.new_frame_s)
	{
		actual_displaying_frame_2_ = client_recievers_[1]->next_recieved_cloud_.decompressed_cloud_s;
		client_recievers_[1]->next_recieved_cloud_.new_frame_s = false;
	}
	client_recievers_[1]->where_was_gondor_.unlock();

	glTranslatef(0.0f, 0.0f, viewer_distance_);
	glColor3f(1.0, 1.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex3d(-(0.95), (0.95), -1.0 - viewer_distance_); //upper left
	glTexCoord2f(1.0f, 0.0f);
	glVertex3d(-(0.70), (0.95), -1.0 - viewer_distance_); // upper right
	glTexCoord2f(1.0f, 1.0f);
	glVertex3d(-(0.70), (0.70), -1.0 - viewer_distance_); // down right
	glTexCoord2f(0.0f, 1.0f);
	glVertex3d(-(0.95), (0.70), -1.0 - viewer_distance_); // down left
	glEnd();


	glRotatef(180.0 + rotation_y_, 0.0f, 1.0f, 0.0f);
	glRotatef(rotation_x_, 1.0f, 0.0f, .0f);

	//	glTranslatef(6.0f + x_axis_pos, 0.0f, 0.0f);
	//
	//	if (!(actual_displaying_frame_1_ == nullptr) && actual_displaying_frame_2_ == nullptr)
	//	{
	//		glBegin(GL_POINTS);
	//
	//		for (size_t i = 0; i < actual_displaying_frame_1_->points.size(); ++i)
	//		{
	//			glColor4f(actual_displaying_frame_1_->points[i].r / 255.0, actual_displaying_frame_1_->points[i].g / 255.0, actual_displaying_frame_1_->points[i].b / 255.0, actual_displaying_frame_1_->points[i].a / 255.0);
	//			glVertex3f(actual_displaying_frame_1_->points[i].x, actual_displaying_frame_1_->points[i].y, actual_displaying_frame_1_->points[i].z);
	//		}
	//
	//		glEnd();
	//	}

	//	glTranslatef(-12.0f, 0.0f, 0.0f);
	//	if (!(actual_displaying_frame_2_ == nullptr) && actual_displaying_frame_1_ == nullptr)
	//	{
	//		glBegin(GL_POINTS);
	//
	//		for (size_t i = 0; i < actual_displaying_frame_2_->points.size(); ++i)
	//		{
	//			glColor4f(actual_displaying_frame_2_->points[i].r / 255.0, actual_displaying_frame_2_->points[i].g / 255.0, actual_displaying_frame_2_->points[i].b / 255.0, actual_displaying_frame_2_->points[i].a / 255.0);
	//			glVertex3f(actual_displaying_frame_2_->points[i].x, actual_displaying_frame_2_->points[i].y, actual_displaying_frame_2_->points[i].z);
	//		}
	//
	//		glEnd();
	//	}

	static bool is_icp = true;

	glTranslatef(x_axis_pos, 0.0f, 0.0f);;
	if (!(actual_displaying_frame_1_ == nullptr) && !(actual_displaying_frame_2_ == nullptr))
	{
		// set clouds to alligner
//		aligner.setSourceCloud(actual_displaying_frame_1_);
//		aligner.setTargetCloud(actual_displaying_frame_2_);

		//		aligner.InitialAlign();
		//		actual_displaying_frame_final_ = aligner.getFinalCloud();

		if (actual_displaying_frame_1_->size() > 0 && actual_displaying_frame_2_->size() > 0)
		{
//			aligner.align();
			if (is_icp)
			{
				std::cout << "ICP start.";
				is_icp = false;
			}
		}
//		actual_displaying_frame_final_ = aligner.getFinalCloud();
		actual_displaying_frame_final_ = actual_displaying_frame_2_;

		glBegin(GL_POINTS);

		for (size_t i = 0; i < actual_displaying_frame_final_->points.size(); ++i)
		{
			glColor4f(actual_displaying_frame_final_->points[i].r / 255.0, actual_displaying_frame_final_->points[i].g / 255.0, actual_displaying_frame_final_->points[i].b / 255.0, actual_displaying_frame_final_->points[i].a / 255.0);
			glVertex3f(actual_displaying_frame_final_->points[i].x, actual_displaying_frame_final_->points[i].y, actual_displaying_frame_final_->points[i].z);
		}

		glEnd();
	}

	//cOpenGLWindow::xDrawGLScene();

	SwapBuffers(m_hDC);
	//rtri += 0.05f;
	return TRUE; // Wszytko posz³o ok
}

ThorClientSystem::ThorClientSystem(int iResourceID, char* sWindowName) : cOpenGLWindow(iResourceID, sWindowName)
{
	num_of_servers_ = 2;
	client_recievers_ = new ThorClientReciever*[num_of_servers_];
	LoadIPFromFile(); // Load IP from file
	client_recievers_[0] = new ThorClientReciever(sending_port_1_, recieving_port_1_, server_1_);
	client_recievers_[1] = new ThorClientReciever(sending_port_2_, recieving_port_2_, server_2_);
	rotation_x_ = 0.0f;
	rotation_y_ = 0.0f;
	x_axis_pos = 0.0f;
	viewer_distance_ = 0.0;
	left_was_clicked_ = false;
	right_was_clicked_ = false;

	actual_displaying_frame_final_ = (new pcl::PointCloud<pcl::PointXYZRGBA>)->makeShared();


	xInitGLScene();
}

ThorClientSystem::~ThorClientSystem()
{
	delete[] client_recievers_;
	prev_mouse_y_ = 0;
	prev_mouse_y_ = 0;
	xDestroyGLScene();
}
