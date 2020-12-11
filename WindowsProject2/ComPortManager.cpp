#include "framework.h"
#include "ComPortManager.h"
#include "WindowsProject2.h"
#include "cpyalloc.h"
#include <process.h>
extern "C" {
	#include "iGateDeviceSupport.h"
}

WriteArg::WriteArg(const char* buffer, int numChars) {
	this->buffer = cpyalloc(buffer, numChars);
	this->numChars = numChars;
}
WriteArg::WriteArg(const WriteArg &src) {
	this->buffer = cpyalloc(src.buffer, src.numChars);
	this->numChars = src.numChars;
}
WriteArg& WriteArg::operator=(const WriteArg &src) {
	if (this == &src)
		return *this;
	free(this->buffer);
	this->buffer = cpyalloc(src.buffer, src.numChars);
	this->numChars = src.numChars;
	return *this;
}
WriteArg::~WriteArg() {
	free(this->buffer);
}

const char* ComPortManager::TAG = "ComPortManager: ";

void ComPortManager::log(const char* errorMsg) {
	size_t msgLen = strlen(errorMsg);
	size_t tagLen = strlen(TAG);
	size_t bufferLen = msgLen + tagLen + 2;
	char* buffer = (char*)malloc(bufferLen * sizeof(char));
	strcpy_s(buffer, tagLen + 1, TAG);
	strcat(buffer, errorMsg);
	strcat(buffer, "\n");
	OutputDebugStringA(buffer);
	free(buffer);
}

bool ComPortManager::logAndQuit(const char* errorMsg) {
	log(errorMsg);
	return false;
}

ComPortManager::ComPortManager(unsigned comPortNum, const wchar_t* fileName, HWND uiWindow) {
	//Assemble file path from file name:
	size_t filePathLen = GetCurrentDirectory(0, NULL) + wcslen(fileName) + 1; //+1 for additional slash character
	mFilePathW = (wchar_t*)malloc(sizeof(wchar_t)*(filePathLen + 1)); //+1 again for terminating null character
	GetCurrentDirectoryW(filePathLen, mFilePathW);
	wcscat(mFilePathW, L"\\");
	wcscat(mFilePathW, fileName);
	mFilePath = (char*)malloc(sizeof(char*)*(filePathLen + 1));
	wcstombs(mFilePath, mFilePathW, filePathLen + 1);

	//Assemble the com port name from the com port number:
	this->comPortNum = comPortNum;
	size_t itowSize = 0;
	for (unsigned temp = comPortNum; temp > 0; temp /= 10)
		itowSize++;
	mComPortNameW = (wchar_t*)malloc(sizeof(wchar_t)*(8 + itowSize));
	wcscpy(mComPortNameW, L"\\\\.\\COM");
	_itow(comPortNum, mComPortNameW + 7, 10);
	mComPortName = (char*)malloc(sizeof(char*)*(8 + itowSize));
	strcpy(mComPortName, "\\\\.\\COM");
	_itoa(comPortNum, mComPortName + 7, 10);


	mPaneWindow = uiWindow;
	killEvent = CreateEvent(NULL, true, false, NULL);
	writeRequestEvent = CreateEvent(NULL, true, false, NULL);
}

ComPortManager::~ComPortManager() {
	free(mComPortName);
	free(mComPortNameW);
	free(mFilePath);
	free(mFilePathW);
	CloseHandle(killEvent);
	CloseHandle(writeRequestEvent);
}

void ComPortManager::startThread(uint32_t baudRate, uint32_t dataBits, uint32_t parity, uint32_t stopBits) {
	this->baudRate = baudRate;
	this->dataBits = dataBits;
	this->parity = parity;
	this->stopBits = stopBits;
	_beginthread(threadFtn, 0, this);
}

bool ComPortManager::endThread() {
	if (!SetEvent(killEvent))
		return logAndQuit("Failed to set killEvent");
	return true;
}

