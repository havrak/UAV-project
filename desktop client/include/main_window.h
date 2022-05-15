/*
 * main_window.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include "gdkmm/pixbuf.h"
#include "glibmm/refptr.h"
#include "gtkmm/textbuffer.h"
#include "gtkmm/textview.h"
#include "protocol_spec.h"
#include <cairomm/refptr.h>
#include <cairomm/surface.h>
#include <gdk/gdk.h>
#include <gtkmm.h>
#include <mutex>
#include <opencv2/opencv.hpp>
#include <thread>
#include "control_interpreter.h"

#define UI_SIZE 240
#define NUMBER_OF_INDICATORS 3
#define LOW_CONTROLLER_AXIS_VALUE 0
#define MID_CONTROLLER_AXIS_VALUE 32767
#define MAX_CONTROLLER_AXIS_VALUE 65534


/**
 * wrapper to store information regarding onscreen elements that
 * need to be updated
 *
 * used by method updateOnScreenTelemetry called by gtk
 */
struct onScreenTelemetryUpdate {
	Glib::RefPtr<Gtk::TextBuffer> buffer;
	Glib::RefPtr<Gtk::DrawingArea> indicator;
	string text;
	onScreenTelemetryUpdate(Glib::RefPtr<Gtk::TextBuffer> buffer, Glib::RefPtr<Gtk::DrawingArea> indicator, string text)
			: buffer(buffer)
			, indicator(indicator)
			, text(text) {};
};


/**
 * enum declaring all indicators which can be displayed
 * in the GUI
 */
enum indicatorTypes{
 ATTITUDE_INDICATOR = 0, AIRSPEED_INDICATOR = 1, HORIZONTAIL_SITUATION_INDICATOR = 2
};



/**
 * Child class extending Gtk::Window
 * takes care of graphics
 */
class MainWindow : public Gtk::Window {

	public:


	MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	virtual ~MainWindow();

	/**
	 * Closes application
	 *
	 */
	void closeWindow();

	/**
	 * Pauses processing of new frames,
	 *
	 */

	void pauseResumeCamera();

	/**
	 * Updates image with new frame
	 *
	 * @param cv::Mat& frame - frame to be set
	 */
	void updateImage(cv::Mat& frame);

	/**
	 * Callback to reset server on Raspberry Pi
	 *
	 */
	void restartServer();

	bool isPaused()
	{
		return this->paused;
	}

	Gtk::ToggleButton* resumePauseButton;
	Gtk::TextView* telemetryField;

	/**
	 * schedules update of data in telemetryBuffer and
	 * outdated Attitude Indicator
	 *
	 * @param pTeleGen data - struct with data to update buffer with
	 * @param mutex* dataMutex - mutex to lock struct with
	 */
	void updateData(pTeleGen data, mutex* dataMutex);

	void displayError(pTeleErr error);

	void switchIndicator(bool back);

	private:
	inline static cairo_surface_t* imgAIBack;
	inline static cairo_surface_t* imgAIFace;
	inline static cairo_surface_t* imgAIRing;
	inline static cairo_surface_t* imgAICase;
	inline static cairo_surface_t* imgASIFace;
	inline static cairo_surface_t* imgASIHand;


	mutex attitudeValuesMutex;
	inline static float pitch = 0;
	inline static float roll = 0;
	inline static float speed = 0;

	int currentIndicator = ATTITUDE_INDICATOR;

	/* protected: */
	Glib::RefPtr<Gtk::Builder> builder;
	Gtk::Button* closeButton;
	Gtk::Button* resartButton;
	/* Gtk::Image* indicator; */
	Gtk::DrawingArea* indicator;
	GtkDrawingArea* indicatorCObj;
	Gtk::Image* drawingImage;
	Glib::RefPtr<Gtk::TextBuffer> textBuffer;
	GtkTextBuffer* telemetryBuffer;

	/**
	 * updates buffer with telemetry
	 * call of function is scheduled by Gtk
	 *
	 * @param textBufferUpdate telmetryBufferUpdate - struct with buffer reference and buffer pointer
	 */
	static bool updateOnScreenTelemetry(onScreenTelemetryUpdate telmetryBufferUpdate); // we will be passing pointer of this function, thus it needs to be statis

	/**
	 * updates attitude indicator
	 * used as a callback for draw method on DrawingArea
	 *
	 * @param GtkWidget *widget - DrawingArea to be drawn in
	 * @param cairo_t *cr - corresponding cairo structure with given widget
	 * @param gpointer data - pointer to data
	 */
	static void drawIndicator(GtkWidget *widget, cairo_t *cr, gpointer data);


	/**
	 * draws graphic of attitude indicator
	 * called by drawIndicator
	 *
	 * @param GtkWidget *widget - DrawingArea to be drawn in
	 * @param cairo_t *cr - corresponding cairo structure with given widget
	 * @param gpointer data - pointer to data
	 */
	static void drawAttitudeIndicator(GtkWidget *widget, cairo_t *cr, gpointer data);


	/**
	 * draws graphic of airspeed indicator
	 * called by drawIndicator
	 *
	 * NOTE: currently graphic doesn't make a lot of sence
	 * though speed indicated is correct
	 *
	 * @param GtkWidget *widget - DrawingArea to be drawn in
	 * @param cairo_t *cr - corresponding cairo structure with given widget
	 * @param gpointer data - pointer to data
	 */
	static void drawAirspeedIndicator(GtkWidget *widget, cairo_t *cr, gpointer data);

	/**
	 * draws graphic of horizontal tail situation
	 * called by drawIndicator
	 *
	 * @param GtkWidget *widget - DrawingArea to be drawn in
	 * @param cairo_t *cr - corresponding cairo structure with given widget
	 * @param gpointer data - pointer to data
	 */
	static void drawHorizonTailSituationIndicator(GtkWidget *widget, cairo_t *cr, gpointer data);

	bool paused;

	// bool process = true;
};

class ControllerUIBridge : ControlInterpreter {
	private:

	public:
	std::chrono::steady_clock::time_point lastChange;

	ControllerUIBridge();

	/**
	 * update method called by Controller Interface
	 *
	 * @param ControlSurface cs - type of control surface for which callback is generated
	 * @param int x - value of X axis
	 * @param int y - value of Y axis
	 */
	int update(ControlSurface cs, int x, int y) override;

	/**
	 * update method called by Controller Interface
	 *
	 * @param ControlSurface cs - type of control surface for which callback is generated
	 * @param int val - value of button
	 */
	int update(ControlSurface cs, int val) override;


};

extern std::mutex imageMutex;
extern Glib::Dispatcher dispatcher;
extern cv::Mat frameBGR, frame, frameCorrected;
extern MainWindow* mainWindow;
extern cv::Size imageSize;

#endif // MAINWINDOW_H_
