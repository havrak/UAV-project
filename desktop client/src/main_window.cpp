/*
 * main_window.cpp
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "main_window.h"
#include "communication_interface.h"
#include "gdkmm/pixbuf.h"
#include "gtkmm/textbuffer.h"
#include "protocol_spec.h"
#include <bits/types/struct_tm.h>
#include <cstring>
#include <ctime>
#include <iterator>
#include <mutex>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <string>

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
	this->builder->get_widget("restartButton", this->resartButton);

	textBuffer = telemetryField->get_buffer();

	/* telemetryField->get_buffer(); */
	this->closeButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::stopCamera));
	this->resumePauseButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::pauseResumeCamera));
	this->resumePauseButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::restartServer));

	this->artHorizon->set("images/image_not_found.png");
	this->drawingImage->set("images/image_not_found.png");
	this->telemetryField->get_buffer()->set_text("No telemetry was received");
	initAttitudeIndicator();
	updateAttitudeIndicator();
}

MainWindow::~MainWindow()
{
}

void MainWindow::restartServer(){
	SendingThreadPool::GetInstance()->scheduleToSend(SendingStructure(P_SET_RESTART, 0x10, 0));
}

void MainWindow::pauseResumeCamera()
{
	this->paused = !this->paused;
	if (this->paused) {
		this->resumePauseButton->set_label("Resume");
	} else {
		this->resumePauseButton->set_label("Pause");
	}
}

void MainWindow::stopCamera()
{
	Window::close();
}

void MainWindow::updateImage(cv::Mat& frame)
{
	if (!frame.empty()) {
		float scaleX = (float)this->drawingImage->get_height() / frame.rows;
		float scaleY = (float)this->drawingImage->get_width() / frame.cols;

		float scaleFactor = (scaleX > scaleY ? scaleY : scaleX);
		Glib::RefPtr<Gdk::Pixbuf> bb = Gdk::Pixbuf::create_from_data(frame.data, Gdk::COLORSPACE_RGB, false, 8, frame.cols, frame.rows, frame.step);
		this->drawingImage->set(bb->scale_simple(bb->get_width() * scaleFactor, bb->get_height() * scaleFactor, (Gdk::InterpType)GDK_INTERP_BILINEAR));
		this->drawingImage->queue_draw();
	}
}

bool MainWindow::updateOnScreenTelemetry(textBufferUpdate telmetryBufferUpdate)
{
	/* GtkTextIter end; */
	cout << "updating\n";
	telmetryBufferUpdate.buffer->set_text(telmetryBufferUpdate.text);

	cout << "updating AI\n";
	mainWindow->updateAttitudeIndicator();
	/* gtk_text_buffer_get_end_iter(, &end); */
	/* cout << "text: " << update.text << " length: " << update.text.size(); */
	/* gtk_text_buffer_insert(telemetryBuffer, &end, update.text.c_str(), update.text.size()); */
	return true;
}

void MainWindow::updateData(pTeleGen data, mutex* dataMutex)
{
	string message;
	lock_guard<mutex> m(*dataMutex);
	message += "temp: " + to_string(data.att.temp) + "\n";
	message += "-----------------------\n";
	message += "yaw x: " + to_string(data.att.yaw) + "\n";
	message += "pitch y: " + to_string(data.att.pitch) + "\n";
	message += "roll y: " + to_string(data.att.roll) + "\n";
	message += "-----------------------\n";
	message += "Voltage: " + to_string(data.batt.getVoltage) + "\n";
	message += "Current: " + to_string(data.batt.getCurrent) + "\n";
	message += "-----------------------\n";
	message += "gyro x: " + to_string(data.att.gyroX) + "\n";
	message += "gyro y: " + to_string(data.att.gyroY) + "\n";
	message += "gyro y: " + to_string(data.att.gyroZ) + "\n";
	message += "-----------------------\n";
	message += "GPS NOS: " + to_string(data.gps.numberOfSatelites) + "\n";
	message += "lat: " + to_string(data.gps.latitude) + "\n";
	message += "lot: " + to_string(data.gps.longitude) + "\n";

	this->pitch = data.att.pitch;
	this->roll = data.att.roll;
	g_idle_add(G_SOURCE_FUNC(updateOnScreenTelemetry), new textBufferUpdate(textBuffer, message));

}

void MainWindow::displayError(pTeleErr error)
{
	GtkWidget* dialog = gtk_dialog_new_with_buttons(
			"Error",
			(GtkWindow*)mainWindow,
			(GtkDialogFlags)(GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT),
			"OK", GTK_RESPONSE_ACCEPT,
			NULL);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
	gtk_widget_set_size_request(GTK_WIDGET(dialog), 200, 130);

	gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(dialog))), gtk_label_new(error.message));
	g_signal_connect_swapped(dialog,
			"response",
			G_CALLBACK(gtk_widget_destroy),
			dialog);

	gtk_widget_show_all(dialog);
}