void ComPortManager::threadFtn(void* threadArg) {
	ComPortManager* cpmptr = (ComPortManager*)threadArg;
	if (!cpmptr->connect()) {
		PostMessage(cpmptr->mPaneWindow, WM_THREAD_DOWN, 0, 0);
		return;
	}
	cpmptr->threadContext = init();
	cpmptr->loop();
	term(cpmptr->threadContext);
	cpmptr->disconnect();
}

void ComPortManager::handleRead(const BYTE* buffer, DWORD numChars) {
	size_t mComPortNameLen = strlen(mComPortName) + 2;
	DWORD numWritten;
	DWORD hexBufferSize = mComPortNameLen + 3 * numChars;
	char* hexBuffer = (char*)malloc(sizeof(char) * (hexBufferSize + 1));
	if (hexBuffer == NULL)
		log("malloc failed");
	hexBuffer[hexBufferSize] = '\0';
	strcpy(hexBuffer, mComPortName);
	hexBuffer[mComPortNameLen - 2] = ':';
	hexBuffer[mComPortNameLen - 1] = ' ';

	for (unsigned i = 0; i < numChars; i++) {
		sprintf_s(mComPortNameLen + hexBuffer + 3 * i, 3, "%02x", (unsigned char)buffer[i] & 0xff);
		if (i != numChars - 1)
			hexBuffer[mComPortNameLen + 3 * i + 2] = ':';
		else
			hexBuffer[mComPortNameLen + 3 * i + 2] = '\n';
	}
	if (!WriteFile(hFile, hexBuffer, hexBufferSize, &numWritten, NULL))
		log("WriteFile returned false");
	log(hexBuffer);
	readHook(threadContext, hexBuffer, hexBufferSize);
	PostMessage(mPaneWindow, WM_THREAD_RECV, (WPARAM)hexBuffer, hexBufferSize);
}

void ComPortManager::handleWrite(int errorCode, const char* errorMsg) {
	log(errorMsg);
	writeCallback(threadContext, errorCode, errorMsg);
	if (errorCode == 0)
		PostMessage(mPaneWindow, WM_THREAD_SENT, 0, 0);
}

