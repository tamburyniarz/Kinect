// Disable Error C4996 that occur when using Boost.Signals2.
#ifdef _DEBUG
#define _SCL_SECURE_NO_WARNINGS
#endif

#include "kinect2_grabber.h"
#include <pcl/visualization/pcl_visualizer.h>



int main(int argc, char* argv[])
{
	// PCL Visualizer
	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(
		new pcl::visualization::PCLVisualizer("Point Cloud Viewer"));
	viewer->setCameraPosition(0.0, 0.0, -2.5, 0.0, 0.0, 0.0);
	int counter = 0;
	// Point Cloud
	pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr cloud;

	// Retrieved Point Cloud Callback Function
	boost::mutex mutex;
	boost::function<void(const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr&)> function =
		[&cloud, &mutex](const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr& ptr) {
		boost::mutex::scoped_lock lock(mutex);
		cloud = ptr;
		printf("Grabbed next frame! \n");
	};

	// Kinect2Grabber
	boost::shared_ptr<pcl::Grabber> grabber = boost::make_shared<pcl::Kinect2Grabber>();

	// Register Callback Function
	boost::signals2::connection connection = grabber->registerCallback(function);

	// Start Grabber
	grabber->start();

	while (!viewer->wasStopped()) {

		//boost::mutex::scoped_try_lock lock(mutex);
		//if (cloud && lock.owns_lock()) {
		//	if (cloud->size() != 0) {
		//		/* Processing to Point Cloud */

		//		// Update Point Cloud
		//		if (!viewer->updatePointCloud(cloud, "cloud")) {
		//			viewer->addPointCloud(cloud, "cloud");
		//		}
		//	}
		//}
	}

	// Stop Grabber
	grabber->stop();

	// Disconnect Callback Function
	if (connection.connected()) {
		connection.disconnect();
	}

	return 0;
}


