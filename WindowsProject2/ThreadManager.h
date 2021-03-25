#pragma once
#include <Windows.h>
#include <stdint.h>
#include "ByteBuffer.h"
#include "MpScQueue.h"
#include "WindowsProject2.h"


/*
	Pure virtual class. Must implement virtual function "receive()" before using.

	Throws exceptions if inter-thread communication HANDLE objects fail,
	which can occur in write(), endThread(), and the constructor.
*/
class ThreadManager {
private:
	uint32_t comPortNum, baudRate, dataBits, parity, stopBits;		//primitives - no allocation or deallocation necessary
	HANDLE killEvent, writeRequestEvent, hCom;						//will be allocated by the constructor. Throws an exception if it fails
	MpScQueue msgQueue;												//lifetime tied to the threadmanager and not the thread, just in case the thread dies during a call to msgQueue.produce()
																	//	Cannot fail to allocate

	static void threadFtn(void*);
	bool setup();
	void loop();
	void teardown();

	ThreadManager(const ThreadManager &src);
	ThreadManager& operator=(const ThreadManager &src);

public:
	ThreadManager();

	/*
		Attempts to start the thread.
		Do not attempt to call startThread on a thread that is starting, running, or stopping (only call on a thread that is stopped).
		Success is indicated by a return value of true followed by an eventual call to receive(WM_THREAD_UP, null);
		Failure is indicated by a return value of false OR a return value of true and an eventual call to receive(WM_THREAD_ERROR, const char*);
		followed by a call to receive(WM_THREAD_DOWN, null) and the thread will stop running.
			@param comPortNum	The number indicating what comport to open on.
			@param baudRate		Settings for the comport.
			@param dataBits		Settings for the comport.
			@param parity		Settings for the comport.
			@param stopBits		Settings for the comport.
			@param errorMsg		populated by an error message upon failure.
	*/
	bool startThread(uint32_t comPortNum,
							uint32_t baudRate,
							uint32_t dataBits,
							uint32_t parity,
							uint32_t stopBits,
							char* errorMsg);

	/*
		Attempts to write the message contained in the indicated ByteBuffer to the comport.
		Returns true of if the message was successfully deposited into the messageQueue.
		Returns false if the queue was full, or if the message was a nullptr.

		A call to write() while a thread is stopped or stopping has no effect.
		A call to write() while a thread is starting may or may not succeed.
		Success is indicated by a later call to receive(WM_THREAD_SENT, null);
		Failure is indicated by a later call to receive(WM_THREAD_ERROR, const char*),
			followed by a call to receive(WM_THREAD_DOWN, null) and the thread will stop running.
	*/
	bool write(ByteBuffer*);

	/*
		Attempts to stop the thread.

		A call to endThread() while a thread is stopped or stopping has no effect.
		A call to endThread() while a thread is starting may or may not cause the thread to terminate.
		Success is indicated by a later call to receive(WM_THREAD_DOWN, null).
		May encounter an error, in which case it will call receive(WM_THREAD_ERROR, const char*) before calling receive(WM_THREAD_DOWN, null).
		May also fail entirely, in which case it throws an exception with GetLastError() as the argument.
	*/
	void endThread();

	/*
		Called by the thread to indicate any changes in state or status.
			receive(WM_THREAD_UP, null)				Indicates that a previous call to startThread has succeeded and the thread is now running.
			receive(WM_THREAD_DOWN, null)			Indicates that the thread is now stopped.
			receive(WM_THREAD_ERROR, const char*)	Indicates that the thread has encountered an error. The thread will always shutdown after an error.
			receive(WM_THREAD_RECV, ByteBuffer*)	Indicates that the thread received a message.
			receive(WM_THREAD_SENT, null)			Indicates that a previous call to write has succeeded.
	*/
	virtual void receive(unsigned, void*) = 0;

	/*
		Do not delete the threadManager while a thread is starting, running, or stopping (only delete when the thread is stopped).
	*/
	virtual ~ThreadManager();
};