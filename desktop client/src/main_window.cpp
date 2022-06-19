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
	const char* repo_uri;
	char *cachedir, *cachebasedir;

	repo_uri = osm_gps_map_source_get_repo_uri(opt_map_provider);

	/* if (repo_uri == NULL) { */
	/* } */
	cachebasedir = osm_gps_map_get_default_cache_directory();
	cachedir = g_strdup(OSM_GPS_MAP_CACHE_AUTO);

	map = (OsmGpsMap*)g_object_new(OSM_TYPE_GPS_MAP,
			"map-source", opt_map_provider,
			"tile-cache", cachedir,
			"tile-cache-base", cachebasedir,
			"proxy-uri", g_getenv("http_proxy"),
			"user-agent", "mapviewer.c", // Always set user-agent, for better tile-usage compliance
			NULL);

	osd = (OsmGpsMapLayer*)g_object_new(OSM_TYPE_GPS_MAP_OSD,
			"show-scale", TRUE,
			"show-coordinates", TRUE,
			"show-crosshair", TRUE,
			"show-dpad", TRUE,
			"show-zoom", TRUE,
			"show-gps-in-dpad", TRUE,
			"show-gps-in-zoom", FALSE,
			"dpad-radius", 30,
			NULL);

	osm_gps_map_layer_add(OSM_GPS_MAP(map), osd);
	g_object_unref(G_OBJECT(osd));
	g_free(cachedir);
	g_free(cachebasedir);
	osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_FULLSCREEN, GDK_KEY_F11);
	osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_UP, GDK_KEY_Up);
	osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_DOWN, GDK_KEY_Down);
	osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_LEFT, GDK_KEY_Left);
	osm_gps_map_set_keyboard_shortcut(map, OSM_GPS_MAP_KEY_RIGHT, GDK_KEY_Right);

	this->paused = false;
	this->builder->get_widget("drawingImage", this->cameraViewport);
	this->builder->get_widget("closeButton", this->closeButton);
	this->builder->get_widget("resumePauseButton", this->resumePauseButton);
	this->builder->get_widget("indicator", this->indicator);
	this->builder->get_widget("weatherIndicator", this->weatherIndicator);
	this->builder->get_widget("telemetryField", this->telemetryField);
	this->builder->get_widget("weatherInfo", this->weatherInfoField);
	this->builder->get_widget("restartButton", this->restartButton);
	this->builder->get_widget("map_box", this->map_box);

	telemetryFieldBuffer = telemetryField->get_buffer();
	weatherInfoBuffer = weatherInfoField->get_buffer();

	this->restartButton->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::restartServer));
	this->closeButton->signal_activate().connect(sigc::mem_fun(*this, &MainWindow::closeWindow));
	this->resumePauseButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::pauseResumeCamera));
	this->resumePauseButton->signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::restartServer));
	this->telemetryField->set_size_request(240, -1);
	weatherIndicator->set_size_request(100, 100);
	weatherInfoField->set_size_request(140, -1);
	indicator->set_size_request(240, 240);

	this->cameraViewport->set("images/image_not_found.png");
	this->telemetryField->get_buffer()->set_text("No telemetry was received");
	this->weatherInfoField->get_buffer()->set_text("No weather info was received");

	// initialize variables
	imgAIBack = cairo_image_surface_create_from_png("images/ai_back.png");
	imgAICase = cairo_image_surface_create_from_png("images/ai_case.png");
	imgAIFace = cairo_image_surface_create_from_png("images/ai_face.png");
	imgAIRing = cairo_image_surface_create_from_png("images/ai_ring.png");
	imgASIFace = cairo_image_surface_create_from_png("images/asi_face.png");
	imgASIHand = cairo_image_surface_create_from_png("images/asi_hand.png");
	imgWHBack = cairo_image_surface_create_from_png("images/wh_back.png");
	imgWHHand = cairo_image_surface_create_from_png("images/wh_hand.png");

	g_signal_connect((GtkWidget*)indicator->gobj(), "draw", G_CALLBACK(MainWindow::drawIndicator), NULL);
	g_signal_connect((GtkWidget*)weatherIndicator->gobj(), "draw", G_CALLBACK(MainWindow::drawWeatherIndicator), NULL);

	indicator->queue_draw();
	weatherIndicator->queue_draw();
	/* updateAttitudeIndicator(); */
	gtk_box_pack_start(GTK_BOX(gtk_builder_get_object(builder->gobj(), "map_box")), GTK_WIDGET(map), TRUE, TRUE, 0);

	/* gtk_box_pack_start(map_box->gobj(), GTK_WIDGET(map), TRUE, TRUE, 0); */
	OsmGpsMapTrack* gpstrack = osm_gps_map_gps_get_track(map);
	gtk_widget_set_size_request(GTK_WIDGET(map), 300, -1);
	gtk_widget_show_all(GTK_WIDGET(map));
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
		float scaleX = (float)this->cameraViewport->get_height() / frame.rows;
		float scaleY = (float)this->cameraViewport->get_width() / frame.cols;

		float scaleFactor = (scaleX > scaleY ? scaleY : scaleX);
		Glib::RefPtr<Gdk::Pixbuf> bb = Gdk::Pixbuf::create_from_data(frame.data, Gdk::COLORSPACE_RGB, false, 8, frame.cols, frame.rows, frame.step);
		this->cameraViewport->set(bb->scale_simple(bb->get_width() * scaleFactor, bb->get_height() * scaleFactor, (Gdk::InterpType)GDK_INTERP_BILINEAR));
		this->cameraViewport->queue_draw();
	}
}

