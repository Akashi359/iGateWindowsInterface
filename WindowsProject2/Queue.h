#pragma once
#include<winnt.h>
#include "framework.h"
#include <stdio.h>

#define MAXQUEUESIZE 256

/*
	"Remember that if you are doing template programming,
	then you have to keep everything in the .h file
	so that the compiler will instantiate the right code
	at the moment of compilation"
*/

template <class T>
class Queue {
private:
	UINT32 headIndex;
	UINT32 tailIndex;
	volatile T* backingArray[MAXQUEUESIZE];
public:
	Queue() {
		headIndex = 0;
		tailIndex = 0;
		memset(backingArray, 0, MAXQUEUESIZE * sizeof(void*));
	}
	//So long as we don't produce while the thread is dead, the destructor won't run at the same time as a member function
	~Queue() {
		for (unsigned k = 0; k < MAXQUEUESIZE; k++)
			delete backingArray[k];
	}

	bool produce(T value) {
		//1) prevents concurrent calls to produce() from causing corruption:
		UINT32 indexRetVal;
		UINT32 reservedIndex;
		do {
			reservedIndex = tailIndex;
			//This interlocked function expects 32 bit variables
			indexRetVal = InterlockedCompareExchangeNoFence(&tailIndex, (reservedIndex + 1) % MAXQUEUESIZE, reservedIndex);
		} while (indexRetVal != reservedIndex);

		//2) allocates the node.
		T* newValPtr = new T(value);

		//3) prevents a concurrent call to consume from causing corruption by atomically replacing the old pointer:
		volatile void* valPtrRetVal = InterlockedCompareExchangePointerRelease((volatile PVOID*)backingArray + reservedIndex, newValPtr, NULL);
		//if the previous value wasn't NULL, then our circular buffer overflowed:
		if (valPtrRetVal != NULL) {
			OutputDebugString(L"Queue: circular buffer overflowed");
			delete newValPtr;
			return false;
		}

		//otherwise, everything worked fine
		return true;
	}

	void consumeAll(void(*process)(T value)) {
		while (backingArray[headIndex] != NULL) {
			volatile T* valPtr = backingArray[headIndex];
			backingArray[headIndex] = NULL;
			process(valPtr);
			delete valPtr;
			headIndex = (headIndex + 1) % MAXQUEUESIZE;
		}
	}

	/*
		Pops one message off of the queue
	*/
	volatile T* pop() {
		if (backingArray[headIndex] != NULL) {
			volatile T* valPtr = backingArray[headIndex];
			backingArray[headIndex] = NULL;
			headIndex = (headIndex + 1) % MAXQUEUESIZE;
			return valPtr;
		}
		else
			return NULL;
	}

	/*
		Enforces a memory barrier which prevents reordering of memory operations, then checks if the queue was empty.
		This does not guarantee that the queue is empty at the time this function returns - just that it was empty at some point after the memory barrier.
	*/
	bool flush_checkEmpty() {
		MemoryBarrier();
		volatile T* valPtr = backingArray[headIndex];
		return backingArray[headIndex] == NULL;
	}
};
