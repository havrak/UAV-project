// vim: set ft=arduino:
/*
 * sensor.h
 * Copyright (C) 2022 Kryštof Havránek <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef SENSOR_H
#define SENSOR_H
#include "sensor_metadata.h"
#include "callback_interface.h"
#include "tasker.h"
#include "tasker_function_ids.h"

#define MAX_NO_TASK_PER_SENSOR 4

class Sensor: public CallbackInterface{
  protected:
    bool active = true;
    bool first = true;
    bool error = true;
    bool newData = true;
		uint8_t activeTasksCount = 0;
		uint16_t activeTasks[MAX_NO_TASK_PER_SENSOR] = {0};

    uint8_t multiplexerPosition = 64;
    uint8_t i2cAddress;

    uint8_t* peripheryAddresses;
    uint8_t* peripherySubaddresses;

    uint8_t inputsTracked = 1;
    bool subaddressUsed = false;
    uint16_t scanFrequency;

  public:

    virtual ~Sensor(){};
    Sensor(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t peripheryAddress){ // single periphery number
      if(multiplexerPosition < 8) this->multiplexerPosition = multiplexerPosition;
      inputsTracked = 1;
      this->i2cAddress = i2cAddress;
      this->peripheryAddresses = new uint8_t[1];
      this->peripheryAddresses[0] = peripheryAddress;
      this->subaddressUsed = false;
    }

    Sensor(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t* peripheryAddresses, uint8_t inputsTracked){ // multiple periphery numbers
      if(multiplexerPosition < 8) this->multiplexerPosition = multiplexerPosition;
      this->inputsTracked = inputsTracked;
      this->i2cAddress = i2cAddress;
      this->inputsTracked = inputsTracked;
      this->peripheryAddresses = new uint8_t[inputsTracked];
      memcpy(&(this->peripheryAddresses), &peripheryAddresses, inputsTracked);
      this->subaddressUsed = false;
    }

    Sensor(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t peripheryAddress, uint8_t peripherySubaddress){ // single periphery number, single subperiphery number
      if(multiplexerPosition < 8) this->multiplexerPosition = multiplexerPosition;
      inputsTracked = 1;
      this->i2cAddress = i2cAddress;
      this->peripheryAddresses = new uint8_t[1];
      this->peripheryAddresses[0] = peripheryAddress;
      this->peripherySubaddresses = new uint8_t[1];
      this->peripherySubaddresses[0] = peripherySubaddress;
      this->subaddressUsed = true;
    }

    Sensor(uint8_t multiplexerPosition, uint8_t i2cAddress, uint8_t peripheryAddresses, uint8_t* peripherySubaddresses, uint8_t inputsTracked){ // single periphery number, multiple periphery numbers
      if(multiplexerPosition < 8) this->multiplexerPosition = multiplexerPosition;
      this->inputsTracked = inputsTracked;
      this->i2cAddress = i2cAddress;
      this->peripheryAddresses = new uint8_t[1];
      this->peripheryAddresses[0] = peripheryAddresses;
      this->peripherySubaddresses = new uint8_t[inputsTracked];
      memcpy(&(this->peripheryAddresses), &peripherySubaddresses, inputsTracked);
      this->subaddressUsed = true;
    }

    virtual bool read(){return 0;}
    virtual bool initialize(){return 0;}
		virtual bool addTasks(){return 0;}

    virtual uint8_t type() const {
      return CLASS_TYPE_SENSOR;
    }


    bool isActive(){
      return active;
    }

    virtual void setActive(bool active){
      this->active = active;
    }


    bool getError(){
      return error;
    }

    uint8_t getI2CAddress(){
      return i2cAddress;
    }
    uint8_t getMultiplexerPosition(){
      return multiplexerPosition;
    }

    bool setError(bool error){
      if(!error && this->error){
        initialize();
        return true;
      }
      this->error = error;
      return false;
    }

    bool getNewDataFlag(){
      return newData;
    }

    void clearNewDataFlag(){
      newData = false;
    }

		uint16_t getActiveTask(uint8_t index){
				return activeTasks[index];
		}

		uint16_t getActiveTasksCount(){
				return activeTasksCount;
		}

};

#endif /* !SENSOR_H */
