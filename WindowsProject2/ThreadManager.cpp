#include "ThreadManager.h"
#include <process.h>

ThreadManager::ThreadManager(){
	//allocate killEvent and writeRequestEvent
	killEvent = CreateEvent(NULL, true, false, NULL);
	if (!killEvent) {
		throw GetLastError();
	}
	writeRequestEvent = CreateEvent(NULL, true, false, NULL);
	if (!writeRequestEvent) {
		DWORD temp = GetLastError();
		CloseHandle(killEvent);
		throw temp;
	}

}

bool ThreadManager::startThread(uint32_t comPortNum,
								uint32_t baudRate,
								uint32_t dataBits,
								uint32_t parity,
								uint32_t stopBits,
								char* errorMsg) {
	this->comPortNum = comPortNum;
	this->baudRate = baudRate;
	this->dataBits = dataBits;
	this->parity = parity;
	this->stopBits = stopBits;

	if (_beginthread(ThreadManager::threadFtn, 0, this) == -1L) {
		strcpy(errorMsg, "_beginthread returned -1L");
		return false;
	}
	return true;
}

void ThreadManager::threadFtn(void* argPtr) {
	ThreadManager* tmp = (ThreadManager*)argPtr;
	if (!tmp->setup()) {
		tmp->receive(WM_THREAD_DOWN, nullptr);
		return;
	}
	tmp->receive(WM_THREAD_UP, nullptr);
	tmp->loop();
	tmp->teardown();
	tmp->receive(WM_THREAD_DOWN, nullptr);
}

bool ThreadManager::setup() {
	if (!ResetEvent(killEvent)) {
		receive(WM_THREAD_ERROR, "failed to reset killEvent");
		return false;
	}
	if (!ResetEvent(writeRequestEvent)) {
		receive(WM_THREAD_ERROR, "failed to reset writeRequestEvent");
		return false;
	}

	//assemble the com port name.
	size_t itowSize = 0;
	for (unsigned temp = comPortNum; temp > 0; temp /= 10)
		itowSize++;
	wchar_t* comPortName = (wchar_t*)malloc(sizeof(wchar_t)*(8 + itowSize));
	if (!comPortName) {
		receive(WM_THREAD_ERROR, "failed to malloc comPortName");
		return false;
	}

	//open the comPort
	hCom = CreateFile(comPortName,
		GENERIC_READ | GENERIC_WRITE,
		0,      //  must be opened with exclusive-access
		NULL,   //  default security attributes
		OPEN_EXISTING, //  must use OPEN_EXISTING
		FILE_FLAG_OVERLAPPED,      //  can't write in one thread while blocked for a read in another unless port is declared overlapped
		NULL); //  hTemplate must be NULL for comm devices
	free(comPortName);
	if (hCom == INVALID_HANDLE_VALUE) {
		receive(WM_THREAD_ERROR, "failed to create hCom handle");
		return false;
	}

	//By default, all bytes received are considered part of a single, long message
	COMMTIMEOUTS commTimeOuts;
	SecureZeroMemory(&commTimeOuts, sizeof(COMMTIMEOUTS));
	//"only bytes received within 1 ms of each other are considered part of the same message"
	commTimeOuts.ReadIntervalTimeout = 1;
	if (!SetCommTimeouts(hCom, &commTimeOuts)) {
		CloseHandle(hCom);
		receive(WM_THREAD_ERROR, "failed to SetCommTimeouts");
		return false;
	}

	//Set Baudrate and the like
	DCB dcb;
	SecureZeroMemory(&dcb, sizeof(DCB));
	//For some reason, only these five fields matter, despite the spec's claims to the contrary.
	//The rest I keep at zero
	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = baudRate;
	dcb.ByteSize = dataBits;
	dcb.Parity = parity;
	dcb.StopBits = stopBits;
	if (!SetCommState(hCom, &dcb)) {
		CloseHandle(hCom);
		receive(WM_THREAD_ERROR, "failed to SetCommState");
		return false;
	}

	//Empty the msgQueue - a previous iteration of the thread may have died before getting a chance to empty the queue,
	//meaning there are msgs in the queue that shouldn't be there.
	//we want the memory barrier on this, so we just call consume() normally instead of some builtin flush method.
	while (msgQueue.notEmpty())
		delete(msgQueue.consume());

	return true;
}

