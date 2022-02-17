/*
 * camera_streamer.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

//#include "opencv2/videoio.hpp"
//#include <opencv2/core/core.hpp>
#include "protocol_spec.h"
#include <string>
#include <iostream>

//using namespace cv;
using namespace std;

#ifndef CAMERA_STREAMER_H
#define CAMERA_STREAMER_H

// TODO: rewrite it into a object that remembers each stream, not rely on setup and forget
class CameraStreamer{
	private:

		const bool debug = true;
		const int cameraIndex = 0;
		int port;
		string ipaddr = "";


	public:
		CameraStreamer(const int port, const string ipaddr):port(port),ipaddr(ipaddr){};
		CameraStreamer();
		bool setupStream();
		int setUpCamera(ProcessingStructure ps);
};

#endif /* !CAMERA_STREAMER_H */
