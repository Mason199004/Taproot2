#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <mem.h>
#include <unistd.h>

typedef struct {
	void* Result;
	void* (*Action)(void*); //void* Action(void* arg)
	void* Arg;
	bool IsCompleted;
} TRTask;

TRTask* CreateTask(void* (*Action)(void*), void* Arg)
{
	TRTask* task = malloc(sizeof(TRTask));
	task->Action = Action;
	task->Arg = Arg;
	task->IsCompleted = false;
	task->Result = nullptr;
	return task;
}

typedef struct {
	pthread_mutex_t lock;
	TRTask* CurrentTask;
} TRTaskRunner;

void* TRTaskRunnerFunc(void* TaskRunner)
{
	TRTaskRunner* self = TaskRunner;
	self->CurrentTask->Result = self->CurrentTask->Action(self->CurrentTask->Arg);
	self->CurrentTask->IsCompleted = true;
	self->CurrentTask = nullptr;
	pthread_mutex_unlock(&self->lock);
	return 0;
}

void TRTaskRunnerInit(TRTaskRunner* runner)
{
	pthread_mutex_init(&runner->lock, NULL);
}

typedef struct {
	TRTask** TaskArray;
	pthread_mutex_t lock;
	int ArrSize;
	int begin;
	int end;
} TRQueue;

void TRQueueAdd(TRQueue* queue, TRTask* task)
{
	pthread_mutex_lock(&queue->lock);
	if (queue->end == queue->ArrSize)
	{
		fprintf(stderr, "TaskQueue is full. Aborting!!");
		abort();
	}

	queue->end++;
	queue->TaskArray[queue->end] = task;

	pthread_mutex_unlock(&queue->lock);
}

TRTask* TRQueueGetNext(TRQueue* queue)
{
	pthread_mutex_lock(&queue->lock);

	queue->begin++;
	TRTask* ret = queue->TaskArray[queue->begin];

	if (queue->begin == queue->end)
	{
		queue->begin = 0;
		queue->end = 0;
	}

	pthread_mutex_unlock(&queue->lock);
	return ret;
}

TRQueue TRQueueInit(int Size)
{
	TRQueue queue;
	memset(&queue, 0, sizeof(TRQueue));
	queue.TaskArray = malloc(sizeof(TRTask*) * Size);
	queue.ArrSize = Size;
	return queue;
}

typedef struct {
	int NumRunners;
	TRQueue TaskQueue;
	TRTaskRunner Runners[];
} TRScheduler;

_Noreturn void* SchedulerLoop(void* scheduler)
{
	TRScheduler* Sched = scheduler;
	start:
	while (Sched->TaskQueue.end - Sched->TaskQueue.begin == 0)
	{ usleep(10); }
	for (int i = 0; i < Sched->NumRunners; ++i)
	{
		if (pthread_mutex_trylock(&Sched->Runners[i].lock))
		{
			Sched->Runners[i].CurrentTask = TRQueueGetNext(&Sched->TaskQueue);
			pthread_create(NULL, NULL, TRTaskRunnerFunc, &Sched->Runners[i]);
			goto start;
		}
	}
	goto start;
}

TRScheduler* TRSchedulerInit(int QueueSize, int RunnerCount)
{
	TRScheduler* scheduler = malloc(sizeof(TRScheduler) + (sizeof(TRTaskRunner) * RunnerCount));
	scheduler->NumRunners = RunnerCount;
	scheduler->TaskQueue = TRQueueInit(QueueSize);
	for (int i = 0; i < RunnerCount; ++i)
	{
		TRTaskRunnerInit(&scheduler->Runners[i]);
	}
	pthread_create(NULL, NULL, SchedulerLoop, scheduler);
	return scheduler;
}

TRTask* CreateAndEnqueueTask(TRScheduler* Scheduler, void* (*Action)(void*), void* Arg)
{
	TRTask* task = CreateTask(Action, Arg);
	TRQueueAdd(&Scheduler->TaskQueue, task);
	return task;
}