bool ComPortManager::connect() {
	BOOL fSuccess;
	//  Open a handle to the specified com port.
	hCom = CreateFile(mComPortNameW,
		GENERIC_READ | GENERIC_WRITE,
		0,      //  must be opened with exclusive-access
		NULL,   //  default security attributes
		OPEN_EXISTING, //  must use OPEN_EXISTING
		FILE_FLAG_OVERLAPPED,      //  can't write in one thread while blocked for a read in another unless port is declared overlapped
		NULL); //  hTemplate must be NULL for comm devices
	if (hCom == INVALID_HANDLE_VALUE)
		return logAndQuit("Failed to create hCom handle");

	//By default, all bytes received are considered part of a single, long message
	COMMTIMEOUTS commTimeOuts;
	SecureZeroMemory(&commTimeOuts, sizeof(COMMTIMEOUTS));
	//"only bytes received within 1 ms of each other are considered part of the same message"
	commTimeOuts.ReadIntervalTimeout = 1;
	fSuccess = SetCommTimeouts(hCom, &commTimeOuts);
	if (!fSuccess) {
		log("SetCommTimeouts failed");
		CloseHandle(hCom);
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
	fSuccess = SetCommState(hCom, &dcb);
	if (!fSuccess) {
		log("SetCommState failed");
		CloseHandle(hCom);
		return false;
	}

	//Serial port is up and running
	log("Serial Port successfully reconfigured");

	//Create output file
	hFile = CreateFileW(mFilePathW,  // name of the file to write
		FILE_APPEND_DATA,          // open for appending
		0,							// no sharing
		NULL,                      // default security
		OPEN_ALWAYS,               // open existing file or create new file 
		FILE_ATTRIBUTE_NORMAL,     // normal file
		NULL);                     // no attr. template
	if (hFile == INVALID_HANDLE_VALUE) {
		char buff[256];
		sprintf(buff, "Failed to create hFile handle: %d", GetLastError());
		log(buff);
		log("The filename cannot be \"comx.y\", where x is a digit and y is a file extension");
		CloseHandle(hCom);
		return false;
	}

	//Everything good
	PostMessage(mPaneWindow, WM_THREAD_UP, 0, 0);
	return true;
}

void ComPortManager::loop() {
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
				log("killEvent was signaled. Exiting loop");
				if (!ResetEvent(killEvent))
					log("failed to reset killEvent");
				break;
			}
			else if (result != WAIT_TIMEOUT) {
				//some error occured
				log("WaitForSingleObject returned an error");
				break;
			}
			//Otherwise, there was no kill request, continue as normal.
			//Attempt to create new read request
			osReader = { 0 };
			osReader.hEvent = CreateEvent(NULL, true, false, NULL);
			//read event failed to allocate, return:
			if (osReader.hEvent == NULL) {
				log("failed to create readEvent");
				break;
			}
			eventArray[1] = osReader.hEvent;
			//Execute the asynchronous read:
			if (!ReadFile(hCom, readBuffer, BUFFSIZE, &bytesRead, &osReader)) {
				//The asynchronous read request succeeded and will be completed later:
				if (GetLastError() == ERROR_IO_PENDING) {
					log("Read request queued and pending");
					fWaitingOnRead = TRUE;
				}
				//The asynchronous read request failed:
				else {
					log("ReadFile returned an error");
					CloseHandle(osReader.hEvent);
					break;
				}
			}
			//The asynchronous read request succeeded and completed immediately:
			else {
				log("Read request queued and returned immediately");
				CloseHandle(osReader.hEvent);
				handleRead(readBuffer, bytesRead);
			}
		}
		//We are waiting on an outstanding read request:
		else {
			//block until a signal arrives
			//if we are waiting on a write, then we block for a write response
			//otherwise, we wait for a write request
			if (fWaitingOnWrite)
				log("\tblocking for kills, reads, and writeReponses.");
			else
				log("\tblocking for kills, reads, and writeRequests.");
			result = WaitForMultipleObjects(3, eventArray, FALSE, INFINITE);
			continueLooping = false;
			switch (result) {
				//The killEvent handle received a signal. This has priority over every other signal.
			case WAIT_OBJECT_0:
				log("Received killEvent. Exiting loop");
				if (!ResetEvent(killEvent))
					log("failed to reset killEvent");
				break;

				//The com port handle received a signal
			case WAIT_OBJECT_0 + 1:
				log("received read event");
				//Unsuccessful read
				if (!GetOverlappedResult(hCom, &osReader, &bytesRead, FALSE)) {
					log("GetOverlappedResult returned an error");
					break;
				}
				//Successful read, continue looping
				log("Outstanding read request fulfilled");
				handleRead(readBuffer, bytesRead);
				fWaitingOnRead = FALSE;
				CloseHandle(osReader.hEvent);
				continueLooping = true;
				break;

				//This is either a request for a write, or the response from a previous write:
			case WAIT_OBJECT_0 + 2:
				//response from previous write:
				if (fWaitingOnWrite) {
					log("received write response event");
					//Unsuccessful write
					if (!GetOverlappedResult(hCom, &osWriter, &bytesWritten, FALSE)) {
						handleWrite(1, "GetOverlappedResult returned an error");
						break;
					}
					//Successful write, continue looping
					handleWrite(0, "Outstanding write request fulfilled");
					fWaitingOnWrite = FALSE;
					CloseHandle(osWriter.hEvent);
					continueLooping = true;
					eventArray[2] = writeRequestEvent;
					break;
				}
				//request for a write:
				else {
					log("received write request event");
					//retrieve message from queue:
					volatile WriteArg* writeArgPtr = messageQueue.pop();
					if (writeArgPtr != NULL) {
						if (writeArgPtr->numChars > BUFFSIZE) {
							log("message too large. Truncating message");
							numChars = BUFFSIZE;
						}
						else
							numChars = writeArgPtr->numChars;
						strncpy((char*)writeBuffer, writeArgPtr->buffer, BUFFSIZE);
						delete writeArgPtr;
					}
					//unset the event flag:
					if (!ResetEvent(writeRequestEvent)) {
						handleWrite(1, "failed to reset writeRequestEvent");
						break;
					}
					//flush reads and writes to memory - if the messageQueue is empty after the flush, set the flag.
					//the order of operations here is intentional - unset the flag, flush, set if not empty.
					if (!messageQueue.flush_checkEmpty())
						if (!SetEvent(writeRequestEvent)) {
							handleWrite(1, "failed to set writeRequestEvent");
							break;
						}

					//attempt to create writeResponseEvent:
					osWriter = { 0 };
					osWriter.hEvent = CreateEvent(NULL, true, false, NULL);
					if (osWriter.hEvent == NULL) {
						handleWrite(1, "failed to create writeResponseEvent");
						break;
					}
					eventArray[2] = osWriter.hEvent;
					//execute the asynchronous write:
					if (!WriteFile(hCom, writeBuffer, numChars, &bytesWritten, &osWriter)) {
						//The asynchronous write request succeeded and will be completed later:
						if (GetLastError() == ERROR_IO_PENDING) {
							log("Write request queued and pending");
							fWaitingOnWrite = true;
							continueLooping = true;
							break;
						}
						//The asynchronous write request failed:
						else {
							handleWrite(1, "WriteFile returned an error");
							CloseHandle(osWriter.hEvent);
							break;
						}
					}
					//The asynchronous write request succeeded and completed immediately
					else {
						handleWrite(0, "Write request queued and returned immediately");
						CloseHandle(osWriter.hEvent);
						continueLooping = true;
						eventArray[2] = writeRequestEvent;
						break;
					}
				}
				// Error in WaitForMultipleObjects()
			default:
				log("WaitForMultipleObjects returned an error");
				break;
			}
		}
	}
	//Exited loop
	CancelIo(hCom);
	if (fWaitingOnRead)
		CloseHandle(osReader.hEvent);
	if (fWaitingOnWrite) {
		handleWrite(1, "Thread exited before write could complete");
		CloseHandle(osWriter.hEvent);
	}
}

