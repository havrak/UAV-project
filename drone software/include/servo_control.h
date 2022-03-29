/*
 * servoControl.h
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include "protocol_spec.h"
#define MIXING_GAIN 0.5

#define MIN_PULSE_LENGTH 200 // Minimum pulse length in µs1
#define CEN_PULSE_LENGTH 1100 // Central pulse length in µs
#define MAX_PULSE_LENGTH 2000 // Maximum pulse length in µs
#define MAX_MOTOR_PULSE_LENGTH 1400

// NOTE: current max is over 1900
#define SERVO_ACCELERATION_MULTIPLIER 5
#define LOW_CONTROLLER_AXIS_VALUE 0
#define MID_CONTROLLER_AXIS_VALUE 32767
#define MAX_CONTROLLER_AXIS_VALUE 65534
#define LEVERAGE 20;

/* #include "../libraries/rpidmx512-Lib-PCA9685/pca9685servo.h" */
#include "../libraries/lib-pca9685/pca9685servo.h"

#include <chrono>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

/**
 * enum for configuration of plane
 */
enum WingSurfaceConfiguration{
	V_SHAPE_TAIL_WING = 1,
	STANDARD_TAIL_WING = 2,
};

enum ControlMethodAdjuster{
	TRIGONOMETRIC, SQUARING
};


/**
 * class handeling interfacing between
 * PCA9685 and rest of the system
 */
class ServoControl {
private:
  int fd = -1;
  const bool debug = true;
  static ServoControl *servoControl;
  static mutex mutexServoControl;
	thread pidControllerThread;
	float pitch = 0, roll = 0;

	 WingSurfaceConfiguration planeConfiguration = V_SHAPE_TAIL_WING;
	 ControlMethodAdjuster controllAdjuster = SQUARING;
	bool pca9685Up = false;
	unsigned int short mainMotorMS;

	PCA9685Servo servo = NULL;
	int currentMotorPulse;

	struct StandardTailWingConfiguration{
		int rightFlapIndex = 1;
		int leftFlapIndex = 2;
		int rightElevatorIndex = 3;
		int leftElevatorIndex = 4;
		int rudderIndex = 5;
		unsigned int short leftFlap;
		unsigned int short rightFlap;
		unsigned int short leftElevator;
		unsigned int short rightElevator;
		unsigned int short rudder;

	};
	struct VShapeTailWingConfiguration{
		int rightFlapIndex = 4;
		int leftFlapIndex = 5;
		int rightRuddervatorIndex = 2;
		int leftRuddervatorIndex = 3;
		unsigned int short leftFlap;
		unsigned int short rightFlap;
		unsigned int short leftRuddervator;
		unsigned int short rightRuddervator;
	};

	VShapeTailWingConfiguration vTail;
	StandardTailWingConfiguration standard;

	/**
	 * adjust control of main motor new
	 * value depends on position of left
	 * and right trigger
	 *
	 * @param pConStr ps - struture from which position of triggers is extracted
	 * @return bool - true if value was set successfully
	 */
	bool adjustMainMotorSpeed(pConStr ps);

	/**
   * sets angle of given servo
	 * servos on right side are reversed, thus value of angle
	 * needs to be flipped
	 *
	 * @param int channel - on which port is servo connected
	 * @param bool right - is servo on the right side of the plane
	 * @param unsigned char angle - desired angle of servo
	 */
	void setAngleOfServo(int channel, bool right, unsigned char angle);


	// PID controller
	bool pidOn = false;
	/**
	 * method run if pid controller is active
	 */
	bool pidController();


protected:
  ServoControl();

public:
	/**
	 * main method used to access ServoControl
	 * if instace wasn't created it will initialize
	 * ServoControl
	 */
  static ServoControl *GetInstance();

	/**
   * attaches PCA9685, runs initial configuration
	 *
	 */
	bool attachPCA9685(int address);

	/**
	 * calibrates ESC
	 * sets ms to min value and after while to max
	 * needs to be called only when using brand new ESC
	 *
	 * @return bool - true if calibration runned
	 */
  bool calibrateESC();

	/**
	 * armESC after boot, needs to be called in order to use it
	 *
	 * @return bool - true if ESC was armed
	 */
	bool armESC();

	/**
	 * slows down main motor to min speed
	 * necessary to be called before turing of the system
	 */
	void slowDownToMin();

	/**
	 * configures how controlls should be interpreted
	 *
	 * @param WingSurfaceConfiguration wsc - plane configuration
	 * @param ControlMethodAdjuster cma - method how controller input should be adjusted
	 */
	void setOperationalParameters(WingSurfaceConfiguration wsc, ControlMethodAdjuster cma);

	/**
	 * get status of PCA9685
	 *
	 * @return bool - true of PCA9685 is online
	 */
	bool getPCA9865Status();

	/**
	 * set status of PCA9685 as it is telemetry.h
	 * who checks i2c devices
	 *
	 * @param bool status - new status
	 */
	void setPCA9865Status(bool status);

	/**
	 * start/stops PID controller
	 * if PID is running acces standard
	 * controll will not be processed
	 */
	bool togglePIDController();

	unsigned int short getMainMotorMS();

	/**
	 * returns configuration of the plane alongside angle of all control surfaces
	 *
	 * @return<int, unsigned char*> - first value corresponds to configuration second to array of angles of controll surfaces
	 */
	pair<int,unsigned char*> getControlSurfaceConfiguration();

	/**
	 * processes new control from client
	 * calls corresponding method depending
	 * on configuration of the plane
	 *
	 * @param ProcessingStructure ps - strucure with new configuration
	 * @return int - 0 if processesing went well
	 */
	bool processControl(ProcessingStructure ps);

	/**
	 * interprets controller state for V-Tail
	 *
	 * @param pConStr ps - structure with controller state
	 * @return int - 0 if processesing went well
	 */
	bool processMovementForVTail(pConStr ps);

	/**
	 * interprets controller state for standard
	 * plane configuration
	 *
	 * @param pConStr ps - structure with controller state
	 * @return int - 0 if processesing went well
	 */
	bool processMovementForStandart(pConStr  ps);

	/**
	 * sets pitch and roll value used by PID
	 *
	 * @param float pitch - new pitch value
	 * @param float roll - new roll value
	 */
	void setPichAndRoll(float pitch, float roll);
};


#endif /* !SERVOCONTROL_H */
