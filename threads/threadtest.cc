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

// testnum is set in main.cc
int testnum = 1;
extern int existingThreadcnt;
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

void ThreadTest2()
{
    DEBUG('t', "Entering ThreadTest2");

    Thread *t1 = new Thread("forked_1");
	Thread *t2 = new Thread("forked_2");
	scheduler->Print();
	
	
    t1->Fork(SimpleThread, 1);
	scheduler->Print();
	
	t2->Fork(SimpleThread, 2);	
	scheduler->Print();
	
    SimpleThread(0);
}

void ThreadTest3(){
    DEBUG('t', "Entering ThreadTest3");
	Thread *t;
	for (int i=0;i<130;i++){
		t = new Thread("forked");
		printf("Fork thread %d\n",existingThreadcnt);
	}
    
}

Lock *mutex;
Lock *w;
Semaphore *full;
Semaphore *empty;
int item_cnt = 0;
int rc=0;

void Produce(int arg){
	while (arg--){
		empty->P();
		mutex->Acquire();
		item_cnt++;
		printf("Produce! Now Buffer = %d\n",item_cnt);
		mutex->Release();
		full->V();
	}
}
void Consume(int arg){
	while (arg--){
		full->P();
		mutex->Acquire();
		item_cnt--;
		printf("Consume!! Now Buffer = %d\n",item_cnt);
		mutex->Release();
		empty->V();
	}
}
void ThreadTest4(){
	mutex = new Lock("lock");
	full = new Semaphore("FULL",0);
	empty = new Semaphore("EMPTY",5);
	Thread *consumer = new Thread("Consumer");
	Thread *producer = new Thread("Producer");
    consumer->Fork(Consume,10);
	producer->Fork(Produce,10);
	delete mutex;
	delete full;
	delete empty;
}

void reader(int arg){
	while (arg--){
		mutex->Acquire();
		rc=rc+1;
		if (rc==1) w->Acquire();
		mutex->Release();
		printf("%s starts reading\n",currentThread->getName());
		mutex->Acquire();
		rc=rc-1;
		if (rc==0) w->Release();
		mutex->Release();
		printf("%s finishes reading\n",currentThread->getName());
		currentThread->Yield();
	}
}
void writer(int arg){
	while (arg--){
		w->Acquire();
		printf("Writing\n");
		w->Release();
		currentThread->Yield();
	}
}
void ThreadTest5(){
	mutex = new Lock("mutex");
	w = new Lock("w");
	rc = 0;
	Thread* r1=new Thread("Reader_1");
	Thread* r2=new Thread("Reader_2");
	Thread* r3=new Thread("Reader_3");
	Thread* w0=new Thread("Writer");
	r1->Fork(reader,2);
	w0->Fork(writer,5);
	r2->Fork(reader,2);
	r3->Fork(reader,2);
	delete mutex;
	delete w;
}

int unreached;
Condition* barrier;
Lock* bar_lock;

void BarrierThread(int arg){
	mutex->Acquire();
	unreached--;
	printf("%s reach barrier!\n",currentThread->getName());
	if (unreached==0){
		mutex->Release();
		bar_lock->Acquire();
		barrier->Broadcast(bar_lock);
	}
	else{
		mutex->Release();
		bar_lock->Acquire();
		barrier->Wait(bar_lock);
	}
	bar_lock->Release();
	printf("%s cross barrier!\n",currentThread->getName());
}
void ThreadTest6(){
	mutex = new Lock("mutex");
	bar_lock = new Lock("bar_lock");
	barrier = new Condition("barrier");
	unreached = 3;
	Thread* t1=new Thread("Thread_1");
	Thread* t2=new Thread("Thread_2");
	Thread* t3=new Thread("Thread_3");
	t1->Fork(BarrierThread,0);
	t2->Fork(BarrierThread,0);
	t3->Fork(BarrierThread,0);
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
	ThreadTest2();
	scheduler->Print();
	break;
	case 3:
	ThreadTest3();
	break;
	case 4:
	ThreadTest4();
	break;
	case 5:
	ThreadTest5();
	break;
	case 6:
	ThreadTest6();
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

