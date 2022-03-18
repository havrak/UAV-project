/*
 * camera.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "camera.h"

Camera::Camera(int cameraPort, string myIP)
{
	this->cameraPort = cameraPort;
	this->myIP = myIP;
	cameraThread = thread(&Camera::cameraLoop, this);
}

/* bool Camera::setupCamera() */
/* { */
/* 	cameraThread = thread(&Camera::cameraLoop, this); */

/* 	while (!cameraInitialized && captureVideoFromCamera) { */
/* 		sendConfigurationOfCamera(); */
/* 		cout << "MAINWINDOW | setupCamera | setting up camera\n"; */
/* 	} */
/* 	return cameraInitialized; */
/* } */

void Camera::cameraLoop()
{
	while (!cameraInitialized && captureVideoFromCamera) {
		cameraInitialized = initializeCamera();
		if (cameraInitialized) {
			captureVideoFromCamera = true;

		} else {
			cerr << "MAINWINDOW | pausedResumeCamera | failed to initialize camera " << endl;
			/* mainWindow->resumePauseButton->set_label("start cam"); */
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

	}

	while (captureVideoFromCamera) {
		bool continueToGrabe = true;
		bool paused = mainWindow->isPaused();
		if (!paused) {
			continueToGrabe = cam.read(frameBGR);
			if (continueToGrabe) {
				imageMutex.lock();
				cv::cvtColor(frameBGR, frame, cv::COLOR_RGB2BGR);
				/* frameCorrected = cv::getOptimalNewCameraMatrix(frame, , imageSize, 1, imageSize, 0); */
				/* cv::undistort( frame, frame, frameCorrected, , frameCorrected); */
				imageMutex.unlock();
				dispatcher.emit();
			}
		}
		if (!continueToGrabe) {
			captureVideoFromCamera = false;
			cerr << "MAINWINDOW | main | Failed to retrieve frame from the device" << endl;
		} else if (paused) {
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		}
	}
}

bool Camera::initializeCamera()
{
	bool result = cam.open("udpsrc port=" + to_string(cameraPort) + "! application/x-rtp,media=video,payload=26,clock-rate=90000,encoding-name=JPEG,framerate=30/1 ! rtpjpegdepay ! jpegdec ! videoconvert ! appsink", cv::CAP_GSTREAMER);
	cout << "Camera was initialized\n";

	if (result) {
		for (int i = 0; i < 3; i++) {
			cam.grab();
		}
		for (int i = 0; result && i < 3; i++) { // calculate checksum
			result = result && cam.read(frameBGR);
		}
		imageSize = cv::Size(frameBGR.cols, frameBGR.rows);
	} else {
		cerr << "MAINWINDOW | initializeCamera | Camera failed to initialize\n";
	}

	return result;
}

void Camera::closeCamera()
{
	if (cam.isOpened()) {
		cam.release();
		cout << "MAIN | main | Camera released success!" << endl;
	}
}

