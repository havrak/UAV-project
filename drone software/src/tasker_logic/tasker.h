// vim: set ft=arduino:
/*
 * tasker.h
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef TASKER_H
#define TASKER_H
#include "../sleep_manager.h"
#include <Arduino.h>

#include <vector>
#include "callback_interface.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <sys/time.h>
#define MAX_HEAP_SIZE 201
#define MAX_HEAP_SIZE_HALF 100
#define MAX_NO_OUT_OFF_ORDER_TASK 10
#define MAX_RUNTIME_OUT_OFF_ORDER_TASK 10 // max runtime for out of order exectuion tasks
#define TASKER_DEBUG if(1)


class Task {

	private:
		uint64_t argc = 0;
		uint8_t* args = nullptr;

	public:
		CallbackArgsType ct = CallbackArgsType::NON_ACTIVE;
		CallbackInterface* owner;
		int64_t timeOfExecution =0;
		uint16_t expectedRunTime = 0;
		uint16_t id = 0;
		uint8_t priority  =0;

		/**
		 * generic constructor of empty function
		 * no code would be executed upon invoking
		 */
		Task():owner(nullptr), timeOfExecution(UINT64_MAX), expectedRunTime(0), priority(0), ct(CallbackArgsType::NON_ACTIVE)
		{
		}

		/**
		 * constructor for function without arguments
		 * will call simple call() function upon invoke
		 *
		 * @param CallbackInterface* owner - class whose method should be executed;
		 * @param uint16_t id - tasks in tasker have ids
		 * @param uint64_t executeIn - after how many ms should task be executed
		 * @param uint16_t expectedRunTime - how long do we expect function to run
		 * @param uint8_t priority - priority of task against other tasks
		 */
		Task(CallbackInterface* owner, uint16_t id, uint32_t executeIn, uint16_t expectedRunTime, uint8_t priority):owner(owner), id(id), timeOfExecution(millis()+executeIn), expectedRunTime(expectedRunTime), priority(priority), ct(CallbackArgsType::NO_ARGS)
		{
		}

		/**
		 * constructor for function with single uint64_t as argument
		 * will call simple call(uint64_t) function upon invoke
		 *
		 * @param CallbackInterface* owner - class whose method should be executed;
		 * @param uint16_t id - tasks in tasker have ids
		 * @param uint32_t executeIn - after how many ms should task be executed
		 * @param uint16_t expectedRunTime - how long do we expect function to run
		 * @param uint8_t priority - priority of task against other tasks
		 * @param uint64_t arg - argument with which function will be executed
		 */
		Task(CallbackInterface* owner, uint16_t id, uint32_t executeIn, uint16_t expectedRunTime, uint8_t priority, uint64_t arg) :owner(owner), id(id), timeOfExecution(millis()+executeIn), expectedRunTime(expectedRunTime), priority(priority), argc(arg), ct(CallbackArgsType::SINGLE_INPUT)
		{
		}

		/**
		 * constructor for function with byte buffer as its argument
		 * will call simple call(uint8_t*, uint64_t) function upon invoke
		 *
		 * @param CallbackInterface* owner - class whose method should be executed;
		 * @param uint16_t id - tasks in tasker have ids
		 * @param uint32_t executeIn - after how many ms should task be executed
		 * @param uint16_t expectedRunTime - how long do we expect function to run
		 * @param uint8_t priority - priority of task against other tasks
		 * @param uint8_t* args - pointer to byte array
		 * @param uint64_t argc - number of arguments
		 */
		Task(CallbackInterface* owner, uint16_t id, uint32_t executeIn, uint16_t expectedRunTime, uint8_t priority, uint8_t* args, uint64_t argc): owner(owner), id(id), timeOfExecution(millis()+executeIn), expectedRunTime(expectedRunTime), priority(priority), argc(argc), ct(CallbackArgsType::BYTE_ARRAY)
		{
			this->args = new uint8_t[argc];
			memcpy(this->args, args, argc);
		}

		~Task()
		{
			delete[] args;
		}

		/**
		 * calls one of call() function on owner object
		 *
		 * @return uint8_t - return value of function
		 */
		uint8_t invoke();
};

class Tasker {
	private:
		static Tasker* instance;
		std::unique_ptr<std::unique_ptr<Task>[]> heap;
		uint16_t heapSize = 0;
		bool active = true;

		Tasker();


		void printHeap();

	public:

		/**
		 * main method to access Tasker
		 * if instance of proxy wasn't yet created
		 * method will do the initialization
		 */
		static Tasker* getInstance();


		/**
		 * check if tasks at the top of the heap are ought to be
		 * invoked, if thats the case it will invoke them
		 *
		 * when there isn't any task to be called next 3000 ms  schedules sleep in 3 seconds
		 */
		void tick();

		/**
		 * adds new task to heap
		 *
		 * @param Task *f - pointer to Task class where tasks is setup
		 * @return bool - true if task was added
		 */
		bool addNewTask(Task* f);

		/**
		 * adds new task to heap if id of task is not present
		 *
		 * @param Task *f - pointer to Task class where tasks is setup
		 * @return bool - true if task was added
		 */
		bool addNewTaskId(Task* f);

		/**
		 * return number of tasks in heap
		 *
		 * @return uint16_t sizeOfHeap
		 */
		uint16_t getHeapSize();

		/**
		 * deletes task of given index from heap
		 *
		 * @param uint16_t index - index of task to be deleted
		 * @return bool - true if deletion was successful
		 */
		bool deleteTaskByIndex(uint16_t index);

		/**
		 * deletes first task of given id from heap
		 *
		 * @param uint16_t id - id of task to be deleted
		 * @return bool - true if deletion was successful
		 */
		bool deleteTaskById(uint16_t id);

		/**
		 * deletes all tasks with given id
		 *
		 * @param uint16_t id - id of tasks to be deleted
		 * @return bool - true if deletion was successful
		 */
		bool deleteTasksById(uint16_t id);

		/**
		 * retrieves first task from heap
		 * function is returned, task is deleted
		 *
		 * @return Task* - pointer to Task on top of the heap
		 */
		Task* pop();

		/**
		 * retrieves task on given index
		 * task is not removed from heap
		 *
		 * @param uint16_t index - index of task
		 * @return Task* - pointer to Task on given index
		 */
		Task* getTaskByIndex(uint16_t index);

		/**
		 * retrieves first task with given id
		 * task is not removed from heap
		 *
		 * @param uint16_t id - id of task
		 * @return Task* - pointer to Task on given index
		 */
		Task* getTaskById(uint16_t id);

		/**
		 * checks if task with given id is in the heap
		 *
		 * @param uint16_t id - id of task
		 * @return bool - true if present
		 */
		bool isIdPresent(uint16_t id);

		/**
		 * checks if task with given id is in the heap
		 *
		 * @param uint16_t id - id of task
		 * @return bool - true if present
		 */
		bool isIdPresentFromOwner(uint16_t id, CallbackInterface* owner);

		/**
		 * retrieves first task from heap
		 * task is not removed from heap
		 *
		 * @return Task* - pointer to Task on top of the heap
		 */
		Task* getHead();


		void setActive(bool active){
			this->active = active;
		}

		bool getActive(){
			return active;
		}
};

#endif /* !TASKER_H */