bool MainWindow::textBufferUpdate(textBufferUpdateStruct data)
{
	data.buffer->set_text(data.text);
	return true;
}

void MainWindow::updateWeather(weatherStruct data)
{
	weather = data;
	weatherIndicator->queue_draw();
	stringstream ss;
	ss << setprecision(3)
		 << "Temperature: " << weather.temperature << "°C\n"
		 << "Wind speed: " << weather.windSpeed << "%\n"
		 << "Wind direction: " << weather.windDirection << "°\n"
		 << "Condition: " << weather.condition;

	g_idle_add(G_SOURCE_FUNC(textBufferUpdate), new textBufferUpdateStruct(weatherInfoBuffer, ss.str()));
}

void MainWindow::updateTelemetry(pTeleGen data, mutex* dataMutex)
{
	stringstream ss;
	{
		lock_guard<mutex> m(*dataMutex);
		ss << setprecision(5)
			 << "-----------------------\n"
			 << "\nYaw x:   " << data.att.yaw
			 << "\nPitch y: " << data.att.pitch
			 << "\nRoll y:  " << data.att.roll
			 << "\n-----------------------\n"
			 << "\nVoltage: " << data.batt.getVoltage
			 << "\nCurrent: " << data.batt.getCurrent
			 << "\nTemp:    " << data.att.temp
			 << "\n-----------------------\n"
			 << "\nGPS NOS: " << data.gps.numberOfSatelites
			 << "\nLat:     " << data.gps.latitude
			 << "\nLot:     " << data.gps.longitude
			 << "\nG speed: " << data.gps.groundSpeed;

		this->pitch = data.att.pitch;
		this->roll = data.att.roll;
		this->speed = data.gps.groundSpeed;
		if (!trackFlightPath)
			osm_gps_map_gps_clear(map);
		osm_gps_map_gps_add(map, data.gps.latitude, data.gps.longitude, data.gps.heading);
	}

	indicator->queue_draw();
	g_idle_add(G_SOURCE_FUNC(textBufferUpdate), new textBufferUpdateStruct(telemetryFieldBuffer, ss.str()));
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

void MainWindow::drawWeatherIndicator(GtkWidget* widget, cairo_t* cr, gpointer data)
{
	guint width, height;
	GtkStyleContext* context;

	context = gtk_widget_get_style_context(widget);

	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);

	gtk_render_background(context, cr, 0, 0, width, height);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface(cr, imgWHBack, 0, 0);
	cairo_paint(cr);
	cairo_translate(cr, 50, 50);
	cairo_rotate(cr, ((float)weather.windDirection) / 180 * M_PI);
	cairo_translate(cr, -50, -50);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface(cr, imgWHHand, 0, 0);
	cairo_paint(cr);

	cairo_fill(cr);
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
	if (speed < 20)
		angle += 3.6 * speed;
	else if (speed < 40)
		angle += 74 + 4 * (speed - 20.0);
	else if (speed < 60)
		angle += 154.0 + 3.8 * (speed - 40.0);
	else
		angle += 230.0 + 4 * (speed - 60.0);

	cairo_translate(cr, 120, 120);
	cairo_rotate(cr, angle / 180 * M_PI);
	cairo_translate(cr, -120, -120);

	cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
	cairo_set_source_surface(cr, imgASIHand, 0, 0);
	cairo_paint(cr);

	cairo_fill(cr);
}
void MainWindow::drawHorizonTailSituationIndicator(GtkWidget* widget, cairo_t* cr, gpointer data)
{
}

void MainWindow::toggleMap()
{
	if (showingMap) {
		showingMap = false;
		gtk_widget_hide(GTK_WIDGET(map));
	} else {
		showingMap = true;
		gtk_widget_show_all(GTK_WIDGET(map));
	}
}

void MainWindow::toggleTracking()
{
	trackFlightPath = !trackFlightPath;
}

void MainWindow::switchIndicator(bool back)
{
	if (back) {
		if (currentIndicator == 0) {
			currentIndicator = NUMBER_OF_INDICATORS - 1;
		} else {
			currentIndicator--;
		}
	} else {
		if (currentIndicator == NUMBER_OF_INDICATORS - 1) {
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
		if ((std::chrono::steady_clock::now() - lastChange).count() < 200000000)
			return 0;
		lastChange = std::chrono::steady_clock::now();
		if (x == LOW_CONTROLLER_AXIS_VALUE)
			mainWindow->switchIndicator(true);
		if (x == MAX_CONTROLLER_AXIS_VALUE)
			mainWindow->switchIndicator(false);
		if (y == LOW_CONTROLLER_AXIS_VALUE)
			mainWindow->toggleMap();
		if (y == MAX_CONTROLLER_AXIS_VALUE)
			mainWindow->toggleTracking();
	} break;
	default:
		break;
	}
	return 0;
}
