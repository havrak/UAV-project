/*
 * airspace_info_provider.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef AIRMAP_PROVIDER_H
#define AIRMAP_PROVIDER_H

#include "drone_telemetry.h"
#include <cpr.h>
#include <iostream>
#include "api_keys.h"
#include <thread>

/**
 * intefaces with Airmap.com via its REST API
 * gets information about flight zone, etc.
 *
 * NOTE: consider whether it should be singleton
 */
class AirmapProvider{
	private:
		static AirmapProvider* airmapProvider;
		static mutex airmapProviderMutex;
		thread updateInfoThread;
		bool update = true;
		char* APIurl =  "https://api.airmap.com/airspace/v2/";
		double searchRadius = 0.00017; // cca 200meters

		AirmapProvider();
		void updateInfo();

	public:
		/**
		 * main method used to access AirmapProvider
		 * if instace isn't created it will create it
		 */
		static AirmapProvider* GetInstance();


		/**
		 * udpdates information about airspace (formed by drawing dodecagon) around the drone
		 * size of search airspace is dictated by searchRadius
		 */
		void getFlightZoneInfo();



};

#endif /* !AIRSPACE_INFO_PROVIDER_H */