void ThreadManager::loop() {
	const unsigned long BUFFSIZE = 256;
	BYTE readBuffer[BUFFSIZE + 1];
	readBuffer[BUFFSIZE] = '\0';
	BYTE writeBuffer[BUFFSIZE + 1];
	writeBuffer[BUFFSIZE] = '\0';
	int numChars;
	DWORD bytesRead, bytesWritten, result;
	OVERLAPPED osReader;
	OVERLAPPED osWriter;
	HANDLE eventArray[3];
	eventArray[0] = killEvent;
	eventArray[2] = writeRequestEvent;

	BOOL continueLooping = true;
	BOOL fWaitingOnRead = FALSE;
	BOOL fWaitingOnWrite = FALSE;

	while (continueLooping) {
		//There is no outstanding read request and we must create a new one:
		if (!fWaitingOnRead) {
			//First, check to make sure we haven't received a killEvent.
			result = WaitForSingleObject(killEvent, 0); //returns immediately
			if (result == WAIT_OBJECT_0) {
				//received killEvent, exiting
				if (!ResetEvent(killEvent))
					receive(WM_THREAD_ERROR, "failed to reset killEvent");
				break;
			}
			else if (result != WAIT_TIMEOUT) {
				//some error occured
				receive(WM_THREAD_ERROR, "WaitForSingleObject returned an error");
				break;
			}
			//Otherwise, there was no kill request, continue as normal.
			//Attempt to create new read request
			osReader = { 0 };
			osReader.hEvent = CreateEvent(NULL, true, false, NULL);
			//read event failed to allocate, return:
			if (osReader.hEvent == NULL) {
				receive(WM_THREAD_ERROR, "failed to create readEvent");
				break;
			}
			eventArray[1] = osReader.hEvent;
			//Execute the asynchronous read:
			if (!ReadFile(hCom, readBuffer, BUFFSIZE, &bytesRead, &osReader)) {
				//The asynchronous read request succeeded and will be completed later:
				if (GetLastError() == ERROR_IO_PENDING) {
					fWaitingOnRead = TRUE;
				}
				//The asynchronous read request failed:
				else {
					receive(WM_THREAD_ERROR, "ReadFile returned an error");
					CloseHandle(osReader.hEvent);
					break;
				}
			}
			//The asynchronous read request succeeded and completed immediately:
			else {
				CloseHandle(osReader.hEvent);
				receive(WM_THREAD_RECV, ByteBuffer::getByteBuffer(readBuffer, bytesRead));
			}
		}
		//We are waiting on an outstanding read request:
		else {
			//block until a signal arrives
			//if we are waiting on a write, then we block for a write response
			//otherwise, we wait for a write request
			result = WaitForMultipleObjects(3, eventArray, FALSE, INFINITE);
			continueLooping = false;
			switch (result) {
				//The killEvent handle received a signal. This has priority over every other signal.
			case WAIT_OBJECT_0:
				if (!ResetEvent(killEvent))
					receive(WM_THREAD_ERROR, "failed to reset killEvent");
				break;

				//The com port handle received a signal
			case WAIT_OBJECT_0 + 1:
				//Unsuccessful read
				if (!GetOverlappedResult(hCom, &osReader, &bytesRead, FALSE)) {
					receive(WM_THREAD_ERROR, "GetOverlappedResult returned an error");
					break;
				}
				//Successful read, continue looping
				receive(WM_THREAD_RECV, ByteBuffer::getByteBuffer(readBuffer, bytesRead));
				fWaitingOnRead = FALSE;
				CloseHandle(osReader.hEvent);
				continueLooping = true;
				break;

				//This is either a request for a write, or the response from a previous write:
			case WAIT_OBJECT_0 + 2:
				//response from previous write:
				if (fWaitingOnWrite) {
					//Unsuccessful write
					if (!GetOverlappedResult(hCom, &osWriter, &bytesWritten, FALSE)) {
						receive(WM_THREAD_ERROR, "GetOverlappedResult returned an error");
						break;
					}
					//Successful write, continue looping
					receive(WM_THREAD_SENT, nullptr);
					fWaitingOnWrite = FALSE;
					CloseHandle(osWriter.hEvent);
					continueLooping = true;
					eventArray[2] = writeRequestEvent;
					break;
				}
				//request for a write:
				else {
					//order matters here: unset, check nonEmpty, consume if nonEmpty, check nonEmpty again, reset if nonEmpty
					//unset the event flag:
					if (!ResetEvent(writeRequestEvent)) {
						receive(WM_THREAD_ERROR, "failed to reset writeRequestEvent");
						break;
					}
					//retrieve message from queue:
					if (msgQueue.notEmpty()) {
						ByteBuffer* newMsg = msgQueue.consume();
						if (newMsg->getCount > BUFFSIZE) {
							delete(newMsg);
							receive(WM_THREAD_ERROR, "message in message queue is too large for the writeBuffer");
							break;
						}
						numChars = newMsg->getCount();
						newMsg->getBinary(writeBuffer);
						delete(newMsg);
					}
					//reset the flag if the queue is non-empty.
					if(msgQueue.notEmpty())
						if (!SetEvent(writeRequestEvent)) {
							receive(WM_THREAD_ERROR, "failed to set writeRequestEvent");
							break;
						}
					//attempt to create writeResponseEvent:
					osWriter = { 0 };
					osWriter.hEvent = CreateEvent(NULL, true, false, NULL);
					if (osWriter.hEvent == NULL) {
						receive(WM_THREAD_ERROR, "failed to create writeResponseEvent");
						break;
					}
					eventArray[2] = osWriter.hEvent;
					//execute the asynchronous write:
					if (!WriteFile(hCom, writeBuffer, numChars, &bytesWritten, &osWriter)) {
						//The asynchronous write request succeeded and will be completed later:
						if (GetLastError() == ERROR_IO_PENDING) {
							fWaitingOnWrite = true;
							continueLooping = true;
							break;
						}
						//The asynchronous write request failed:
						else {
							receive(WM_THREAD_ERROR, "WriteFile returned an error");
							CloseHandle(osWriter.hEvent);
							break;
						}
					}
					//The asynchronous write request succeeded and completed immediately
					else {
						receive(WM_THREAD_SENT, nullptr);
						CloseHandle(osWriter.hEvent);
						continueLooping = true;
						eventArray[2] = writeRequestEvent;
						break;
					}
				}
				// Error in WaitForMultipleObjects()
			default:
				receive(WM_THREAD_ERROR, "WaitForMultipleObjects returned an error");
				break;
			}
		}
	}
	//Exited loop
	CancelIo(hCom);
	if (fWaitingOnRead)
		CloseHandle(osReader.hEvent);
	if (fWaitingOnWrite)
		CloseHandle(osWriter.hEvent);
}

void ThreadManager::teardown() {
	CloseHandle(hCom);
}

ThreadManager::~ThreadManager(){
	CloseHandle(killEvent);
	CloseHandle(writeRequestEvent);
}

bool ThreadManager::write(ByteBuffer* arg) {
	if (msgQueue.produce(arg)) {
		if (!SetEvent(writeRequestEvent)) {
			throw GetLastError();
		}
		return true;
	}
	return false;
}

void ThreadManager::endThread() {
	if (!SetEvent(killEvent))
		throw GetLastError();
}