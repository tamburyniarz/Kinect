#pragma once
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

class AlignService
{
public:
	AlignService();
	~AlignService();
	void align();
	void InitialAlign();
	void InitialAlign(pcl::PointCloud<pcl::PointXYZRGBA>::Ptr& source_cloud, pcl::PointCloud<pcl::PointXYZRGBA>::Ptr& target_cloud);

	void DisplaySizes();

	// Getters and setters
	void setSourceCloud(const pcl::PointCloud<pcl::PointXYZRGBA>::Ptr& source_cloud);
	void setTargetCloud(const pcl::PointCloud<pcl::PointXYZRGBA>::Ptr& target_cloud);
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr getSourceCloud() const;
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr getTargetCloud() const;
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr getFinalCloud() const;

	float getLeafSize() const;
	void setLeafSize(float leaf_size);
private:
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr source_cloud_;
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr target_cloud_;
	pcl::PointCloud<pcl::PointXYZRGBA>::Ptr final_cloud_;

	pcl::PointCloud<pcl::PointXYZ>::Ptr source_cloud_XYZ_;
	pcl::PointCloud<pcl::PointXYZ>::Ptr target_cloud_XYZ_;
	pcl::PointCloud<pcl::PointXYZ>::Ptr source_cloud_XYZ_sampled_;
	pcl::PointCloud<pcl::PointXYZ>::Ptr target_cloud_XYZ_sampled_;
	pcl::PointCloud<pcl::PointXYZ>::Ptr aligned_cloud_XYZ_sampled_;

	void initClouds();
	bool CheckClouds();
	void ConvertInputsToXYZ();
	void VoxelGridSample(pcl::PointCloud<pcl::PointXYZ>::Ptr input, float leaf_size);
	void VoxelGridSample(pcl::PointCloud<pcl::PointXYZ>::Ptr input, pcl::PointCloud<pcl::PointXYZ>::Ptr output, float leaf_size);
	void ICP();
	void TransformCloud();
	void LoadMatrixFromfile(char* file_name = "ex_params.txt");

	bool is_debug_;
	float leaf_size_;

	Eigen::Matrix4f init_matrix_;
	Eigen::Affine3f init_transformation_;
	Eigen::Affine3f final_transformation_;
};

