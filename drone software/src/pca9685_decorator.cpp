/*
 * pca9685_decorator.cpp
 * Copyright (C) 2021 havra <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "pca9685_decorator.h"

PCA9685Decorator* PCA9685Decorator::servoControl = nullptr;
mutex PCA9685Decorator::mutexPCA9685Decorator;

PCA9685Decorator::PCA9685Decorator()
{

}

PCA9685Decorator* PCA9685Decorator::getInstance()
{

	if (servoControl == nullptr) {
		mutexPCA9685Decorator.lock();
		if (servoControl == nullptr)
			servoControl = new PCA9685Decorator();
		mutexPCA9685Decorator.unlock();
	}

	return servoControl;
}


bool PCA9685Decorator::attachPCA9685(int address){
	servo = PCA9685Servo(address);

	servo.SetFrequency(60);
	servo.SetLeftUs(MIN_PULSE_LENGTH);
	servo.SetCenterUs(CEN_PULSE_LENGTH);
	servo.SetRightUs(MAX_PULSE_LENGTH);
	servo.SetInvert(false);
	/* armESC(); */

	servo.SetAngle(CHANNEL(2), ANGLE(135));
	servo.SetAngle(CHANNEL(3), ANGLE(135));
	servo.SetAngle(CHANNEL(4), ANGLE(135));
	servo.SetAngle(CHANNEL(5), ANGLE(135));

	/* if (debug) */
	std::cout << "SERVOCONTROL | PCA9685Decorator | servos setted up, ESC armed\n";

}

void PCA9685Decorator::setPichAndRoll(float pitch, float roll)
{
	this->pitch = pitch;
	this->roll = roll;
}

void PCA9685Decorator::setAngleOfServo(int channel, bool right, unsigned char angle)
{
	if (right) {
		servo.SetAngle(channel, 90 + angle);
	} else {
		servo.SetAngle(channel, 180 - angle);
	}
}

bool PCA9685Decorator::calibrateESC()
{
	int b;
	std::cout << "SERVOCONTROL | calibrateESC | setting max\n";
	servo.Set(CHANNEL(0), servo.GetRightUs());
	mainMotorMS = servo.GetRightUs();
	std::cout << "SERVOCONTROL | calibrateESC | press key to set min\n";
	cin >> b;

	std::cout << "SERVOCONTROL | calibrateESC | setting min\n";
	servo.Set(CHANNEL(0), servo.GetLeftUs());
	mainMotorMS = servo.GetLeftUs();
	nanosleep((const struct timespec[]) { { 8, 0L } }, NULL);

	return true;
}

bool PCA9685Decorator::armESC()
{
	std::cout << "SERVOCONTROL | armESC | arming ESC\n";
	slowDownToMin();
	nanosleep((const struct timespec[]) { { 8, 0L } }, NULL);
	return true;
}

void PCA9685Decorator::slowDownToMin()
{
	mainMotorMS = MIN_PULSE_LENGTH;
	servo.Set(CHANNEL(0), mainMotorMS);
}


bool PCA9685Decorator::getPCA9865Status()
{
	return pca9685Up;
}
void PCA9685Decorator::setPCA9865Status(bool status)
{
	pca9685Up= status;
}


pair<int, unsigned char*> PCA9685Decorator::getControlSurfaceConfiguration()
{
	pair<int, unsigned char*> toReturn;
	toReturn.second = new unsigned char[16];
	toReturn.second[vTail.leftFlapIndex] = vTail.leftFlap;
	toReturn.second[vTail.rightFlapIndex] = vTail.leftFlap;
	toReturn.second[vTail.leftRuddervatorIndex] = vTail.leftRuddervator;
	toReturn.second[vTail.leftRuddervatorIndex] = vTail.rightRuddervator;
	return toReturn;
}

