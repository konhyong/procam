#ifndef INCLUDE_FREENECT2_WRAPPER_HPP
#define INCLUDE_FREENECT2_WRAPPER_HPP

// #include <cstlib> // needed?
#include <iostream>

#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/logger.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/libfreenect2.hpp>

using namespace al;
using namespace std;

struct Freenect2Wrapper {
  libfreenect2::Freenect2 freenect2;
  libfreenect2::Freenect2Device *dev = 0;
  libfreenect2::PacketPipeline *pipeline = 0;

  // both not currently used meaningfully but in place for multiple kinect
  // situations
  std::string serial = "";
  int deviceId = -1;
  // size_t framemax = -1;

  libfreenect2::SyncMultiFrameListener listener;
  libfreenect2::FrameMap frames;
  libfreenect2::Registration *registration;
  libfreenect2::Frame undistorted, registered;

  Freenect2Wrapper()
      : listener(libfreenect2::Frame::Color | libfreenect2::Frame::Ir |
                 libfreenect2::Frame::Depth),
        undistorted(512, 424, 4),
        registered(512, 424, 4) {}

  void init() {
    if (!pipeline) pipeline = new libfreenect2::CudaPacketPipeline(deviceId);

    if (freenect2.enumerateDevices() == 0) {
      cout << "no device connected!" << endl;
    }

    if (serial == "") {
      serial = freenect2.getDefaultDeviceSerialNumber();
    }

    if (pipeline) {
      dev = freenect2.openDevice(serial, pipeline);
    }

    if (dev == 0) {
      cout << "failure opening device!" << endl;
      exit(EXIT_FAILURE);
    }

    dev->setColorFrameListener(&listener);
    dev->setIrAndDepthFrameListener(&listener);

    if (!dev->start()) {
      cout << "error starting device" << endl;
      exit(EXIT_FAILURE);
    }

    cout << "device serial: " << dev->getSerialNumber() << endl;
    cout << "device firmware: " << dev->getFirmwareVersion() << endl;

    registration = new libfreenect2::Registration(dev->getIrCameraParams(),
                                                  dev->getColorCameraParams());
  }

  void update() {
    if (!listener.waitForNewFrame(frames, 10 * 1000)) {
      cout << "timed out!" << endl;
      exit(EXIT_FAILURE);
    }
    libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
    libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
    libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];

    registration->apply(rgb, depth, &undistorted, &registered);
  }

  void release() {
    dev->stop();
    dev->close();
    delete registration;
  }
};

#endif