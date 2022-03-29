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
	imgBack = cairo_image_surface_create_from_png("images/ai_back.png");
	imgCase = cairo_image_surface_create_from_png("images/ai_case.png");
	imgFace = cairo_image_surface_create_from_png("images/ai_face_t.png");
	imgRing = cairo_image_surface_create_from_png("images/ai_ring.png");

	this->paused = false;
	this->builder->get_widget("drawingImage", this->drawingImage);
	this->builder->get_widget("closeButton", this->closeButton);
	this->builder->get_widget("resumePauseButton", this->resumePauseButton);
	this->builder->get_widget("indicator", this->indicator);
	this->builder->get_widget("telemetryField", this->telemetryField);
	this->builder->get_widget("restartButton", this->resartButton);
	textBuffer = telemetryField->get_buffer();
	/* indicatorCObj = ; */
	/* telemetryField->get_buffer(); */
	this->closeButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::closeWindow));
	this->resumePauseButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::pauseResumeCamera));
	this->resumePauseButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::restartServer));

	this->drawingImage->set("images/image_not_found.png");
	this->telemetryField->get_buffer()->set_text("No telemetry was received");
	/* gtk_widget_set_size_request((GtkWidget*)indicatorCObj, 240, 240); */
	indicator->set_size_request(240, 240);

	if (GTK_IS_DRAWING_AREA(indicator)) {
		cout << "TRUE\n";
	}
	/* indicator-> */
	g_signal_connect((GtkWidget*)indicator->gobj(), "draw", G_CALLBACK(MainWindow::drawIndicator), NULL);
	/* g_signal_connect(G_OBJECT(artHorizon), "draw", G_CALLBACK(drawIndicator), NULL); */
	/* gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(artHorizon), */
	/* 		sigc::mem_fun(*this, &MainWindow::closeWindow), */
	/* 		NULL, NULL); */
	initAttitudeIndicator();
	indicator->queue_draw();
	/* updateAttitudeIndicator(); */
}

MainWindow::~MainWindow()
{
}

void MainWindow::restartServer()
{
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

bool MainWindow::updateOnScreenTelemetry(onScreenTelemetryUpdate telmetryBufferUpdate)
{
	/* GtkTextIter end; */
	telmetryBufferUpdate.buffer->set_text(telmetryBufferUpdate.text);
	telmetryBufferUpdate.indicator->queue_draw();
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
	g_idle_add(G_SOURCE_FUNC(updateOnScreenTelemetry), new onScreenTelemetryUpdate(textBuffer, Glib::RefPtr<Gtk::DrawingArea>(indicator), message));
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

	gtk_widget_show_all(dialog); // NOTE: warning suppresed
}


void MainWindow::drawIndicator(GtkWidget* widget, cairo_t* cr, gpointer data)
{
	guint width, height;
	GtkStyleContext* context;

	context = gtk_widget_get_style_context(widget);

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);

	gtk_render_background(context, cr, 0, 0, width, height);

	double angle = roll * M_PI / 180;

	cairo_translate(cr, 120, 120);
	cairo_rotate(cr, angle);
	cairo_translate(cr, -120, -120);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);

	cairo_set_source_surface(cr, imgBack, 0, 0);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface(cr, imgRing, 0, 0);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface(cr, imgFace, 0, -pitch);
	cairo_paint(cr);

	// rotate
	cairo_translate(cr, 120, 120);
	cairo_rotate(cr, -angle);
	cairo_translate(cr, -120, -120);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface(cr, imgCase, 0, 0);
	cairo_paint(cr);

	cairo_fill(cr);
}

ControllerUIBridge::ControllerUIBridge()
{
	//
}

int ControllerUIBridge::update(ControlSurface cs, int val)
{
	switch (cs) {
	case START:
		break;
	case SELECT:
		break;
	case XBOX:
		break;
	default:
		break;
	}
	return 0;
}

int ControllerUIBridge::update(ControlSurface cs, int x, int y)
{
	switch (cs) {
	case D_PAD: {

	} break;
	default:
		break;
	}
	return 0;
}