bool PCA9685Decorator::adjustMainMotorSpeed(pConStr ps)
{
	if (ps.lTrigger < 2500)
		ps.lTrigger = 0;
	if (ps.rTrigger < 2500)
		ps.rTrigger = 0;
	if (ps.lTrigger > 63000)
		ps.lTrigger = MAX_CONTROLLER_AXIS_VALUE;
	if (ps.rTrigger > 63000)
		ps.rTrigger = MAX_CONTROLLER_AXIS_VALUE;
	int valL = (ps.lTrigger << 10) >> 10;
	int valR = (ps.rTrigger << 10) >> 10;

	if (valL < valR) {
		// accelerate
		if (mainMotorMS + log(valR - valL) * SERVO_ACCELERATION_MULTIPLIER <= MAX_MOTOR_PULSE_LENGTH) {
			mainMotorMS += log(valR - valL) * SERVO_ACCELERATION_MULTIPLIER;
			servo.Set(CHANNEL(0), mainMotorMS);
			/* std::cout << "SERVO_CONTROL | processMovementForVTail | accelerating: " << mainMotorMS << "\n"; */
		}
	} else {
		// decelerate
		if (mainMotorMS - log(valL - valR) * SERVO_ACCELERATION_MULTIPLIER >= MIN_PULSE_LENGTH) {
			mainMotorMS -= log(valL - valR) * SERVO_ACCELERATION_MULTIPLIER;
			if (mainMotorMS - 80 > MIN_PULSE_LENGTH)
				mainMotorMS = MIN_PULSE_LENGTH;
			/* std::cout << "SERVO_CONTROL | processMovementForVTail | decelerating: " << mainMotorMS << "\n"; */
			servo.Set(CHANNEL(0), mainMotorMS);
		}
	}

	return true;
}

float scalerX, scalerY;
float tmpX, tmpY;
int yaw, pitch;

bool PCA9685Decorator::processMovementForVTail(pConStr ps)
{
	return true;
}

bool PCA9685Decorator::processMovementForStandart(pConStr ps)
{
	return true;
}

bool PCA9685Decorator::processControl(ProcessingStructure ps)
{
	pConStr control;
	memcpy(&control, ps.getMessageBuffer(), sizeof(control));
	if (pidOn) {
		adjustMainMotorSpeed(control);
		return 0;
	}


	switch (controllAdjuster) {

	case TRIGONOMETRIC: {
		float tmp;
		if (control.lAnalog.first == 0) {
			tmp = atan(control.lAnalog.second / control.rAnalog.first);
			if (tmp < 1.047) // we don't want to decrease value
				control.lAnalog.first *= cos(tmp) * 2;
			if (tmp > 0.524)
				control.lAnalog.second *= sin(tmp) * 2;
		}
	}
	case SQUARING: {
		tmpX = control.lAnalog.first / MAX_CONTROLLER_AXIS_VALUE;
		tmpY = control.lAnalog.second / MAX_CONTROLLER_AXIS_VALUE;
		scalerX = tmpX * sqrt(1 - tmpY * tmpY / 2);
		scalerY = tmpY * sqrt(1 - tmpX * tmpX / 2);
		control.lAnalog.first = MAX_CONTROLLER_AXIS_VALUE * scalerX;
		control.lAnalog.second = MAX_CONTROLLER_AXIS_VALUE * scalerY;
	} break;
	}

	if (control.lAnalog.first == 0)
		roll = 0;
	else
		roll = ((float)control.lAnalog.first / MAX_CONTROLLER_AXIS_VALUE) * 90;
	if (control.lAnalog.second == 0)
		pitch = 0;
	else
		pitch = ((float)control.lAnalog.second / MAX_CONTROLLER_AXIS_VALUE) * 90;

	adjustMainMotorSpeed(ps);
	vTail.leftRuddervator = (roll + pitch) * MIXING_GAIN;
	vTail.rightRuddervator = (90 - roll + pitch) * MIXING_GAIN;
	setAngleOfServo(vTail.leftRuddervatorIndex, false, vTail.leftRuddervator);
	setAngleOfServo(vTail.rightRuddervatorIndex, true, vTail.rightRuddervator);
	setAngleOfServo(vTail.leftFlapIndex, false, roll);
	setAngleOfServo(vTail.rightFlapIndex, true, (90 - roll));
	std::cout << "Control processed\n";
	return true;
}

unsigned int short PCA9685Decorator::getMainMotorMS()
{
	return mainMotorMS;
}

