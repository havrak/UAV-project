/*
 * main_window.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "main_window.h"
#include "protocol_spec.h"
#include <opencv2/calib3d.hpp>
#include <opencv2/videoio.hpp>

using namespace std;

MainWindow::MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
		: Gtk::Window(cobject)
		, builder(refGlade)
{

	this->paused = false;
	this->builder->get_widget("drawingImage", this->drawingImage);
	this->builder->get_widget("closeButton", this->closeButton);
	this->builder->get_widget("resumePauseButton", this->resumePauseButton);
	this->builder->get_widget("artHorizon", this->artHorizon);
	this->builder->get_widget("telemetryField", this->telemetryField);
	this->closeButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::stopCamera));
	this->resumePauseButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::pauseResumeCamera));

	this->artHorizon->set("images/image_not_found.png");
	this->drawingImage->set("images/image_not_found.png");
}

MainWindow::~MainWindow()
{
}

void MainWindow::pauseResumeCamera()
{
	this->paused = !this->paused;
	if (this->paused) {
		this->resumePauseButton->set_label("resume");
	} else {
		this->resumePauseButton->set_label("pause");
	}
}

void MainWindow::stopCamera()
{
	Window::close();
}

void MainWindow::updateImage(cv::Mat& frame)
{
	if (!frame.empty()) {
		this->drawingImage->set(Gdk::Pixbuf::create_from_data(frame.data, Gdk::COLORSPACE_RGB, false, 8, frame.cols, frame.rows, frame.step));
		this->drawingImage->queue_draw();
	}
}

void MainWindow::updateData(pTeleGen data, mutex* dataMutex)
{
}

void MainWindow::displayError(pTeleErr error)
{
	GtkWidget *dialog = gtk_dialog_new_with_buttons(
			"Error",
			(GtkWindow*) mainWindow,
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			"OK", GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_container_set_border_width (GTK_CONTAINER (dialog), 5);
	gtk_widget_set_size_request (GTK_WIDGET (dialog), 200, 130);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), gtk_label_new(error.message));
	g_signal_connect_swapped(dialog,
			"response",
			G_CALLBACK(gtk_widget_destroy),
			dialog);

	gtk_widget_show_all(dialog);
}

bool setupCamera()
{
	while (true) {
		cout << "MAINWINDOW | setupCamera | setting up camera\n";
		cameraInitialized = initializeCamera();
		if (cameraInitialized) {
			captureVideoFromCamera = true;
			cameraThread = thread(&cameraLoop);

		} else {
			cerr << "MAINWINDOW | pausedResumeCamera | failed to initialize camera " << endl;
			/* mainWindow->resumePauseButton->set_label("start cam"); */
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	return cameraInitialized;
}

void cameraLoop()
{
	while (captureVideoFromCamera) {
		bool continueToGrabe = true;
		bool paused = mainWindow->isPaused();
		if (!paused) {
			continueToGrabe = camera.read(frameBGR);
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

bool initializeCamera()
{
	bool result = camera.open("udpsrc port=5000 ! application/x-rtp,media=video,payload=26,clock-rate=90000,encoding-name=JPEG,framerate=30/1 ! rtpjpegdepay ! jpegdec ! videoconvert ! appsink", cv::CAP_GSTREAMER);
	cout << "Camera was initialized\n";

	if (result) {
		for (int i = 0; i < 3; i++) {
			camera.grab();
		}
		for (int i = 0; result && i < 3; i++) { // calculate checksum
			result = result && camera.read(frameBGR);
		}
		imageSize = cv::Size(frameBGR.cols, frameBGR.rows);
	} else {
		cerr << "MAINWINDOW | initializeCamera | Camera failed to initialize\n";
	}

	return result;
}
