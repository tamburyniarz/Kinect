// Disable Error C4996 that occur when using Boost.Signals2.
#ifdef _DEBUG
#define _SCL_SECURE_NO_WARNINGS
#endif

#include "kinect2_grabber.h"
#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/compression/octree_pointcloud_compression.h>

typedef pcl::PointXYZRGBA PointType;

void SavingCloudPointFrame(UINT32 number_of_frame, pcl::PointCloud<PointType>::ConstPtr &cloud)
{
	//pcl::io::savePCDFileASCII("test_pcd.pcd", cloud);




	return;
}

int main(int argc, char* argv[])
{
	//Creating a couter for saved frames
	UINT32 frame_counter = 0;

	// PCL Visualizer
	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(
		new pcl::visualization::PCLVisualizer("Point Cloud Viewer"));
	viewer->setCameraPosition(0.0, 0.0, -2.5, 0.0, 0.0, 0.0);

	//Instances for coder and encoder
	pcl::io::OctreePointCloudCompression<PointType>* PointCloudEncoder;
	pcl::io::OctreePointCloudCompression<PointType>* PointCloudDecoder;





	bool showStatistics = true;

	// for a full list of profiles see: /io/include/pcl/compression/compression_profiles.h
	pcl::io::compression_Profiles_e compressionProfile = pcl::io::LOW_RES_ONLINE_COMPRESSION_WITH_COLOR;

	// instantiate point cloud compression for encoding and decoding
	PointCloudEncoder = new pcl::io::OctreePointCloudCompression<PointType>(compressionProfile, showStatistics);
	PointCloudDecoder = new pcl::io::OctreePointCloudCompression<PointType>();


	// Point Cloud
	pcl::PointCloud<PointType>::ConstPtr cloud;

	// Retrieved Point Cloud Callback Function
	boost::mutex mutex;
	boost::function<void(const pcl::PointCloud<PointType>::ConstPtr&)> function =
		[&cloud, &mutex, &frame_counter, &PointCloudDecoder, &PointCloudEncoder, &viewer](const pcl::PointCloud<PointType>::ConstPtr& ptr) {
		if (!viewer->wasStopped())
		{
			boost::mutex::scoped_lock lock(mutex);
			cloud = ptr;
			//SavingCloudPointFrame(frame_counter++);

			pcl::PointCloud<PointType> writeCloud;

			writeCloud = *cloud;
			pcl::io::savePCDFileASCII("original_frame.pcd", writeCloud);
			//cloud = cloudOut;

			// stringstream to store compressed point cloud
			std::stringstream compressedData;
			// output pointcloud
			pcl::PointCloud<PointType>::Ptr cloudOut(new pcl::PointCloud<PointType>());

			// compress point cloud
			PointCloudEncoder->encodePointCloud(cloud, compressedData);

			//// decompress point cloud
			PointCloudDecoder->decodePointCloud(compressedData, cloudOut);

			writeCloud = *cloudOut;

			pcl::io::savePCDFileASCII("frame_after_decopresion.pcd", writeCloud);
			//cloud = cloudOut;
			printf("udalo sie !");
			Sleep(1000);

			printf("too late");
		}
	};


	


	// Kinect2Grabber
	boost::shared_ptr<pcl::Grabber> grabber = boost::make_shared<pcl::Kinect2Grabber>();

	// Register Callback Function
	boost::signals2::connection connection = grabber->registerCallback(function);

	// Start Grabber
	grabber->start();

	while (!viewer->wasStopped()) {
		// Update Viewer
		viewer->spinOnce();
		boost::mutex::scoped_try_lock lock(mutex);
		if (cloud && lock.owns_lock()) {
			if (cloud->size() != 0) {
				/* Processing to Point Cloud */

				// Update Point Cloud
				if (!viewer->updatePointCloud(cloud, "cloud")) {
					viewer->addPointCloud(cloud, "cloud");
				}
			}
		}
	}

	// Stop Grabber
	grabber->stop();

	// Disconnect Callback Function
	if (connection.connected()) {
		connection.disconnect();
	}


	getchar();
	return 0;
}