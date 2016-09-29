#pragma once
#ifndef _SystemClientH_
#define _SystemClientH_


#include "ThorClientReciever.h"
#include "cOpenGLWindow.h"
#include "AlignService.h"

#define SERVER_1 "172.20.128.13"		//ip address od server
#define SENDING_PORT_1 8889			//The port o which to send data
#define RECIEVING_PORT_1 8888			//The port on whichto listen for incoming data

#define SERVER_2 "172.20.128.12"
#define SENDING_PORT_2 8891		//The port o which to send data
#define RECIEVING_PORT_2 8890			//The port on whichto listen for incoming data

#define SCROLLING_VELOCITY 0.05f
#define Y_ROTATION_VELOCITY 0.1f
#define X_ROTATION_VELOCITY 0.1f

class ThorClientSystem :public cOpenGLWindow
{
public:
	ThorClientSystem(int iResourceID, char* sWindowName);
	~ThorClientSystem();

	void RunSystem();


protected:

	virtual INT_PTR CALLBACK xWindowProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	virtual bool xDrawGLScene();
	virtual bool xInitGLScene();
	bool xLoadTexture();
	unsigned char* ReadBmpFromFile(char* szFileName, int &riWidth, int &riHeight);


	short prev_mouse_x_;
	short prev_mouse_y_;
	bool left_was_clicked_,
		right_was_clicked_;
	short len_x = 0,
		len_y = 0;
private:
	GLfloat rotation_x_;
	GLfloat rotation_y_;
	GLfloat viewer_distance_;
	GLuint  texture[1];
	GLfloat x_axis_pos;

	int num_of_servers_;

	AlignService aligner;

	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr actual_displaying_frame_1_,
		actual_displaying_frame_2_,
		actual_displaying_frame_final_;

	ThorClientReciever **client_recievers_;
};

#endif // !_SystemClientH_