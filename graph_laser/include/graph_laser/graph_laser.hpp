﻿#ifndef GRAPH_LASER_H
#define GRAPH_LASER_H

#include <string>
#include <vector>

#include <pcl/common/distances.h>
#include <pcl/common/eigen.h>
#include <pcl/common/transforms.h>
#include <pcl/point_types.h>
#include <pcl/features/vfh.h>
#include <pcl/features/normal_3d.h>
#include <pcl/keypoints/uniform_sampling.h>
#include <pcl/io/pcd_io.h>

#include <laser_slam/parameters.hpp>
#include <laser_slam/incremental_estimator.hpp>
#include <std_srvs/Empty.h>
#include <tf/transform_broadcaster.h>

#include "graph_laser/SaveMap.h"
#include "graph_laser/laser_slam_worker.hpp"

#include "opencv2/core/core.hpp"
#include "opencv2/flann/miniflann.hpp"


//激光建图的参数
struct graphLaserParams
{
  //是否在闭环之后清理局部地图
  bool clear_local_map_after_loop_closure = true;

  //是否发布世界坐标系到里程计坐标系的位姿变换
  bool publish_world_to_odom;
  //世界坐标系
  std::string world_frame;
  //tf发布频率
  double tf_publication_rate_hz;

  //位姿优化参数
  laser_slam::EstimatorParams online_estimator_params;
}; // struct graphLaserParams

class graphLaser
{
 public:
  explicit graphLaser(ros::NodeHandle& n);
  ~graphLaser();

  //建图线程
  void publishMapThread();

  //tf发布线程
  void publishTfThread();

  //闭环优化线程
  void loopProcessThread();

  //闭环检测线程
  void loopDetector();

  void downsample(const pcl::PointCloud<pcl::PointXYZ>::Ptr cloud,
                  const pcl::PointCloud<pcl::PointXYZ>::Ptr downsampled,
                  int factor);
  pcl::VFHSignature308 getVFHistigram(const pcl::PointCloud<pcl::PointXYZ>::Ptr cloud);

  std::vector<cv::DMatch> findMatch(const pcl::PointCloud<pcl::VFHSignature308>::Ptr &featuresDataBase,const int ind);
  void findMatchbyInd(const pcl::PointCloud<pcl::VFHSignature308>::Ptr &featuresDataBase,const int ind,cv::Mat &result);

 protected:
  //保存地图的服务
  bool saveMapServiceCall(graph_laser::SaveMap::Request& request,
                          graph_laser::SaveMap::Response& response);

 private:
  //获取ros的参数
  void getParameters();
  ros::NodeHandle& nh_;

  //参数类
  graphLaserParams params_;

  tf::TransformBroadcaster tf_broadcaster_;

  ros::ServiceServer save_distant_map_;
  ros::ServiceServer save_map_;

  //增量优化器
  std::shared_ptr<laser_slam::IncrementalEstimator> incremental_estimator_;


  //闭环优化线程频率
  static constexpr double kloopThreadRate_hz = 3.0;
  //闭环检测线程频率
  static constexpr double kloopDetetThreadRate_hz = 3.0;

  unsigned int next_track_id_ = 0u;

  //激光的graph_slam
  std::unique_ptr<graph_laser::LaserSlamWorker> laser_slam_worker_;
  //激光的graph_slam参数
  graph_laser::LaserSlamWorkerParams laser_slam_worker_params_;

  std::vector<laser_slam::SE3> features_pose;
  std::vector<laser_slam::Time> features_time;
  bool loop_succed;
  int last_cloudData_size=0;

  mutable std::recursive_mutex loopProcess_mutex_;
  //闭环位姿
  laser_slam::RelativePose loopRelativeclosure;
};

#endif // GRAPH_LASER_H
