#include "MpScQueue.h"
#include <stdexcept>

MpScQueue::MpScQueue() {
	for (unsigned k = 0; k < MAXSIZE; k++)
		backingArray[k] = nullptr;
	head = backingArray;
	tail = backingArray;
}

MpScQueue::~MpScQueue() {
	for (unsigned k = 0; k < MAXSIZE; k++)
		delete(backingArray[k]);
}

boolean MpScQueue::produce(ByteBuffer* newElement) {
	//The notEmpty() function equates nullity with emptiness, so we can't produce null elements.
	if (newElement == nullptr)
		return false;

	ByteBuffer* volatile* newTail;
	ByteBuffer* volatile* reservedSlot;
	//Calculate the new address of the tail pointer
	do {
		reservedSlot = tail;
		newTail = tail + 1;
		if (newTail >= backingArray + MAXSIZE)
			newTail = backingArray;
		//If the queue is full, abort
		//Can spurriously abort, but will never fail to abort when it must.
		if (newTail == head)
			return false;
		//If another producer snuck in and reserved the same slot, then recalculate and try again.
	} while(newTail == InterlockedCompareExchangePointer((PVOID*)&tail, (PVOID)newTail, (PVOID)newTail));

	//Once the slot is reserved, go ahead and fill it.
	//New producers cannot sneak into the gap between these atomic operations,
	//because the head can't advance passed a (null) reservedSlot,
	//and a new producer can't advance passed the head.
	InterlockedExchangePointer((PVOID*)reservedSlot, (PVOID)newElement);
	return true;
}

ByteBuffer* MpScQueue::consume() {
	ByteBuffer* top = *head;
	InterlockedExchangePointer((PVOID*)head, nullptr);
	ByteBuffer* volatile* newHead = head + 1;
	if (head >= backingArray + MAXSIZE)
		newHead = backingArray;
	InterlockedExchangePointer((PVOID*)&head, (PVOID)newHead);
	return top;
}

/*
	If executed by the consumer in a single consumer scenario, then this function cannot be pre-empted by a write to the head variable,
		and is therefore atomic with respect to the head.
	It cannot retrieve a stale value of the head variable either, because all writes to the head set up memory barriers.
	The consumer may retrieve a stale tail value, but because the producer can only add more to the Queue,
		this function may erroneously return that the queue is empty when it isn't,
		but will never report that the queue is not empty when it is.
*/
boolean MpScQueue::notEmpty() {
	return (*head != nullptr);
}