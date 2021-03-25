#pragma once
#include "ByteBuffer.h"
#include <Windows.h>

/*
	Only thread-safe in a single producer, single consumer scenario.
*/
class MpScQueue {
private:
	static const unsigned MAXSIZE = 64;
	ByteBuffer* volatile backingArray[MAXSIZE];
	ByteBuffer* volatile* volatile head;
	ByteBuffer* volatile* volatile tail;
	MpScQueue(const MpScQueue &src);
	MpScQueue& operator=(const MpScQueue &src);
public:
	/*
		Just sets some primitives. Nothing special here.
	*/
	MpScQueue();

	/*
		Never delete while there is an outstanding consume or produce in progress.
		In other words, assure that only one thread is running when the destructor is executed.
	*/
	~MpScQueue();

	/*
		If using interthread signals, always produce first, and then set the signal.

		Returns true on success.
		Returns false if the queue is full.
	*/
	boolean produce(ByteBuffer*);

	/*
		If using interthread signals, always unset the signal first, and then check if not empty, and then consume,
			in that order.

		Use notEmpty() to first check if it is safe to consume(). It is not threadsafe to consume without checking.
	*/
	ByteBuffer* consume();

	/*
		When called by the consumer, then in all cases where it returns true, it remains true up until the next consume.
		If it returns false, it may or may not be correct.
		No guarantees when called by the producer, but the producer has no reason to care if its empty or not anyways.
	*/
	boolean notEmpty();
};
