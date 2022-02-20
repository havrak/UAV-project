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
#include "protocol_spec.h"
#include <csignal>
#include <gtkmm.h>
#include <iostream>
#include <stdio.h>
#include <thread>

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
LinuxControllerImplementation lci;

void signalHandler( int sigNum){
	CommunicationInterface::GetInstance()->cleanUp();
	lci.process = false;
	captureVideoFromCamera = false;
	exit(127);
}

int main(int argc, char** argv)
{
	signal(SIGABRT, signalHandler);
	signal(SIGKILL, signalHandler);

	Gtk::Main app(argc, argv); // stupstíme gtk okno
	Glib::RefPtr<Gtk::Builder> builder;
	try {
		builder = Gtk::Builder::create_from_file("main_window.glade");  // create gtk builder, load glade file
	} catch (const std::exception& p_exception) {
		cerr << p_exception.what();
	}
	cout << "MAIN | main | GTK window created\n";

	builder->get_widget_derived("MainWindow", mainWindow); // vytvoří widget cameraGrabberWindow, který pracuje s GTK třídami
	cout << "MAIN | main | cameraGrabberWindow created\n";

	CommunicationInterface::GetInstance()->setupSocket();
	cout << "MAIN | main | socket setted up \n";

	LinuxControllerImplementation lci = LinuxControllerImplementation();
	cout << "MAIN | main | controller setted up \n";

	ControllerDroneBridge::GetInstance();
	cout << "MAIN | main | drone bridge setted up \n";
	ControlInterpreter* ci = (ControlInterpreter* ) ControllerDroneBridge::GetInstance();

	lci.addObserver(ci);
	lci.generateEventForEveryButton();
	cout << "MAIN | main | drone bridge setted up \n";

	if (mainWindow) { // pokud se úspěšně vytvořilo, tak zobraz

		dispatcher.connect([&]() {
			imageMutex.lock();
			mainWindow->updateImage(frame);
			imageMutex.unlock();
		});

		thread cameraThread = thread(&setupCamera);

		/* while(true){ */ // debugging
		/* 	asm("nop"); */
		/* this_thread::sleep_for(chrono::milliseconds(100)); */
		/* } */
		//setupCamera();

		Gtk::Main::run(*mainWindow);

		// NOTE: cleanup after window is closed
		captureVideoFromCamera = false; // stop capturing video
		CommunicationInterface::GetInstance()->cleanUp();
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
