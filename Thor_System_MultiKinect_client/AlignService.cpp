#define _CRT_SECURE_NO_WARNINGS

#include "AlignService.h"

#include <fstream>
#include <pcl/filters/voxel_grid.h>
#include <pcl/registration/icp.h>

AlignService::AlignService()
{
	initClouds();

	is_debug_ = true;
	leaf_size_ = 0.01f;

	// Load external params
	LoadMatrixFromfile();
}


AlignService::~AlignService()
{
	// TODO erasing clouds
}

void AlignService::align()
{
	// Check clouds
	if (!CheckClouds())
	{
		std::cerr << "Align ERROR!";
		return;
	}

	// Convert inputs to XYZ
	ConvertInputsToXYZ();

	// voxel grid
	VoxelGridSample(source_cloud_XYZ_, source_cloud_XYZ_sampled_, leaf_size_);
	VoxelGridSample(target_cloud_XYZ_, target_cloud_XYZ_sampled_, leaf_size_);

	//	// align
	if (source_cloud_XYZ_sampled_->size() > 0 && target_cloud_XYZ_sampled_->size() > 0)
	{
		ICP();
		TransformCloud();
	} else
	{
		InitialAlign();
	}
}

void AlignService::InitialAlign()
{
	pcl::transformPointCloud(*source_cloud_, *final_cloud_, init_transformation_);
	*final_cloud_ += *target_cloud_;
}

void AlignService::InitialAlign(pcl::PointCloud<pcl::PointXYZRGBA>::Ptr& source_cloud, pcl::PointCloud<pcl::PointXYZRGBA>::Ptr& target_cloud)
{
	pcl::transformPointCloud(*source_cloud_, *target_cloud, init_transformation_);
}

void AlignService::DisplaySizes()
{
	std::cout << "source_cloud_XYZ_ size: " << source_cloud_XYZ_->size() << "\n";
	std::cout << "source_cloud_XYZ_ size: " << source_cloud_XYZ_->size() << "\n";
	std::cout << "source_cloud_XYZ_sampled_ size: " << source_cloud_XYZ_sampled_->size() << "\n";
	std::cout << "target_cloud_XYZ_sampled_ size: " << target_cloud_XYZ_sampled_->size() << "\n";
}

void AlignService::initClouds()
{
	final_cloud_ = (new pcl::PointCloud<pcl::PointXYZRGBA>)->makeShared();
	source_cloud_XYZ_ = (new pcl::PointCloud<pcl::PointXYZ>)->makeShared();
	target_cloud_XYZ_ = (new pcl::PointCloud<pcl::PointXYZ>)->makeShared();
	source_cloud_XYZ_sampled_ = (new pcl::PointCloud<pcl::PointXYZ>)->makeShared();
	target_cloud_XYZ_sampled_ = (new pcl::PointCloud<pcl::PointXYZ>)->makeShared();
	aligned_cloud_XYZ_sampled_ = (new pcl::PointCloud<pcl::PointXYZ>)->makeShared();
}

bool AlignService::CheckClouds()
{
	if (source_cloud_ == nullptr)
	{
		std::cerr << "source_cloud_ is NOT setted! \n";
		return false;
	}
	if (target_cloud_ == nullptr)
	{
		std::cerr << "target_cloud_ is NOT setted! \n";
		return false;
	}
	return true;
}

void AlignService::ConvertInputsToXYZ()
{
	pcl::copyPointCloud(*source_cloud_, *source_cloud_XYZ_);
	pcl::copyPointCloud(*target_cloud_, *target_cloud_XYZ_);
}

void AlignService::VoxelGridSample(pcl::PointCloud<pcl::PointXYZ>::Ptr input, float leaf_size)
{
	VoxelGridSample(input, input, leaf_size);
}

void AlignService::VoxelGridSample(pcl::PointCloud<pcl::PointXYZ>::Ptr input, pcl::PointCloud<pcl::PointXYZ>::Ptr output, float leaf_size)
{
	pcl::VoxelGrid<pcl::PointXYZ> vox_grid;
	vox_grid.setLeafSize(leaf_size, leaf_size, leaf_size);
	vox_grid.setInputCloud(input);
	vox_grid.filter(*output);
}

void AlignService::ICP()
{
	pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp;

	icp.setInputSource(source_cloud_XYZ_sampled_);
	icp.setInputTarget(target_cloud_XYZ_sampled_);

	// Set the max correspondence distance to 5cm (e.g., correspondences with higher distances will be ignored)
	icp.setMaxCorrespondenceDistance(0.05);
	// Set the maximum number of iterations (criterion 1)
	icp.setMaximumIterations(50);
	// Set the transformation epsilon (criterion 2)
	icp.setTransformationEpsilon(1e-8);
	// Set the euclidean distance difference epsilon (criterion 3)
	icp.setEuclideanFitnessEpsilon(1);

	icp.align(*aligned_cloud_XYZ_sampled_, init_matrix_);

	final_transformation_.matrix() = icp.getFinalTransformation();
}

void AlignService::TransformCloud()
{
	pcl::transformPointCloud(*source_cloud_, *final_cloud_, final_transformation_);
	*final_cloud_ += *target_cloud_;
}

void AlignService::LoadMatrixFromfile(char* file_name)
{
	float x, y, z, w;
	init_matrix_ = Eigen::Matrix4f::Identity();

	std::ifstream file(file_name);
	const float METER = 1000;
	for (int i = 0; i < 3; ++i)
	{
		file >> x >> y >> z >> w;

		init_matrix_(i, 0) = x;
		init_matrix_(i, 1) = y;
		init_matrix_(i, 2) = z;
		init_matrix_(i, 3) = w / METER; //convert mm to m
	}

	file.close();

	init_transformation_ = Eigen::Affine3f::Identity();
	init_transformation_.matrix() = init_matrix_;

	if (is_debug_)
	{
		std::cout << ">>Loaded init_transformation from " << file_name << ":\n";
		std::cout << init_matrix_ << "\n";
	}
}

/*
*
* Getters and setters
*
*/

void AlignService::setSourceCloud(const pcl::PointCloud<pcl::PointXYZRGBA>::Ptr& source_cloud)
{
	source_cloud_ = source_cloud;
}

void AlignService::setTargetCloud(const pcl::PointCloud<pcl::PointXYZRGBA>::Ptr& target_cloud)
{
	target_cloud_ = target_cloud;
}

pcl::PointCloud<pcl::PointXYZRGBA>::Ptr AlignService::getSourceCloud() const
{
	return source_cloud_;
}

pcl::PointCloud<pcl::PointXYZRGBA>::Ptr AlignService::getTargetCloud() const
{
	return target_cloud_;
}

pcl::PointCloud<pcl::PointXYZRGBA>::Ptr AlignService::getFinalCloud() const
{
	return final_cloud_;
}

float AlignService::getLeafSize() const
{
	return leaf_size_;
}

void AlignService::setLeafSize(float leaf_size)
{
	leaf_size_ = leaf_size;
}
