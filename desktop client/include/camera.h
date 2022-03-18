/*
 * camera.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef CAMERA_H
#define CAMERA_H

#include "gdkmm/pixbuf.h"
#include "glibmm/refptr.h"
#include "gtkmm/textbuffer.h"
#include "gtkmm/textview.h"
#include "main_window.h"
#include "protocol_spec.h"
#include "communication_interface.h"
#include <cairomm/refptr.h>
#include <cairomm/surface.h>
#include <gdk/gdk.h>
#include <gtkmm.h>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <thread>

using namespace std;

// TODO: do something like object pool, from which it would be possible to access camera anywhere from the codebase
// necessary in order for communication_interface not to have to remember useless crap which doesn't belong there

class Camera {
	public:
	thread cameraThread;

	Camera(int cameraPort, string myIP);
	void closeCamera();

	void sendConfigurationOfCamera();

	private:
	cv::VideoCapture cam;
	int cameraPort = 5000;
	string myIP;
	/* bool setupCamera(); */
	void cameraLoop();
	bool initializeCamera();

};

extern std::mutex imageMutex;
extern Glib::Dispatcher dispatcher;
extern volatile bool captureVideoFromCamera;
extern cv::Mat frameBGR, frame, frameCorrected;
extern MainWindow* mainWindow;
extern bool cameraInitialized;
extern cv::Size imageSize;


#endif /* !CAMERA_H */
