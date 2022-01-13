/*
 * camera_streamer.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

//#include "opencv2/videoio.hpp"
//#include <opencv2/core/core.hpp>
#include <string>
#include <iostream>

//using namespace cv;
using namespace std;

#ifndef CAMERA_STREAMER_H
#define CAMERA_STREAMER_H

// TODO:: don't rely on calling external command
class CameraStreamer{
	private:

		const bool debug = true;
		const int cameraIndex;
		const int port;
		const string ipadd;


	public:
		CameraStreamer(const int cameraIndex, const int port, const string ipadd):cameraIndex(cameraIndex),port(port),ipadd(ipadd){};
		bool setupStream();
};

#endif /* !CAMERA_STREAMER_H */
