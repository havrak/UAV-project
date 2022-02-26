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

#include "../libraries/rpidmx512-Lib-PCA9685/pca9685servo.h"
#include <bcm2835.h>

#include <chrono>
#include <mutex>
#include <string>
#include <thread>

using namespace std;

enum wingSurfaceConfiguration{
	V_SHAPE_TAIL_WING = 1,
	STANDARD_TAIL_WING = 2,
};

class ServoControl {
private:
  int fd = -1;
  const bool debug = true;
  static ServoControl *servoControl;
  static mutex mutexServoControl;
	thread pidControllerThread;

	const wingSurfaceConfiguration configuration = V_SHAPE_TAIL_WING;
	bool pca9685Up = false;
	unsigned int short mainMotorMS;

	PCA9685Servo servo;
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

	bool adjustMainMotorSpeed(pConStr ps);
	void setAngleOfServo(int channel, bool right, unsigned char angle);


	// PID controller
	bool pidOn = false;
	bool pidController();


protected:
  ServoControl();

public:
  static ServoControl *GetInstance();
  bool calibrateESC();
	bool armESC();
	void slowDownToMin();
	void testServo();
	bool getPCA9865Status();
	bool togglePIDController();
	unsigned int short getMainMotorMS();
	pair<int,unsigned int short*> getControlSurfaceConfiguration();
	int processControl(ProcessingStructure ps);
	int processMovementForVTail(pConStr ps);
	int processMovementForStandart(pConStr  ps);
};


#endif /* !SERVOCONTROL_H */
