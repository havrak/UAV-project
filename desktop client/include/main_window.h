/*
 * main_window.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */


#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include "gtkmm/textbuffer.h"
#include "gtkmm/textview.h"
#include "protocol_spec.h"
#include <gtkmm.h>
#include <gdk/gdk.h>
#include <opencv2/opencv.hpp>
#include <mutex>
#include <thread>

bool setupCamera();
void cameraLoop();
bool initializeCamera();


struct textBufferUpdate {
	Glib::RefPtr<Gtk::TextBuffer> buffer;
    string text;
		textBufferUpdate(Glib::RefPtr<Gtk::TextBuffer> buffer, string text): buffer(buffer), text(text){};
};



class MainWindow : public Gtk::Window
{

public:

	MainWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
	virtual ~MainWindow();
	void stopCamera();
	void pauseResumeCamera();
	void updateImage(cv::Mat & frame);

	bool isPaused() {
		return this->paused;
	}
	Gtk::ToggleButton *resumePauseButton;
	Gtk::TextView *telemetryField;
	void updateData(pTeleGen data, mutex *dataMutex);
	void displayError(pTeleErr error);

private:

	Glib::RefPtr<Gtk::Builder> builder;
	Gtk::Button *closeButton;
	Gtk::Image *artHorizon;
	Gtk::Image *drawingImage;
	Glib::RefPtr<Gtk::TextBuffer> textBuffer;
	GtkTextBuffer *telemetryBuffer;
	static bool updateTextField(textBufferUpdate update); // we will be passing pointer of this function, thus it needs to be statis

	bool paused;
	//bool process = true;

};

extern std::mutex imageMutex;
extern Glib::Dispatcher dispatcher;
extern volatile bool captureVideoFromCamera;
extern cv::VideoCapture camera;
extern cv::Mat frameBGR, frame, frameCorrected;
extern MainWindow *mainWindow;
extern bool cameraInitialized;
extern std::thread cameraThread;
extern cv::Size imageSize;


#endif // MAINWINDOW_H_
