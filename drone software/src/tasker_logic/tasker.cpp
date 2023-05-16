// vim: set ft=arduino:
/*
 * tasker.cpp
 * Copyright (C) 2022 Havránek Kryštof <krystof@havrak.xyz>
 *
 * Distributed under terms of the MIT license.
 */

#include "tasker.h"


uint8_t Task::invoke(){
	try{
		switch (ct) {
			case CallbackArgsType::NO_ARGS:
				TASKER_DEBUG  printf("TASK | invoke | no args, id: %04X\n", id);
				owner->call(id);
				return 1;
				break;
			case CallbackArgsType::BYTE_ARRAY:
				TASKER_DEBUG   printf("TASK | invoke | byte array, id: %04X\n", id);
				owner->callArgs(id,args, argc);
				return 1;
				break;
			case CallbackArgsType::SINGLE_INPUT:
				TASKER_DEBUG   printf("TASK | invoke | single arg (%06d), id: %04X\n", (uint32_t) argc, id);
				owner->callArg(id,argc);
				return 1;
				break;
			case CallbackArgsType::NON_ACTIVE:
				return 0;
		}
	}
	catch (const std::exception&) {
		Serial.println("TASK | invoke | exception occurred");
	}
	return 0;
}

Tasker* Tasker::instance = nullptr;

Tasker::Tasker():
	heap(new std::unique_ptr<Task>[201])
	/* outOfOrderTasks(new std::unique_ptr<Task>[MAX_NO_OUT_OFF_ORDER_TASK]) */
{
}

Tasker* Tasker::getInstance()
{
	if (instance == nullptr) {
		instance = new Tasker();
	}
	return instance;
}

void Tasker::tick()
{
	if (!active || heap[1] == nullptr || heap[1]->timeOfExecution > millis())
		return;

	printf("\n");
	static uint16_t index;
	static uint16_t i;
	static Task* t;
	static Task* outOfOrderTasks[MAX_NO_OUT_OFF_ORDER_TASK] = {nullptr};

	index = 0;

	for (i = 2; i <= heapSize; i++) {
		if (heap[i]->priority > heap[1]->priority &&
				heap[i]->timeOfExecution < (heap[1]->timeOfExecution + heap[1]->expectedRunTime) &&
				heap[i]->expectedRunTime < MAX_RUNTIME_OUT_OFF_ORDER_TASK ) {
			TASKER_DEBUG Serial.println("TASKER | tick | found task with higher priority that would finish late");
			outOfOrderTasks[index] = pop();
			index++;
			if(index == MAX_NO_OUT_OFF_ORDER_TASK) break;
		}
	}

	for(i = 0; i < index; i++){
		outOfOrderTasks[i]->invoke();
		delete outOfOrderTasks[i];
		outOfOrderTasks[i] = nullptr;
	}

	printf("TASKER | tick | Free memory: %d bytes\n", esp_get_free_heap_size());
	printHeap();
	t = pop();
	t->invoke();
	delete t;

	if (heap[1] != nullptr && heap[1]->timeOfExecution - millis() > 3000 && !SleepManager::getInstance()->isInServiceMode()){
		printf("TASKER | tick | next task with id: %04X, starting sleep\n", heap[1]->id);
		SleepManager::getInstance()->sleepFor(heap[1]->timeOfExecution-millis());
	}

}

void Tasker::printHeap(){
	for(uint16_t i =1 ; i < 11; i++){
		if(heap[i] == nullptr)
			printf("(%d: %04X) ", i, 0);
		else
			printf("(%d: %04X) ", i, heap[i]->id);

	}
	printf("\n");
}

bool Tasker::addNewTaskId(Task* f){
	return !isIdPresent(f->id) ? addNewTask(f) : false;
}

bool Tasker::addNewTask(Task* f)
{
	if(heapSize+1 == MAX_HEAP_SIZE){
		TASKER_DEBUG Serial.println("TASKER | addNewTask | trying to add task - heap full");
		return false;
	}

	static uint16_t index;


	heapSize++;
	heap[heapSize]= std::unique_ptr<Task>(f);
	index = heapSize;

	for (; index > 1; index /=2) {
		if (heap[index / 2]->timeOfExecution > heap[index]->timeOfExecution) {
			heap[index / 2].swap(heap[index]);
		} else
			break;
	}
	TASKER_DEBUG printf("TASKER | addNewTask | added task with id: %04X, pos: %d\n", f->id, index);
	return true;
}

uint16_t Tasker::getHeapSize()
{
	return heapSize;
}

bool Tasker::deleteTaskById(uint16_t id)
{
	for (uint16_t i = 1; i <= heapSize; i++) {
		if (heap[i]->id == id) {
			deleteTaskByIndex(i);
			return true;
		}
	}
	return false;
}

bool Tasker::deleteTasksById(uint16_t id){
	for (uint16_t i = 1; i <= heapSize; i++) {
		if (heap[i]->id == id) {
			deleteTaskByIndex(i);
			i--;
			break;
		}
	}
	return true;
}

bool Tasker::deleteTaskByIndex(uint16_t index)
{

	heap[index].reset();
	heap[heapSize].swap(heap[index]);
	static uint16_t index2;
	static uint16_t tmp;

	for(; index < MAX_HEAP_SIZE_HALF;){
		index2 = index*2;
		tmp = index;
		if(heap[index2] != nullptr && heap[tmp]->timeOfExecution > heap[index2]->timeOfExecution)
			tmp = index2;

		if(heap[index2+1] != nullptr && heap[tmp]->timeOfExecution > heap[index2+1]->timeOfExecution)
			tmp = index2+1;

		if(tmp == index)
			break;

		heap[index].swap(heap[tmp]);
		index = tmp;
	}

	heapSize--;
	return true;
}

Task* Tasker::getTaskByIndex(uint16_t index)
{
	return index > 0 && index <= heapSize ? heap[index].get() : nullptr;
}

Task* Tasker::getTaskById(uint16_t id)
{
	if(id == 0) return nullptr;
	for (uint16_t i = 1; i <= heapSize; i++) {
		if (heap[i]->id == id) {
			return heap[id].get();
		}
	}
	return nullptr;
}

bool Tasker::isIdPresent(uint16_t id)
{
	if(id == 0) return false;
	for (int i = 1; i  <= heapSize; i++) {
		if (heap[i]->id == id) {
			return true;
		}
	}
	return false;
}

bool Tasker::isIdPresentFromOwner(uint16_t id, CallbackInterface* owner)
{
	if(id == 0) return false;
	for (int i = 1; i  <= heapSize; i++) {
		if (heap[i]->id == id && heap[i]->owner == owner) {
			return true;
		}
	}
	return false;
}


Task* Tasker::pop()
{
	Task* toReturn = heap[1].release();
	deleteTaskByIndex(1);
	return toReturn;
}

Task* Tasker::getHead()
{
	return heap[1].get();
}
