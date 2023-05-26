// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "synch.h"
#include "thread.h"
#include "syscall.h"

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//---------------------------------------------------------------------

#define BUFFER_SIZE 5 // 버퍼 크기
#define NUM_ITEMS 20  // 전체 아이템 수

int buffer[BUFFER_SIZE]; // 공유 버퍼
int count = 0;           // 버퍼에 있는 아이템 수를 나타내는 변수
int in = 0;              // 다음에 아이템을 넣을 위치
int out = 0;             // 다음에 아이템을 가져올 위치

Semaphore *mutex;        // 상호 배제를 위한 세마포어
Semaphore *empty;        // 버퍼가 비어있음을 나타내는 세마포어
Semaphore *full;         // 버퍼가 가득 찼음을 나타내는 세마포어

void Producer(int id) {
    for (int i = 0; i < NUM_ITEMS; i++) {
        empty->P();    // 버퍼에 빈 공간이 있는지 확인
        mutex->P();    // cs 진입 전 mutual exclusion 실행

        // 생산자가 아이템을 생산하여 버퍼에 넣음
        buffer[in] = i;
        in = (in + 1) % BUFFER_SIZE;
        count++;

		printf("Producer %d produced item %d\n", id, i);
        mutex->V();    // cs 나오면서  mutual exclusion 해제:
        full->V();     // 소비자에게 버퍼에 새로운 아이템이 있다고 알림

        // ReadyToRun에 들어있는 다른 thread가 실행할 수 있도록 현재 thread는 양보한다.
        currentThread->Yield();
    }
}

void Consumer(int id) {
    for (int i = 0; i < NUM_ITEMS; i++) {
        full->P();     // 버퍼에 아이템이 있는지 확인
        mutex->P();    // cs 진입 전 mutual exclusion 실행 

        // 소비자가 버퍼에서 아이템을 가져옴
        int item = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;

		printf("Consumer %d consumed item %d\n", id, item);
		mutex->V();    // cs 나오면서  mutual exclusion 해제:
        empty->V();    // 생산자에게 버퍼에 빈 공간이 있다고 알림

        // ReadyToRun에 들어있는 다른 thread가 실행할 수 있도록 현재 thread는 양
보한다.
        currentThread->Yield();
    }
}

void ProducerConsumerTest() {
    mutex = new Semaphore("mutex", 1);
    empty = new Semaphore("empty", BUFFER_SIZE);
    full = new Semaphore("full", 0);

	Semaphore *semaphore = new Semaphore("semaphore",1);

    Thread *producer = new Thread("producer");
	Thread *consumer = new Thread("consumer");

	producer->Fork(Producer,0);
	consumer->Fork(Consumer,1);
}

//---------------------------------------------------------------------
void printPriority(int priority){
	printf("this thread's priority is %d\n", priority);
}

void PrioritySchedularTest(){
	Thread *thread1 = new Thread("thread1");
	Thread *thread2 = new Thread("thread2");
	Thread *thread3 = new Thread("thread3");
	Thread *thread4 = new Thread("thread4");

	thread1->setPriority(10);
	thread2->setPriority(2);
	thread3->setPriority(5);
	thread4->setPriority(3);

	thread1->Fork(printPriority,thread1->getPriority());
	thread2->Fork(printPriority,thread2->getPriority());
	thread3->Fork(printPriority,thread3->getPriority());
	thread4->Fork(printPriority,thread4->getPriority());

	currentThread->Finish();
}



//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
		ThreadTest1();
		break;
	case 2:
		ProducerConsumerTest();
		break;
	case 3:
		PrioritySchedularTest();
		break;
    default:
		printf("No test specified.\n");
    }
}

