/*
 * camera_streamer.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "camera_streamer.h"


bool CameraStreamer::setupStream(){
	string command = "gst-launch-1.0 -v v4l2src device=/dev/video"+to_string(cameraIndex)+" ! videoconvert ! video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 ! jpegenc ! rtpjpegpay ! udpsink host="+ipadd+" port="+to_string(port)+" &";

	system(command.c_str());

	if(debug) cout << "CAMERA_STREAMER | setupStream | resutl: " << endl;
	return true;
}


