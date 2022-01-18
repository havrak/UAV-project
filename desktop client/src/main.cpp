/*
 * main.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "main_window.h"
#include "controller_interface.h"
#include "linux_controller_implementation.h"
#include <gtkmm.h>
#include <iostream>
#include <stdio.h>

using namespace std;

Glib::Dispatcher dispatcher;
volatile bool captureVideoFromCamera = false;
mutex imageMutex;
cv::VideoCapture camera; // opencv camera
cv::Mat frameBGR, frame, frameCorrected; // Matrix to store image from camera
cv::Size imageSize;
thread cameraThread;
MainWindow* mainWindow = nullptr;
bool cameraInitialized;

int main(int argc, char** argv)
{


	Gtk::Main app(argc, argv); // stupstíme gtk okno
	Glib::RefPtr<Gtk::Builder> builder;
	try {
		builder = Gtk::Builder::create_from_file("main_window.glade");  // create gtk builder, load glade file
	} catch (const std::exception& p_exception) {
		cerr << p_exception.what();
	}
	cout << "MAIN | main | GTK window created" << endl;

	builder->get_widget_derived("MainWindow", mainWindow); // vytvoří widget cameraGrabberWindow, který pracuje s GTK třídami
	cout << "MAIN | main | cameraGrabberWindow created" << endl;

	// ControllerInterface* ci = dynamic_cast<ControllerInterface*>(new LinuxControllerImplementation());

	LinuxControllerImplementation lci = LinuxControllerImplementation();

	if (mainWindow) { // pokud se úspěšně vytvořilo, tak zobraz
		dispatcher.connect([&]() {
			imageMutex.lock();
			mainWindow->updateImage(frame);
			imageMutex.unlock();
		});


		setupCamera();

		Gtk::Main::run(*mainWindow);

		// NOTE: cleanup after window is closed
		captureVideoFromCamera = false; // stop capturing video
		cameraThread.join(); // wait for camera thread to end

	} else {
		cerr << "MAIN | main | Failed to initialize the GUI" << endl;
	}


	if (camera.isOpened()) {
		camera.release();
		cout << "MAIN | main | Camera released success!" << endl;
	}
	return 0;
}
