#pragma once

#include <libuvc/libuvc.h>

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <image_transport/camera_publisher.h>
#include <dynamic_reconfigure/server.h>
#include <camera_info_manager/camera_info_manager.h>
#include <boost/thread/mutex.hpp>

#include <uvc_camera_driver/UVCCameraConfig.h>

namespace libuvc_camera {

class CameraDriver {
public:
  CameraDriver(ros::NodeHandle nh, ros::NodeHandle priv_nh);
  ~CameraDriver();

  bool Start();
  void Stop();

private:
  enum State {
    kInitial = 0,
    kStopped = 1,
    kRunning = 2,
  };

  // Flags controlling whether the sensor needs to be stopped (or reopened) when changing settings
  // 控制更改设置时是否需要停止（或重新打开）传感器的标志
  static const int kReconfigureClose = 3; // Need to close and reopen sensor to change this setting // 需要关闭并重新打开传感器才能更改此设置
  static const int kReconfigureStop = 1; // Need to stop the stream before changing this setting // 更改此设置前需要停止数据流
  static const int kReconfigureRunning = 0; // We can change this setting without stopping the stream // 我们可以在不停止数据流的情况下更改这一设置

  void OpenCamera(UVCCameraConfig &new_config);
  void CloseCamera();

  // Accept a reconfigure request from a client
  // 接受来自客户端的重新配置请求
  void ReconfigureCallback(UVCCameraConfig &config, uint32_t level);
  enum uvc_frame_format GetVideoMode(std::string vmode);
  // Accept changes in values of automatically updated controls
  // 接受自动更新控件值的更改
  void AutoControlsCallback(enum uvc_status_class status_class,
                            int event,
                            int selector,
                            enum uvc_status_attribute status_attribute,
                            void *data, size_t data_len);
  static void AutoControlsCallbackAdapter(enum uvc_status_class status_class,
                                          int event,
                                          int selector,
                                          enum uvc_status_attribute status_attribute,
                                          void *data, size_t data_len,
                                          void *ptr);
  // Accept a new image frame from the camera
  // 从摄像机接收新的图像帧
  void ImageCallback(uvc_frame_t *frame);
  static void ImageCallbackAdapter(uvc_frame_t *frame, void *ptr);

  ros::NodeHandle nh_, priv_nh_;

  State state_;
  boost::recursive_mutex mutex_;

  uvc_context_t *ctx_;
  uvc_device_t *dev_;
  uvc_device_handle_t *devh_;
  uvc_frame_t *rgb_frame_;

  image_transport::ImageTransport it_;
  image_transport::CameraPublisher cam_pub_;

  dynamic_reconfigure::Server<UVCCameraConfig> config_server_;
  UVCCameraConfig config_;
  bool config_changed_;

  camera_info_manager::CameraInfoManager cinfo_manager_;

  // // autoadapt flag
  // bool autoFit_;
  // uvc_frame_t last_frame;
  // void autoFitlightness();
};

};