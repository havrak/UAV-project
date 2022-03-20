/*
 * main.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "communication_interface.h"
#include "control_interpreter.h"
#include "main_window.h"
#include "controller_interface.h"
#include "linux_controller_implementation.h"
#include "camera.h"
#include "protocol_spec.h"
#include "config.h"
#include <csignal>
#include <gtkmm.h>
#include <iostream>
#include <stdio.h>
#include <thread>
#include <X11/Xlib.h>



using namespace std;

Glib::Dispatcher dispatcher;
volatile bool captureVideoFromCamera = true;
mutex imageMutex;
cv::Mat frameBGR, frame, frameCorrected; // Matrix to store image from camera
cv::Size imageSize;
thread cameraThread;
MainWindow* mainWindow = nullptr;
bool cameraInitialized = false;
Camera* cam = nullptr;
LinuxControllerImplementation* lci = nullptr;

void signalHandler( int sigNum){
	CommunicationInterface::GetInstance()->cleanUp();
	lci->process = false;
	mainWindow->closeWindow();
	captureVideoFromCamera = false;
	exit(127);
}

int main(int argc, char** argv)
{
	signal(SIGABRT, signalHandler);
	signal(SIGKILL, signalHandler);

	XInitThreads();


	Gtk::Main app(argc, argv);
	Glib::RefPtr<Gtk::Builder> builder;
	try {
		builder = Gtk::Builder::create_from_file("main_window.glade");  // create gtk builder, load glade file
	} catch (const std::exception& p_exception) {
		cerr << p_exception.what();
	}
	cout << "MAIN | main | GTK window created\n";

	builder->get_widget_derived("MainWindow", mainWindow);
	cout << "MAIN | main | cameraGrabberWindow created\n";

	Config config;

	/* CommunicationInterface::GetInstance()->setupSocket(config.getServerIP(), config.getMyIP(), config.getServerPort()); */
	/* CommunicationInterface::GetInstance()->setCameraPort(config.getCameraPort()); // TODO: rework camera logic */
	cout << "MAIN | main | socket setted up \n";


	ControlInterpreter* droneControlInterpreter = nullptr;
	if(config.getControlEnabled())
		droneControlInterpreter = (ControlInterpreter* ) ControllerDroneBridge::GetInstance();


	switch(config.getOperatingSystem()){
		case LINUX:
			{
			lci = new LinuxControllerImplementation(config.getControllerType());
			if(droneControlInterpreter){
				lci->addObserver(droneControlInterpreter);
				cout << "MAIN | main | observer for drone bridge added\n";
			}
			}
		default:
			break;
	}
	cout << "MAIN | main | controller setted up \n";


	Camera camera(config.getCameraPort(), config.getMyIP());

	if (mainWindow) {

		dispatcher.connect([&]() {
			imageMutex.lock();
			mainWindow->updateImage(frame);
			imageMutex.unlock();
		});

		/* while(true){ */ // debugging
		/* 	asm("nop"); */
		/* this_thread::sleep_for(chrono::milliseconds(100)); */
		/* } */

		Gtk::Main::run(*mainWindow);

		// NOTE: cleanup after window is closed
		captureVideoFromCamera = false; // stop capturing video
		CommunicationInterface::GetInstance()->cleanUp();
		cameraThread.join(); // wait for camera thread to end

	} else {
		cerr << "MAIN | main | Failed to initialize the GUI" << endl;
	}


	return 0;
}