// courtesty of  https://stackoverflow.com/questions/37520296/can-a-gdk-pixbuf-be-rotated-by-something-less-than-90-degrees/39130357#39130357
Glib::RefPtr<Gdk::Pixbuf> MainWindow::rotatePixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, double angle)
{
	double s = sin(angle/180*M_PI), c = cos(angle/180*M_PI);
	double as = s < 0 ? -s : s, ac = c < 0 ? -c : c;
	int width, height, nwidth, nheight;
	int hasalpha, nhasalpha;
	int nr, nc, r, col;
	double nmodr, nmodc;
	int alpha = 0;
	guchar *pixels, *npixels, *pt, *npt;
	int rowstride, nrowstride, pixellen;
	;

	width = pixbuf->get_width();
	height = pixbuf->get_height();
	hasalpha = pixbuf->get_has_alpha();
	rowstride = pixbuf->get_rowstride();
	pixels = pixbuf->get_pixels();
	pixellen = hasalpha ? 4 : 3;

	if (true) {
		nwidth = round(ac * width + as * height);
		nheight = round(as * width + ac * height);
		nhasalpha = TRUE;
	} else {
		double denom = as * as - ac * ac;
		if (denom < .1e-7 && denom > -1.e-7) {
			nwidth = nheight = round(width / sqrt(2.0));
		} else {
			nwidth = round((height * as - width * ac) / denom);
			nheight = round((width * as - height * ac) / denom);
		}
		nhasalpha = hasalpha;
	}
	Glib::RefPtr<Gdk::Pixbuf> toReturn = Gdk::Pixbuf::create((Gdk::Colorspace)GDK_COLORSPACE_RGB, true, 8, nwidth, nheight);

	nrowstride = toReturn->get_rowstride();
	npixels = toReturn->get_pixels();

	for (nr = 0; nr < nheight; ++nr) {
		nmodr = nr - nheight / 2.0;
		npt = npixels + nr * nrowstride;
		for (nc = 0; nc < nwidth; ++nc) {
			nmodc = nc - nwidth / 2.0;
			/* Where did this pixel come from? */
			r = round(height / 2 - nmodc * s + nmodr * c);
			col = round(width / 2 + nmodc * c + nmodr * s);
			if (r < 0 || col < 0 || r >= height || col >= width) {
				alpha = 0;
				if (r < 0)
					r = 0;
				else if (r >= height)
					r = height - 1;
				if (col < 0)
					col = 0;
				else if (col >= width)
					col = width - 1;
			} else
				alpha = 0xff;
			pt = pixels + r * rowstride + col * pixellen;
			*npt++ = *pt++;
			*npt++ = *pt++;
			*npt++ = *pt++;
			if (hasalpha && alpha != 0)
				alpha = *pt;
			if (nhasalpha)
				*npt++ = alpha;
		}
	}
	return toReturn;
}

void MainWindow::initAttitudeIndicator()
{
	imgBackOri = Gdk::Pixbuf::create_from_file("images/ai_back.png");
	imgCaseOri = Gdk::Pixbuf::create_from_file("images/ai_case.png");
	imgFaceOri = Gdk::Pixbuf::create_from_file("images/ai_face.png");
	imgRingOri = Gdk::Pixbuf::create_from_file("images/ai_ring.png");

}

void MainWindow::setRollAndPitch(float pitch, float roll)
{
	lock_guard<mutex> mutex(attitudeValuesMutex);
}



void MainWindow::updateAttitudeIndicator()
	{
	cout << imgBackOri->get_width() << '\n';
	cout << "size: " << artHorizon->get_width() << " x " << artHorizon->get_height() << "\n";
	float scaleX = (float)artHorizon->get_height() / imgBackOri->get_height();
	float scaleY = (float)artHorizon->get_width() / imgBackOri->get_width();
	float scaleFactor = (scaleX > scaleY ? scaleY : scaleX);
	// create scaled copies
	imgBackCpy = imgBackOri->copy()->scale_simple(imgBackOri->get_width() * scaleFactor, imgBackOri->get_height() * scaleFactor, (Gdk::InterpType)GDK_INTERP_BILINEAR);
	imgRingCpy = imgRingOri->copy()->scale_simple(imgCaseOri->get_width() * scaleFactor, imgRingOri->get_height() * scaleFactor, (Gdk::InterpType)GDK_INTERP_BILINEAR);
	imgCaseCpy = imgCaseOri->copy()->scale_simple(imgCaseOri->get_width() * scaleFactor, imgCaseOri->get_height() * scaleFactor, (Gdk::InterpType)GDK_INTERP_BILINEAR);
	imgFaceCpy = imgFaceOri->copy()->scale_simple(imgCaseOri->get_width() * scaleFactor, imgFaceOri->get_height() * scaleFactor, (Gdk::InterpType)GDK_INTERP_BILINEAR);

	int width  = imgBackCpy->get_width();
	int height = imgBackCpy->get_height();
	// rotate copies;
	imgBackCpy = rotatePixbuf(imgBackCpy, roll);
	imgFaceCpy = rotatePixbuf(imgFaceCpy, roll);
	imgRingCpy = rotatePixbuf(imgRingCpy, roll);
	Glib::RefPtr<Gdk::Pixbuf> tmpbuf = Gdk::Pixbuf::create((Gdk::Colorspace)GDK_COLORSPACE_RGB, true, 8, width, height);

  double roll_rad = M_PI *roll / 180.0;

  double delta  = 1.7 * pitch;

  float faceDeltaX = delta * sin( roll_rad );
  float faceDeltaY = delta * cos( roll_rad );
	imgBackCpy->composite(imgRingCpy, 0, 0, width, height, 0,0,1, 1, (Gdk::InterpType) GDK_INTERP_BILINEAR, 1);
	imgBackCpy->composite(imgRingCpy, 0, 0, width, height, 0,0,1, 1, (Gdk::InterpType) GDK_INTERP_BILINEAR, 1);
	imgBackCpy->composite(imgFaceCpy, 0, 0, width, height, 0,0,1, 1, (Gdk::InterpType) GDK_INTERP_BILINEAR, 1);
	artHorizon->set(imgBackCpy);
}

bool setupCamera()
{
	while (!captureVideoFromCamera) {
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
