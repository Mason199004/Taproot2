#ifndef TAPROOT2_LIBRARY_H
#define TAPROOT2_LIBRARY_H

#include <stdio.h>
#include <stdlib.h>
#include <mem.h>
#include <unistd.h>
#include <pthread.h>

typedef struct {
	void* Result;
	void* (*Action)(void*); //void* Action(void* arg)
	void* Arg;
	bool IsCompleted;
} TRTask;

typedef struct {
	pthread_mutex_t lock;
	TRTask* CurrentTask;
} TRTaskRunner;

typedef struct {
	TRTask** TaskArray;
	pthread_mutex_t lock;
	int ArrSize;
	int begin;
	int end;
} TRQueue;

typedef struct {
	int NumRunners;
	TRQueue TaskQueue;
	TRTaskRunner Runners[];
} TRScheduler;

TRTask* CreateTask(void* (*Action)(void*), void* Arg);

void* TRTaskRunnerFunc(void* TaskRunner);

void TRTaskRunnerInit(TRTaskRunner* runner);

void TRQueueAdd(TRQueue* queue, TRTask* task);

TRTask* TRQueueGetNext(TRQueue* queue);

TRQueue TRQueueInit(int Size);

_Noreturn void* SchedulerLoop(void* scheduler);

TRScheduler* TRSchedulerInit(int QueueSize, int RunnerCount);

TRTask* CreateAndEnqueueTask(TRScheduler* Scheduler, void* (*Action)(void*), void* Arg);

#endif //TAPROOT2_LIBRARY_H
