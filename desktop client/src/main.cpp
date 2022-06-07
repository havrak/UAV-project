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
#include "airmap_provider.h"
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
mutex imageMutex;


cv::Mat frameBGR, frame, frameCorrected; // Matrix to store image from camera
cv::Size imageSize;
bool cameraInitialized = false;


MainWindow* mainWindow = nullptr;
Camera* cam = nullptr;
ControllerInterface * ci = nullptr;

void cleanup(){
	cam->closeCamera();
	ci->process= false;
	ci->terminateObservatoryAndObservers();

	CommunicationInterface::GetInstance()->cleanup();
}

void signalHandler( int sigNum){
	mainWindow->closeWindow();
	cleanup();
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

	builder->get_widget_derived("MainWindow", mainWindow);
	cout << "MAIN | main | GTK window created\n";

	Config config;
	config.loadConfiguration();

	CommunicationInterface::GetInstance()->setupSocket(config.getServerIP(), config.getMyIP(), config.getServerPort());
	CommunicationInterface::GetInstance()->setCameraPort(config.getCameraPort());
	cout << "MAIN | main | socket setted up \n";

	if(!config.getAirmapAPIKey().empty()){ // don't start it's internal loop to fetch data if api key was not provided
		AirmapProvider::GetInstance()->setupFetching(config.getAirmapAPIKey());
	}

	switch(config.getOperatingSystem()){
		case LINUX:
			ci = new LinuxControllerImplementation(config.getControllerType());
		default:
			break;
	}

	if(config.getControlEnabled())
		ci->addObserver((ControlInterpreter*) new ControllerDroneBridge);

	ci->addObserver((ControlInterpreter*) new ControllerUIBridge);

	cout << "MAIN | main | controller setted up \n";

	cam = new Camera(config.getCameraPort(), config.getMyIP());

	if (mainWindow) {

		dispatcher.connect([&]() {
			imageMutex.lock();
			mainWindow->updateImage(frame);
			imageMutex.unlock();
		});

		Gtk::Main::run(*mainWindow);

		// NOTE: cleanup after window is closed
		cleanup();
	} else {
		cerr << "MAIN | main | Failed to initialize the GUI" << endl;
	}


	return 0;
}
