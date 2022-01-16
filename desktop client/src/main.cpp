#include "main_window.h"
#include "controller_interface.h"
#include "linux_controller_implementation.h"
#include <gtkmm.h>
#include <iostream>
#include <stdio.h>

using namespace std;

std::mutex imageMutex;			 // pro managování zdílení obrazu mezi vláky
Glib::Dispatcher dispatcher; // pro komunikaci mezi vlákny
volatile bool captureVideoFromCamera = false;
cv::VideoCapture camera; // třída pro zachytávání orbrzau
cv::Mat frameBGR, frame; // dvojrozměnré pole, které reprezentuje koktrétní obrázek
MainWindow* cameraGrabberWindow = nullptr;

int main(int argc, char** argv)
{

	/* 	Gtk::Main app(argc, argv); */
	/* 	string imagePath = string{argv[2]}; */
	/* 	SingleImageWindow singleImageWindow(imagePath); */
	/* 	Gtk::Main::run(singleImageWindow); */

	Gtk::Main app(argc, argv); // stupstíme gtk okno
	Glib::RefPtr<Gtk::Builder> builder;
	try {
		builder = Gtk::Builder::create_from_file("main_window.glade");  // create gtk builder, load glade file
	} catch (const std::exception& p_exception) {
		cerr << p_exception.what();
	}
	cout << "MAIN | main | GTK window created" << endl;

	builder->get_widget_derived("MainWindow", cameraGrabberWindow); // vytvoří widget cameraGrabberWindow, který pracuje s GTK třídami
	cout << "MAIN | main | cameraGrabberWindow created" << endl;

	// ControllerInterface* ci = dynamic_cast<ControllerInterface*>(new LinuxControllerImplementation());

	LinuxControllerImplementation lci = LinuxControllerImplementation();

	if (cameraGrabberWindow) { // pokud se úspěšně vytvořilo, tak zobraz

		dispatcher.connect([&]() {
			imageMutex.lock();
			cameraGrabberWindow->updateImage(frame);
			imageMutex.unlock();
		});

		bool cameraInitialized = initializeCamera();

		if (cameraInitialized) {
			captureVideoFromCamera = true;
			std::thread cameraThread = std::thread(&cameraLoop);
			Gtk::Main::run(*cameraGrabberWindow);

			captureVideoFromCamera = false;
			cameraThread.join();
		} else {
			cout << "Failed to initialize the camera" << endl;
		}

	} else {
		cout << "Failed to initialize the GUI" << endl;
	}
	if (camera.isOpened()) {
		camera.release();
		cout << "Camera released success!" << endl;
	}
	return 0;
}
