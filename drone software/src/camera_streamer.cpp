/*
 * camera_streamer.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "camera_streamer.h"
#include "communication_interface.h"
#include "protocol_spec.h"
#include <cstring>


CameraStreamer::CameraStreamer(){

}

bool CameraStreamer::setupStream()
{
	string command = "gst-launch-1.0 -v v4l2src device=/dev/video" + to_string(cameraIndex) + " ! videoconvert ! video/x-raw,format=YUY2,width=640,height=480,framerate=30/1 ! jpegenc ! rtpjpegpay ! udpsink host=" + ipaddr + " port=" + to_string(port) + " &";

	system(command.c_str());

	if (debug)
		cout << "CAMERA_STREAMER | setupStream | resutl: " << endl;
	return true;
}

int CameraStreamer::setUpCamera(ProcessingStructure ps)
{
	pSetCamera pc;
	memcpy(&pc, &ps.messageBuffer, sizeof(ps.messageBuffer));
	port = pc.port;
	for(int i = 0; i < 4 ; i++){
		ipaddr+= (int) pc.ip[i];
		if(i != 3) ipaddr+=".";
	}
	return setupStream();
}
