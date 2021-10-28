#include <gtkmm.h>
#include <iostream>

#include "camera_image_grabber.h"
//#include "single_image_window.h"

using namespace std;

std::mutex imageMutex;			 // pro managování zdílení obrazu mezi vláky
Glib::Dispatcher dispatcher; // pro komunikaci mezi vlákny
volatile bool captureVideoFromCamera = false;
cv::VideoCapture camera; // třída pro zachytávání orbrzau
cv::Mat frameBGR, frame; // dvojrozměnré pole, které reprezentuje koktrétní obrázek
CameraGrabberWindow* cameraGrabberWindow = nullptr;

int main(int argc, char** argv)
{

	/* 	Gtk::Main app(argc, argv); */
	/* 	string imagePath = string{argv[2]}; */
	/* 	SingleImageWindow singleImageWindow(imagePath); */
	/* 	Gtk::Main::run(singleImageWindow); */

	Gtk::Main app(argc, argv); // stupstíme gtk okno
	cout << "GTK window created" << endl;
	Glib::RefPtr<Gtk::Builder> builder;
	try {
		builder = Gtk::Builder::create_from_file("main_window.glade"); // načteme jeho podobu z glade
	} catch (const std::exception& p_exception) {
		cerr << p_exception.what();
	}
	cout << "GTK window created" << endl;

	builder->get_widget_derived("MainWindow", cameraGrabberWindow); // vytvoří widget cameraGrabberWindow, který pracuje s GTK třídami
	cout << "cameraGrabberWindow registered by GTK" << endl;
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
}
