/*
 * main_window.h
 * Copyright (C) 2021 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */


#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <gtkmm.h>
#include <opencv2/opencv.hpp>
#include <mutex>
#include <thread>

bool setupCamera();
void cameraLoop();
bool initializeCamera();

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

private:

	Glib::RefPtr<Gtk::Builder> builder;
	Gtk::Image *drawingImage;
	Gtk::Button *closeButton;

	bool paused;

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
