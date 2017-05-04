#ifndef _ThorClientRecieverH_
#define _ThorClientRecieverH_

#include <mutex>
#include <queue>
#include <thread>
#include <math.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/compression/octree_pointcloud_compression.h>
#include <winsock2.h>
#include "DatagramCollector.h"


#define BUFLEN 10					//Max length of buffer
#define SEND_BUF_LEN 1000
#define RECBUFLEN 1024				// Length of recieving bufer
#define DEBUG_FLAG 1
#define RECIEVING_SOCKET_TIMEOUT 200
#define COMPRESSION_PROFILE pcl::io::LOW_RES_ONLINE_COMPRESSION_WITH_COLOR

class ThorClientReciever
{
public:
	ThorClientReciever(int sending_port, int recieving_port, char* server_IP);
	~ThorClientReciever();

	void RunReciever();

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
		unsigned int datagram_number_s;
		int	frame_number_s,
			sender_s;
	};

	struct NextDecompresedCloudBuffer
	{
		pcl::PointCloud<pcl::PointXYZRGBA>::Ptr decompressed_cloud_s;
		bool new_frame_s;

	};

	mutable std::mutex where_was_gondor_;											//mutex for comunication between decoder and visualizer
	ThorClientReciever::NextDecompresedCloudBuffer next_recieved_cloud_;	//next recieved cloud

private:
	int sending_port_,
		recieving_port_;
	char *server_IP_;
	HANDLE recieved_next_frame_event_;

	void ClientSender();
	void ClientReciever();
	void CloudDecoder();

	mutable std::mutex you_shall_not_pass_;										//mutex for comunication between reciever and decoder
	DatagramCollector *next_recieved_frame_;							//datagtrams containing next recieved frame;

	mutable std::mutex one_to_rule_them_all_;									//mutex for client sender and reciever
	std::queue<ClientReplyMessage> message_queue_;						//queue for comunication between sender and reciever

	void ThorClientReciever::CreateMessageToSend(char *buffer, ThorClientReciever::ClientReplyMessage client_reciever_message);
	ThorClientReciever::ClientReplyMessage DatagramRecognition(char *buffer);


	static void DecoderThreadStarter(ThorClientReciever *my_client) { return my_client->CloudDecoder(); }
	static void SenderThreadStarter(ThorClientReciever *my_client) { return my_client->ClientSender(); }
	static void RecieverThreadStarter(ThorClientReciever *my_client) { return my_client->ClientReciever(); }
};

#endif // !_ThorClientRecieverH_
