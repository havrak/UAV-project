#include "camera_image_grabber.h"
#include <opencv2/videoio.hpp>

using namespace std;

CameraGrabberWindow::CameraGrabberWindow(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
		: Gtk::Window(cobject)
		, builder(refGlade)
{

	this->paused = false;
	this->builder->get_widget("DrawingImage", this->drawingImage);
	this->builder->get_widget("closeButton", this->closeButton);
	this->builder->get_widget("resumePauseButton", this->resumePauseButton);
	this->closeButton->signal_clicked().connect(sigc::mem_fun(*this, &CameraGrabberWindow::stopCamera));
	this->resumePauseButton->signal_clicked().connect(sigc::mem_fun(*this, &CameraGrabberWindow::pauseResumeCamera));

	this->drawingImage->set("../images/image_not_found.png");
}

CameraGrabberWindow::~CameraGrabberWindow()
{
}

void CameraGrabberWindow::pauseResumeCamera()
{
	this->paused = !this->paused;
	if (this->paused) {
		this->resumePauseButton->set_label("resume");
	} else {
		this->resumePauseButton->set_label("pause");
	}
}

void CameraGrabberWindow::stopCamera()
{
	Window::close();
}

void CameraGrabberWindow::updateImage(cv::Mat& frame)
{
	if (!frame.empty()) {
		this->drawingImage->set(Gdk::Pixbuf::create_from_data(frame.data, Gdk::COLORSPACE_RGB, false, 8, frame.cols, frame.rows, frame.step));
		this->drawingImage->queue_draw();
	}
}

void cameraLoop()
{

	while (captureVideoFromCamera) {
		bool continueToGrabe = true;
		bool paused = cameraGrabberWindow->isPaused();
		if (!paused) {
			continueToGrabe = camera.read(frameBGR);
			if (continueToGrabe) {
				imageMutex.lock();
				cv::cvtColor(frameBGR, frame, cv::COLOR_RGB2BGR);
				imageMutex.unlock();
				dispatcher.emit();
			}
		}
		if (!continueToGrabe) {
			captureVideoFromCamera = false;
			std::cerr << "Faleid to retrieve frame from the device. The camera was stopped." << std::endl;
		} else if (paused) {
			//yields to avoid other threads to starve
			std::this_thread::sleep_for(std::chrono::milliseconds(30));
		}
	}
}

bool initializeCamera() {

	bool result = camera.open("udpsrc port=5000 ! application/x-rtp,media=video,payload=26,clock-rate=90000,encoding-name=JPEG,framerate=30/1 ! rtpjpegdepay ! jpegdec ! videoconvert ! appsink", cv::CAP_GSTREAMER);

	if(result) {
		for(int i = 0; i < 3; i++) {
			camera.grab();
		}
		//check decode valid image
		for(int i = 0; result && i < 3; i++) {
			result = result && camera.read(frameBGR);
		}
	}

	return result;
}
