/*
 * peripherials_manager.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "peripherials_manager.h"
#include "communication_interface.h"

PeripherialsManager* PeripherialsManager::telemetry = nullptr;
mutex PeripherialsManager::peripherialsMutex;

PeripherialsManager::PeripherialsManager()
{
}

PeripherialsManager* PeripherialsManager::GetInstance()
{
	if (telemetry == nullptr)
		telemetry = new PeripherialsManager();
	return telemetry;
}

void PeripherialsManager::telemetryThreadMethod(){
	while(true){
		peripherialsMutex.lock();
		wt901b->read();
		ubloxGPS->read();
		vaBattery->read();
		vaInternals->read();
		peripherialsMutex.unlock();
		this_thread::sleep_for(chrono::milliseconds(50));
	}
}

bool PeripherialsManager::initializePeripherials(uint8_t inaBatAddress, uint8_t inaPowerAddress, uint8_t pcaAddress, uint8_t imuAddress)
{
	cout << "PeripherialsManager | initializePeripherials | initializing peripherials: ";
	cout << "ina battery, ";
	vaBattery = new INA226Decorator(inaBatAddress);
	cout << "ina power, ";
	vaInternals = new INA226Decorator(inaPowerAddress);
	cout << "pca9685, ";
	pca9685 = new PCA9685Decorator(pcaAddress);
	cout << "wt901b, ";
	wt901b = new WT901Decorator(imuAddress, IMU_Orientation::X_Y_INVERTED);
	cout << "ublox gps" << endl;
	ubloxGPS = new UBloxGPSDecorator();

	telemetryThread = thread(&PeripherialsManager::telemetryThreadMethod, this);
	return true;
}

bool PeripherialsManager::i2cScan()
{
	lock_guard<mutex> lock(peripherialsMutex);

	char buffer[128];
	std::string result = "";
	FILE* pipe = popen("echo $(i2cdetect -y 1 2>/dev/null | tail -7 | cut -d':' -f2 | sed 's/[^0-9]*\\(.\\)/\\1/g')", "r");
	if (!pipe) {
		cerr << "PeripherialsManager | checkSensorStatus | popen() failed!\n";
		return false;
	}
	try {
		while (fgets(buffer, sizeof buffer, pipe) != NULL) {
			result += buffer;
		}
	} catch (...) {
		pclose(pipe);
		cerr << "PeripherialsManager | checkSensorStatus | reading failed!\n";
		return false;
	}
	std::map<uint8_t, bool> addressesMap;
	// 40, 50, 44
	addressesMap[((I2CPeriphery*) pca9685)->getI2CBusAddress()] = false;
	addressesMap[((I2CPeriphery*) wt901b)->getI2CBusAddress()] = false;
	addressesMap[((I2CPeriphery*) vaBattery)->getI2CBusAddress()] = false;
	addressesMap[((I2CPeriphery*) vaInternals)->getI2CBusAddress()] = false;


	uint8_t tmp;

	for (int i = 0; i < result.length() && i+2 < result.length(); i += 3) {
		tmp = stoul("0x" + result.substr(i, i + 2), nullptr, 16);
		if (addressesMap.find(tmp) != addressesMap.end())
			addressesMap[tmp] = true;

	}
	pclose(pipe);

	pca9685->setError(!addressesMap[((I2CPeriphery*) pca9685)->getI2CBusAddress()]);
	wt901b->setError(!addressesMap[((I2CPeriphery*) wt901b)->getI2CBusAddress()]);
	vaBattery->setError(!addressesMap[((I2CPeriphery*) vaBattery)->getI2CBusAddress()]);
	vaInternals->setError(!addressesMap[((I2CPeriphery*) vaInternals)->getI2CBusAddress()]);
	return true;
}

pTeleATT PeripherialsManager::createTeleAttStruct()
{
	pTeleATT toReturn(wt901b->getYaw(), wt901b->getPitch(), wt901b->getRoll(), wt901b->getAccX(), wt901b->getAccY(), wt901b->getAccZ(), wt901b->getGyroX(), wt901b->getGyroY(), wt901b->getGyroZ(), wt901b->getMagX(), wt901b->getMagY(), wt901b->getMagZ(), wt901b->getPressure(), wt901b->getTemp());
	return toReturn;
}

pTeleGPS PeripherialsManager::createTeleGPSStruct()
{
	pTeleGPS toReturn(ubloxGPS->getError(), ubloxGPS->getAltitude(), ubloxGPS->getLon(), ubloxGPS->getLat(), ubloxGPS->getGroundSpeed(), ubloxGPS->getHeading(), ubloxGPS->getNOS());
	return toReturn;
}

pTelePOW PeripherialsManager::createTelePOWStuct()
{
	pTelePOW toReturn(vaBattery->getVoltage(), vaBattery->getCurrent(), vaBattery->getPower(), vaInternals->getVoltage(), vaInternals->getCurrent(), vaInternals->getCurrent());
	return toReturn;
}

pTeleIOStat PeripherialsManager::createTeleIOStatStruct()
{
	pTeleIOStat toReturn(!vaBattery->getError(), !vaInternals->getError(), !pca9685->getError(), !wt901b->getError(), ubloxGPS->getError());
	return toReturn;
}

pTelePWM PeripherialsManager::createTelePWMStruct() // TODO: What a horrendous way to do it
{
	pTelePWM toReturn;
	toReturn.motorMS = pca9685->getMainMotorMS();
	pair<int, unsigned char*> tmp = pca9685->getControlSurfaceConfiguration();
	toReturn.configuration = tmp.first;
	memcpy(&toReturn.angle, &tmp.second, 16);
	delete[] tmp.second;
	return toReturn;
}

bool PeripherialsManager::processGeneralTelemetryRequest(const client cli)
{
	pTeleGen data;
	data.att = createTeleAttStruct();
	data.gps = createTeleGPSStruct();
	data.pow = createTelePOWStuct();
	data.io = createTeleIOStatStruct();
	data.pwm = createTelePWMStruct();
	/* cout << "yaw:   " << data.att.yaw << "\npitch: " << data.att.pitch << "\nroll:  " << data.att.roll << "\nacc:   " << data.att.accX << "\ntemp:  " << data.att.temp << "\nvolt:  " << data.batt.voltage << "\ncurr:  " << data.batt.current << "\n\n"; */

	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_GEN, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}

bool PeripherialsManager::processAttGPSRequest(const client cli)
{
	pTeleATTGPS data;
	data.att = createTeleAttStruct();
	data.gps = createTeleGPSStruct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_ATTGPS, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}
bool PeripherialsManager::processPowerRequest(const client cli)
{
	pTelePOW data = createTelePOWStuct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_POW, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}

bool PeripherialsManager::processPWMRequest(const client cli)
{
	pTelePWM data = createTelePWMStruct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_PWM, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}
bool PeripherialsManager::processIORequest(const client cli)
{
	pTeleIOStat data = createTeleIOStatStruct();
	SendingStructure ss(cli.fd, cli.cMutex, P_TELE_IOSTAT, 0x01, sizeof(data));
	memcpy(ss.getMessageBuffer(), &data, sizeof(data));
	SendingThreadPool::GetInstance()->scheduleToSend(ss);
	return true;
}
