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
	this->builder->get_widget("indicator", this->indicator);
	this->builder->get_widget("telemetryField", this->telemetryField);
	this->builder->get_widget("weatherInfo", this->weatherInfoField);
	this->builder->get_widget("restartButton", this->resartButton);
	telemetryFieldBuffer = telemetryField->get_buffer();
	weatherInfoBuffer = weatherInfoField->get_buffer();
	this->closeButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::closeWindow));
	this->resumePauseButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::pauseResumeCamera));
	this->resumePauseButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::restartServer));

	this->telemetryField->set_size_request(240, -1);
	indicator->set_size_request(240, 240);

	this->drawingImage->set("images/image_not_found.png");
	this->telemetryField->get_buffer()->set_text("No telemetry was received");



	// initialize variables
	imgAIBack = cairo_image_surface_create_from_png("images/ai_back.png");
	imgAICase = cairo_image_surface_create_from_png("images/ai_case.png");
	imgAIFace = cairo_image_surface_create_from_png("images/ai_face.png");
	imgAIRing = cairo_image_surface_create_from_png("images/ai_ring.png");
	imgASIFace = cairo_image_surface_create_from_png("images/asi_face.png");
	imgASIHand = cairo_image_surface_create_from_png("images/asi_hand.png");

	/* indicator-> */
	g_signal_connect((GtkWidget*)indicator->gobj(), "draw", G_CALLBACK(MainWindow::drawIndicator), NULL);
	/* g_signal_connect(G_OBJECT(artHorizon), "draw", G_CALLBACK(drawIndicator), NULL); */
	/* gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(artHorizon), */
	/* 		sigc::mem_fun(*this, &MainWindow::closeWindow), */
	/* 		NULL, NULL); */
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

bool MainWindow::updateTelemeryInfoOnscreen(onScreenTelemetryUpdate telmetryBufferUpdate)
{
	telmetryBufferUpdate.buffer->set_text(telmetryBufferUpdate.text);
	telmetryBufferUpdate.indicator->queue_draw();
	return true;
}

bool MainWindow::updateWeatherInfoOnscreen(onScreenWeatherInfoUpdate onScreenWeatherInfoUpdate)
{
	onScreenWeatherInfoUpdate.buffer->set_text(onScreenWeatherInfoUpdate.text);
	return true;
}


void MainWindow::updateData(string AirpaceInfo){
	g_idle_add(G_SOURCE_FUNC(updateWeatherInfoOnscreen), new onScreenWeatherInfoUpdate(weatherInfoBuffer, AirpaceInfo));
}

void MainWindow::updateData(pTeleGen data, mutex* dataMutex)
{
	stringstream ss;
	{
		lock_guard<mutex> m(*dataMutex);
		ss
		<< "-----------------------\n"
		<< "\nyaw x:   " + to_string(data.att.yaw)
		<< "\npitch y: " + to_string(data.att.pitch)
		<< "\nroll y:  " + to_string(data.att.roll)
		<< "\n-----------------------\n"
		<< "\nvoltage: " + to_string(data.batt.getVoltage)
		<< "\ncurrent: " + to_string(data.batt.getCurrent)
		<< "\ntemp:    " + to_string(data.att.temp)
		<< "\n-----------------------\n"
		<< "\nGPS NOS: " + to_string(data.gps.numberOfSatelites)
		<< "\nlat:     " + to_string(data.gps.latitude)
		<< "\nlot:     " + to_string(data.gps.longitude)
		<< "\ng speed: " + to_string(data.gps.groundSpeed);

		this->pitch = data.att.pitch;
		this->roll = data.att.roll;
		this->speed = data.gps.groundSpeed;
	}
	g_idle_add(G_SOURCE_FUNC(updateTelemeryInfoOnscreen), new onScreenTelemetryUpdate(telemetryFieldBuffer, Glib::RefPtr<Gtk::DrawingArea>(indicator), ss.str()));
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
	switch (mainWindow->currentIndicator) {
	case ATTITUDE_INDICATOR:
		mainWindow->drawAttitudeIndicator(widget, cr, data);
		break;
	case AIRSPEED_INDICATOR:
		mainWindow->drawAirspeedIndicator(widget, cr, data);
		break;
	case HORIZONTAIL_SITUATION_INDICATOR:
		mainWindow->drawHorizonTailSituationIndicator(widget, cr, data);
		break;
	}
}

void MainWindow::drawAttitudeIndicator(GtkWidget* widget, cairo_t* cr, gpointer data)
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

	cairo_set_source_surface(cr, imgAIBack, 0, 0);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface(cr, imgAIRing, 0, 0);
	cairo_paint(cr);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface(cr, imgAIFace, 0, -pitch);
	cairo_paint(cr);

	// rotate
	cairo_translate(cr, 120, 120);
	cairo_rotate(cr, -angle);
	cairo_translate(cr, -120, -120);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface(cr, imgAICase, 0, 0);
	cairo_paint(cr);

	cairo_fill(cr);
}

void MainWindow::drawAirspeedIndicator(GtkWidget* widget, cairo_t* cr, gpointer data)
{
	guint width, height;
	GtkStyleContext* context;

	context = gtk_widget_get_style_context(widget);

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);

	gtk_render_background(context, cr, 0, 0, width, height);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface(cr, imgASIFace, 0, 0);
	cairo_paint(cr);

	// rotate

	// TODO: temporary solution, graphic is incorrect
	// need to find
	// 	* stall speed
	//  * operating speed
	//  * optimal speed (?)
	//  * never exceed speed
	double angle = 36;
	if ( speed < 20 )
			angle += 3.6 * speed;
	else if ( speed < 40 )
			angle += 74 + 4 * ( speed - 20.0 );
	else if ( speed < 60 )
			angle += 154.0 + 3.8 * ( speed - 40.0 );
	else
			angle += 230.0 + 4 * ( speed - 60.0 );

	cairo_translate(cr, 120, 120);
	cairo_rotate(cr, angle/180*M_PI);
	cairo_translate(cr, -120, -120);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface(cr, imgASIHand, 0, 0);
	cairo_paint(cr);

	cairo_fill(cr);


}
void MainWindow::drawHorizonTailSituationIndicator(GtkWidget* widget, cairo_t* cr, gpointer data)
{
}



void MainWindow::switchIndicator(bool back)
{
	if (back) {
		if(currentIndicator== 0) {
			currentIndicator = NUMBER_OF_INDICATORS - 1;
		} else {
			currentIndicator--;
		}
	} else {
		if(currentIndicator == NUMBER_OF_INDICATORS-1) {
			currentIndicator = 0;
		} else {
			currentIndicator++;
		}
		cout << currentIndicator << "\n";
	}
	indicator->queue_draw();
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
		if ((std::chrono::steady_clock::now()-lastChange).count() < 200000000)
			return 0;
		lastChange = std::chrono::steady_clock::now();
		if (x == LOW_CONTROLLER_AXIS_VALUE)
			mainWindow->switchIndicator(true);
		if (x == MAX_CONTROLLER_AXIS_VALUE)
			mainWindow->switchIndicator(false);
	} break;
	default:
		break;
	}
	return 0;
}

