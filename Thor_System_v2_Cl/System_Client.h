#pragma once
#ifndef _SystemClientH_
#define _SystemClientH_


#include <mutex>
#include <queue>
#include <thread>
#include <math.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/compression/octree_pointcloud_compression.h>
#include <winsock2.h>
#include "cOpenGLWindow.h"
#include "DatagramCollector.h"

#define SERVER "172.20.128.13"		//ip address od server
#define BUFLEN 10					//Max length of buffer
#define SEND_BUF_LEN 1000
#define RECBUFLEN 1024				// Length of recieving bufer
#define SENDING_PORT 8889			//The port o which to send data
#define RECIEVING_PORT 8888			//The port on whichto listen for incoming data
#define DEBUG_FLAG 0
#define RECIEVING_SOCKET_TIMEOUT 10
#define SCROLLING_VELOCITY 0.05f
#define Y_ROTATION_VELOCITY 0.1f
#define X_ROTATION_VELOCITY 0.1f
#define COMPRESSION_PROFILE pcl::io::LOW_RES_ONLINE_COMPRESSION_WITH_COLOR

class ThorClientSystem :public cOpenGLWindow
{
public:
	ThorClientSystem(int iResourceID, char* sWindowName);
	~ThorClientSystem();

	void RunSystem();

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

	struct NextDecompresedCloudBuffer
	{
		pcl::PointCloud<pcl::PointXYZRGBA>::Ptr decompressed_cloud_s;
		bool new_frame_s;
		
	};


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

	std::mutex one_to_rule_them_all_;									//mutex for client sender and reciever
	std::queue<ClientReplyMessage> message_queue_;						//queue for comunication between sender and reciever

	std::mutex where_was_gondor_;										//mutex for comunication between decoder and visualizer
	ThorClientSystem::NextDecompresedCloudBuffer next_recieved_cloud_;	//next recieved cloud

	std::mutex you_shall_not_pass_;										//mutex for comunication between reciever and decoder
	DatagramCollector *next_recieved_frame_;							//datagtrams containing next recieved frame;

	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr actual_displaying_frame;

	//void CloudVisualiser();
	void CloudDecoder();
	void ClientSender();
	void ClientReciever();
	void CreateMessageToSend(char *buffer, ThorClientSystem::ClientReplyMessage client_reciever_message);
	ThorClientSystem::ClientReplyMessage DatagramRecognition(char *buffer);

	//static void VisualiserThreadStarter(ThorClientSystem *my_client) { return my_client->CloudVisualiser(); }
	static void DecoderThreadStarter(ThorClientSystem *my_client) { return my_client->CloudDecoder(); }
	static void SenderThreadStarter(ThorClientSystem *my_client) { return my_client->ClientSender(); }
	static void RecieverThreadStarter(ThorClientSystem *my_client) { return my_client->ClientReciever(); }
};

#endif // !_SystemClientH_