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

#define UI_SIZE 240

/**
 * wrapper to store pointer to TexBuffer and message to be
 * set, used to schedule update
 *
 */
struct textBufferUpdate {
	Glib::RefPtr<Gtk::TextBuffer> buffer;
	string text;
	textBufferUpdate(Glib::RefPtr<Gtk::TextBuffer> buffer, string text)
			: buffer(buffer)
			, text(text) {};
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


	private:
	inline static cairo_surface_t* imgBack;
	inline static cairo_surface_t* imgFace;
	inline static cairo_surface_t* imgRing;
	inline static cairo_surface_t* imgCase;

	mutex attitudeValuesMutex;
	inline static float pitch = 10;
	inline static float roll = 10;

	/**
	 * Initializes attitude indicator
	 *
	 */
	void initAttitudeIndicator();


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
	static bool updateOnScreenTelemetry(textBufferUpdate telmetryBufferUpdate); // we will be passing pointer of this function, thus it needs to be statis

	/**
	 * updates attitude indicator
	 * used as a callback for draw method on DrawingArea
	 */
	static void drawIndicator(GtkWidget *widget, cairo_t *cr, gpointer data);

	/**
	 * rotates buffer
	 *
	 * @param Glib::RefPtr<Gdk::Pixbuf> - buffer to be rotated
	 * @param double angle - angle to rotate Pixbuf with
	 */
	/* Glib::RefPtr<Gdk::Pixbuf>  rotatePixbuf(Glib::RefPtr<Gdk::Pixbuf> pixbuf, double angle); */

	bool paused;

	// bool process = true;
};

extern std::mutex imageMutex;
extern Glib::Dispatcher dispatcher;
extern volatile bool captureVideoFromCamera;
extern cv::Mat frameBGR, frame, frameCorrected;
extern MainWindow* mainWindow;
extern bool cameraInitialized;
extern cv::Size imageSize;

#endif // MAINWINDOW_H_