void ComPortManager::disconnect() {
	CloseHandle(hCom);
	CloseHandle(hFile);
	PostMessage(mPaneWindow, WM_THREAD_DOWN, 0, 0);
}


void ComPortManager::writeRaw(const char* buffer, int numChars) {
	handleRead((const BYTE*)buffer, numChars);
	WriteArg writeArg(buffer, numChars);
	messageQueue.produce(writeArg);
	if (!SetEvent(writeRequestEvent))
		handleWrite(1, "failed to set writeRequestEvent");
}

int getNibble(char input) {
	if (input >= '0' && input <= '9')
		return input - '0';
	else if (input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	else if (input >= 'a' && input <= 'f')
		return input - 'a' + 10;
	else
		return -1;
}

bool ComPortManager::writeHex(const char* buffer, int numChars) {
	log(buffer);
	if (numChars % 2 == 1)
		return false;
	int numBytes = numChars / 2;
	char* newBuffer = (char*)malloc(sizeof(char)*numBytes);
	for (int k = 0; k < numBytes; k++) {
		int lowerNibble = getNibble(buffer[k * 2 + 1]);
		int upperNibble = getNibble(buffer[k * 2]);
		if (lowerNibble < 0 || upperNibble < 0) {
			free(newBuffer);
			return false;
		}
		newBuffer[k] = lowerNibble;
		newBuffer[k] += 16 * upperNibble;
		unsigned wholeByte = newBuffer[k] & 0x00ff;
		char here[5];
		sprintf(here, "%x", wholeByte);
		log(here);
	}
	this->writeRaw(newBuffer, numBytes);
	free(newBuffer);
	return true;
}

int ComPortManager::getPortNum() {
	return comPortNum;
}
void ComPortManager::getFileName(char* buffer) {
	strcpy(buffer, mFilePath);
}
void ComPortManager::getPortName(char* buffer) {
	strcpy(buffer, mComPortName);
}
HWND* ComPortManager::getPaneWindowPtr() {
	return &mPaneWindow;
}
/*
	Not quite sure how to write this so its callable from c code, but I know its possible.
*/
void writeRequest(void* objectPtr, const char* buffer, int numChars) {
	((ComPortManager*)objectPtr)->writeRaw(buffer, numChars);
}