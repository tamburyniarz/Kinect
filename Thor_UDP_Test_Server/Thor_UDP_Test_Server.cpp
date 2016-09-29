/*
Simple UDP Server
Silver Moon ( m00n.silv3r@gmail.com )
*/
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS


#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/compression/octree_pointcloud_compression.h>


#include<iostream>
#include<sstream>
#include<stdlib.h>
#include<stdio.h>
#include<winsock2.h>

#pragma comment(lib,"ws2_32.lib") //Winsock Library

#define BUFLEN 100  //Max length of buffer
#define PORT 8888   //The port on which to listen for incoming data

int main()
{
	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len, number_of_frame_packets=0;
	char buf[BUFLEN];
	const char *conf = "12345";
	WSADATA wsa;

	bool recieving_packet_flag = false;

	slen = sizeof(si_other);

	//initializing all components needet to decoded and save recieved Point Cloud
	pcl::PointCloud<pcl::PointXYZRGBA> writeCloud;
	pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA>* PointCloudDecoder;
	PointCloudDecoder = new pcl::io::OctreePointCloudCompression<pcl::PointXYZRGBA>();
	// output pointcloud
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloudOut(new pcl::PointCloud<pcl::PointXYZRGBA>());


	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(PORT);

	//Bind
	if (::bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}

	//keep listening for data
	while (1)
	{
		printf("Waiting for data...");
		fflush(stdout);
		std::stringstream recieved_compressed_cp;

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}

		//print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));

		number_of_frame_packets = atoi(buf);

		printf("Number of datagrams recieved: %d", number_of_frame_packets);

		//now reply the client with the same data
		if (sendto(s, conf, 5, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			exit(EXIT_FAILURE);
		}
		printf("Confirmation sended \n");
		
		//Recieving Packets with data
		for (int i = 0; i < number_of_frame_packets; ++i)
		{
			printf("Waiting for data...");
			fflush(stdout);

			//clear the buffer by filling null, it might have previously received data
			memset(buf, '\0', BUFLEN);
			//try to receive some data, this is a blocking call
			if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
			{
				printf("recvfrom() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}

			//print details of the client/peer and the data received
			printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));


			for (int j = 0; j < BUFLEN; ++j)
				recieved_compressed_cp.put(buf[j]);

			if (i == number_of_frame_packets-1)
			{
				std::string terateratera = recieved_compressed_cp.str();
				// decompress point cloud
				PointCloudDecoder->decodePointCloud(recieved_compressed_cp, cloudOut);

				//writeCloud = *cloudOut;

				//pcl::io::savePCDFileASCII("frame_after_decopresion.pcd", writeCloud);
			}

			Sleep(1);
			//now reply the client with the same data
			if (sendto(s, conf, 5, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
			{
				printf("sendto() failed with error code : %d", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			printf("Confirmation sended \n");
		}
		Sleep(5);
	}

	closesocket(s);
	WSACleanup();

	return 0;
}