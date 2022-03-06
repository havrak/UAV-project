/*
 * camera_streamer.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "camera_streamer.h"
#include "communication_interface.h"
#include "protocol_spec.h"
#include <cstdlib>
#include <cstring>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <string>
#include <unistd.h>

CameraStreamer::CameraStreamer()
{
}


bool CameraStreamer::setupStream()
{
	string command = "gst-launch-1.0 -v autovideosrc device=/dev/video"+to_string(cameraIndex)+" ! video/x-raw,width="+to_string(capture_width)+",height="+to_string(capture_height)+",framerate="+to_string(framerate)+"/1 ! jpegenc ! rtpjpegpay ! udpsink host=" + ipaddr + " port=" + to_string(port) + " >/dev/null &";
	cout << "CAMERA_STREAMER | setupStream | command: " << command << "\n";
	system(command.c_str());

	/* string pipe = "-v autovideosrc device=/dev/video"+to_string(cameraIndex)+"; ! video/x-raw,width="+to_string(capture_width)+",height="+to_string(capture_height)+",framerate="+to_string(framerate)+"/1 ! jpegenc ! rtpjpegpay ! udpsink host=" + ipaddr + " port=" + to_string(port) + " >/dev/null &"; */
  /* char * pipeline[] = {const_cast<char *>(pipe.c_str()) ,NULL}; */
	/* int pid = fork(); */
	/* if(pid == 0) */
	/* 	execv("/usr/bin/gst-launch-1.0",pipeline); */

	//CommunicationInterface::GetInstance()->sendErrorMessageToAll(0x05, "Failed to open camera");
	/* return true; */

	/* string pipeline = "autovideosrc device=/dev/video"+to_string(cameraIndex)+" ! video/x-raw,width="+to_string(capture_width)+",height="+to_string(capture_height)+",framerate="+to_string(framerate)+"/1 ! jpegenc ! rtpjpegpay ! appsink"; */
	/* cout << "CAMERA_STREAMER | setupStream | pipeline: " << pipeline << "\n"; */
	/* cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER); */
	/* // free to do whatever with the stream probably */
	/* cv::VideoWriter writer("appsrc ! udpsink host="+ipaddr+" port="+to_string(port), 0, (double)30, cv::Size(640, 480), true); */
	/* if (!cap.isOpened() && !writer.isOpened()) { */
	/* 	cout << "CAMERA_STREAMER | setupStream | Failed to open camera\n"; */
	/* 	CommunicationInterface::GetInstance()->sendErrorMessageToAll(0x05, "Failed to open camera"); */
	/* 	return false; */
	/* } */
	/* cout << "CAMERA_STREAMER | setupStream | camera was open\n"; */
	/* return true; */
}

bool CameraStreamer::setUpCamera(ProcessingStructure ps)
{
	pSetCamera pc;
	memcpy(&pc, ps.getMessageBuffer(), ps.messageSize);
	for(int i = 0 ; i < 8 ; i++ ){
		cout << int(ps.getMessageBuffer()[i]) << " ";
	}
	cout << "\n";
	port = pc.port;
	for (int i = 0; i < 4; i++) {
		cout << int(pc.ip[i]);
		ipaddr += to_string(int(pc.ip[i]));
		if (i != 3)
			ipaddr += ".";
	}
	return setupStream();
}
