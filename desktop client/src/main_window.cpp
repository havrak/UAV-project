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
	this->closeButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::closeWindow));
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

void MainWindow::closeWindow()
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
	telmetryBufferUpdate.buffer->set_text(telmetryBufferUpdate.text);

/* 	cout << "updating AI\n"; */
/* 	mainWindow->updateAttitudeIndicator(); */
	/* gtk_text_buffer_get_end_iter(, &end); */
	/* cout << "text: " << update.text << " length: " << update.text.size(); */
	/* gtk_text_buffer_insert(telemetryBuffer, &end, update.text.c_str(), update.text.size()); */
	return true;
}

void MainWindow::updateData(pTeleGen data, mutex* dataMutex)
{
	string message;
	{
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
	}
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



void MainWindow::initAttitudeIndicator()
{
	imgBackOri = cairo_image_surface_create_from_png("images/ai_back.png");
	imgCaseOri = cairo_image_surface_create_from_png("images/ai_case.png");
	imgFaceOri = cairo_image_surface_create_from_png("images/ai_face.png");
	imgRingOri = cairo_image_surface_create_from_png("images/ai_ring.png");

}


void MainWindow::updateAttitudeIndicator()
	{
	/* float scaleX = (float)artHorizon->get_height() / imgBackOri->get_height(); */
	/* float scaleY = (float)artHorizon->get_width() / imgBackOri->get_width(); */
	/* float scaleFactor = (scaleX > scaleY ? scaleY : scaleX); */

	/* imgBackCpy = imgBackOri->copy(); */
	/* int width  = imgBackCpy->get_width(); */
	/* int height = imgBackCpy->get_height(); */

	/* imgBackCpy->composite(imgRingCpy, 0, 0, width, height, 0,0,1, 1, (Gdk::InterpType) GDK_INTERP_BILINEAR, 1); */
	/* imgBackCpy->composite(imgRingCpy, 0, 0, width, height, 0,0,1, 1, (Gdk::InterpType) GDK_INTERP_BILINEAR, 1); */

	/* imgBackCpy = rotatePixbuf(imgBackCpy, roll); */
	cairo_surface_t *merged = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, UI_SIZE, UI_SIZE);

	cairo_t *cr = cairo_create(merged);
	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	cairo_set_source_surface(cr, imgBackCpy, 0, 0);
	cairo_set_source_surface(cr, imgRingCpy, 0, 0);
	// rotate


	/* cairo_set_source_surface(cr, surface1, 0, 0); */
	/* cairo_paint(cr); */

	/* cairo_set_source_surface(cr, surface2, 0, height); */
	/* cairo_rectangle(cr, 0, height, width, height); */
	/* cairo_fill(cr); */

	cairo_destroy(cr);

	/* imgFaceCpy = rotatePixbuf(imgFaceCpy, roll); */
	/* imgRingCpy = rotatePixbuf(imgRingCpy, roll); */
	/* Glib::RefPtr<Gdk::Pixbuf> tmpbuf = Gdk::Pixbuf::create((Gdk::Colorspace)GDK_COLORSPACE_RGB, true, 8, width, height); */

  /* double rollRad = M_PI * roll / 180.0; */
  /* double delta  = (1.7*scaleFactor) * pitch; */
  /* float faceDeltaX = delta * sin( rollRad ); */
  /* float faceDeltaY = delta * cos( rollRad ); */
	/* imgBackCpy->composite(imgFaceCpy, 0, 0, width, height, faceDeltaY, faceDeltaX,1, 1, (Gdk::InterpType) GDK_INTERP_BILINEAR, 1); */
	/* imgBackCpy->scale_simple(imgBackOri->get_width() * scaleFactor, imgBackOri->get_height() * scaleFactor, (Gdk::InterpType)GDK_INTERP_BILINEAR); */
	Cairo::Surface sur = Cairo::Surface(imgBackCpy, false);
	sur.finish();

	/* artHorizon->set(); */
}

