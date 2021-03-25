#include "PaneInfo.h"
#include "ByteBuffer.h"
#include "cpyalloc.h"
#include "ControlIdentifiers.h"
#include "stdio.h"
extern "C" {
	#include "iGateDeviceSupport.h"
}

////////////////////////////////////////////////////////////////
//Special functions
////////////////////////////////////////////////////////////////

PaneInfo::PaneInfo() {
	baudRate_i = 4;
	dataBits_i = 4;
	stopBits_i = 0;
	parity_i = 0;
	connectionState = DISCONNECTED;
	file = nullptr;
	dest = nullptr;
	command = nullptr;
}

PaneInfo* PaneInfo::initialize(unsigned comPortNum, const wchar_t* fileName, HWND inputWindow) {
	PaneInfo* temp = new PaneInfo();
	temp->port = comPortNum;
	temp->inputWindow = inputWindow;
	temp->file = cpyalloc(fileName);
	if (!temp->file){
		delete(temp);
		sprintf(errorMsg, "malloc failure in call to PaneInfo::initialize()");
		errorNum = BAD_ALLOC;
		return nullptr;
	}

	//Assemble file path from file name:
	size_t filePathLen = GetCurrentDirectory(0, NULL) + wcslen(fileName) + 1; //+1 for additional slash character
	if (!filePathLen) {
		delete(temp);
		sprintf(errorMsg, "GetCurrentDirectory(0, NULL) failure in PaneInfo::initialize()");
		errorNum = BAD_ALLOC;
		return nullptr;
	}
	wchar_t* filePathW = (wchar_t*)malloc(sizeof(wchar_t)*(filePathLen + 1)); //+1 again for terminating null character
	if (!filePathW) {
		delete(temp);
		sprintf(errorMsg, "malloc() failure in PaneInfo::initialize()");
		errorNum = BAD_ALLOC;
		return nullptr;
	}
	if (!GetCurrentDirectoryW(filePathLen, filePathW)) {
		free(filePathW);
		delete(temp);
		sprintf(errorMsg, "GetCurrentDirectory() failure in PaneInfo::initialize()");
		errorNum = BAD_ALLOC;
		return nullptr;
	}
	wcscat(filePathW, L"\\");
	wcscat(filePathW, fileName);

	//Create output file
	temp->hFile = CreateFileW(filePathW,  // name of the file to write
		FILE_APPEND_DATA,          // open for appending
		0,							// no sharing
		NULL,                      // default security
		OPEN_ALWAYS,               // open existing file or create new file 
		FILE_ATTRIBUTE_NORMAL,     // normal file
		NULL);                     // no attr. template
	free(filePathW);
	if (temp->hFile == INVALID_HANDLE_VALUE) {
		delete(temp);
		sprintf(errorMsg, "Failed to create hFile handle");
		errorNum = BAD_ALLOC;
		return nullptr;
	}

	return temp;
}

PaneInfo::~PaneInfo() {
	CloseHandle(hFile);
	free(file);
	delete(dest);
	delete(command);
}

////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////

const char* PaneInfo::getErrorMsg() { return errorMsg; }
int PaneInfo::getErrorNum() { return errorNum; }
unsigned PaneInfo::getPortNum() { return port; }
const wchar_t* PaneInfo::getFileName() { return file; }
ByteBuffer* PaneInfo::getCommand() { return command; }
ByteBuffer* PaneInfo::getDest() { return (ByteBuffer*)dest; }

void PaneInfo::setCommand(ByteBuffer* newCommand) {
	delete(command);
	command = newCommand;
}

void PaneInfo::setDest(ByteBuffer* newDest) {
	ByteBuffer* oldDest = (ByteBuffer*)InterlockedExchangePointer((volatile PVOID*)&dest, newDest);
	delete(oldDest);
}

////////////////////////////////////////////////////////////////
//Other
////////////////////////////////////////////////////////////////

const uint32_t PaneInfo::BAUDRATE_VAL_ARR[PaneInfo::BAUDRATE_I_COUNT] = { 9600,19200,38400,57600,115200 };
const uint32_t PaneInfo::DATABITS_VAL_ARR[PaneInfo::DATABITS_I_COUNT] = { 4,5,6,7,8 };
const uint32_t PaneInfo::STOPBITS_VAL_ARR[PaneInfo::STOPBITS_I_COUNT] = { ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS };
const uint32_t PaneInfo::PARITY_VAL_ARR[PaneInfo::PARITY_I_COUNT] = { NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY, SPACEPARITY };

const wchar_t* PaneInfo::BAUDRATE_NAME_ARR[PaneInfo::BAUDRATE_I_COUNT] = {L"9600", L"19200", L"38400",L"57600",L"115200"};
const wchar_t* PaneInfo::DATABITS_NAME_ARR[PaneInfo::DATABITS_I_COUNT] = {L"Four", L"Five", L"Six",L"Seven",L"Eight"};
const wchar_t* PaneInfo::STOPBITS_NAME_ARR[PaneInfo::STOPBITS_I_COUNT] = { L"One", L"One and a Half", L"Two" };
const wchar_t* PaneInfo::PARITY_NAME_ARR[PaneInfo::PARITY_I_COUNT] = {L"None", L"Odd", L"Even",L"Mark",L"Space"};


void PaneInfo::initPane(HWND paneWnd, int paneWidth, int avgWidth, int height) {
	//Child Windows are destroyed when the parent window is destroyed
	int hMargin = 2 * avgWidth;
	int labelWidth = 12 * avgWidth;
	int hTab = paneWidth / 2 - 3 * avgWidth;
	int contentWidth = paneWidth - hMargin - hTab;
	int buttonWidth = 100;
	int currHeight = 1; //vertical margin
	CreateWindow(L"Static", L"", WS_VISIBLE | WS_CHILD | SS_CENTER, paneWidth / 2 - 6 * avgWidth, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_PORTNAME_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"Status:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_STATUS_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_STATUS_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"FileName:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_FILENAME_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_FILENAME_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Button", L"test", WS_VISIBLE | WS_CHILD, paneWidth - hMargin - buttonWidth, currHeight*height, buttonWidth, height, paneWnd, (HMENU)ID_CONNECT_BUTTON, 0, 0);
	currHeight += 2;
	CreateWindow(L"Static", L"Dest:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_DEST_LABEL, 0, 0);
	CreateWindow(L"Static", L"<none>", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_DEST_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"Command:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_COMMAND_LABEL, 0, 0);
	CreateWindow(L"Static", L"<none>", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_COMMAND_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Button", L"Msg Settings", WS_VISIBLE | WS_CHILD, paneWidth - hMargin - buttonWidth, currHeight*height, buttonWidth, height, paneWnd, (HMENU)ID_MSGSETTINGS_BUTTON, 0, 0);
	currHeight++;
	CreateWindow(L"Button", L"Write", WS_VISIBLE | WS_CHILD, paneWidth - hMargin - buttonWidth, currHeight*height, buttonWidth, height, paneWnd, (HMENU)ID_WRITE_BUTTON, 0, 0);
	currHeight += 2;
	CreateWindow(L"Static", L"BaudRate:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_BAUDRATE_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_BAUDRATE_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"DataBits:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_DATABITS_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_DATABITS_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"StopBits:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_STOPBITS_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_STOPBITS_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"Parity:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_PARITY_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_PARITY_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Button", L"Port Settings", WS_VISIBLE | WS_CHILD, paneWidth - hMargin - buttonWidth, currHeight*height, buttonWidth, height, paneWnd, (HMENU)ID_SETTINGS_BUTTON, 0, 0);
}

bool PaneInfo::drawPane(HWND paneWnd) {
	//attempt to allocate first:
	size_t itowSize = 0;
	for (unsigned k = port; k > 0; k /= 10)
		itowSize++;
	wchar_t* portNameW = (wchar_t*)malloc(sizeof(wchar_t)*(4 + itowSize));
	wchar_t* fileNameW = (wchar_t*)malloc(sizeof(wchar_t)*(wcslen(file) + 1));
	char* commandString = command->getString(10, ' ');
	char* destString = ((ByteBuffer*)dest)->getString(10, ' ');
	if (!portNameW || !fileNameW || !commandString || !destString) {
		free(portNameW);
		free(fileNameW);
		free(commandString);
		free(destString);
		sprintf(errorMsg, "malloc failure in call to PaneInfo::drawPane");
		errorNum = BAD_ALLOC;
		return false;
	}

	//portname
	wcscpy(portNameW, L"COM");
	_itow(port, portNameW + 3, 10);
	SetDlgItemText(paneWnd, ID_PORTNAME_CONTENT, portNameW);
	free(portNameW);
	//filename
	wcscpy(fileNameW, file);
	SetDlgItemText(paneWnd, ID_FILENAME_CONTENT, fileNameW);
	free(fileNameW);
	//command
	SetDlgItemTextA(paneWnd, ID_COMMAND_CONTENT, commandString);
	free(commandString);
	//dest
	SetDlgItemTextA(paneWnd, ID_DEST_CONTENT, destString);
	free(destString);
	//connectionState
	switch (connectionState) {
	case CONNECTED:
		SetDlgItemText(paneWnd, ID_STATUS_CONTENT, L"CONNECTED");
		SetDlgItemText(paneWnd, ID_CONNECT_BUTTON, L"Disconnect");
		break;
	case DISCONNECTED:
		SetDlgItemText(paneWnd, ID_STATUS_CONTENT, L"DISCONNECTED");
		SetDlgItemText(paneWnd, ID_CONNECT_BUTTON, L"Connect");
		break;
	case WAITING:
		SetDlgItemText(paneWnd, ID_STATUS_CONTENT, L"WAITING");
		SetDlgItemText(paneWnd, ID_CONNECT_BUTTON, L"Connect");
		break;
	default:
		SetDlgItemText(paneWnd, ID_STATUS_CONTENT, L"status not recognized");
		SetDlgItemText(paneWnd, ID_CONNECT_BUTTON, L"????");
	}
	//port settings
	SetDlgItemText(paneWnd, ID_BAUDRATE_CONTENT, BAUDRATE_NAME_ARR[baudRate_i]);
	SetDlgItemText(paneWnd, ID_DATABITS_CONTENT, DATABITS_NAME_ARR[dataBits_i]);
	SetDlgItemText(paneWnd, ID_STOPBITS_CONTENT, STOPBITS_NAME_ARR[stopBits_i]);
	SetDlgItemText(paneWnd, ID_PARITY_CONTENT, PARITY_NAME_ARR[parity_i]);

	return true;
}

bool PaneInfo::connect(char* errorMsg) {
	//Are we connected?
	if (InterlockedCompareExchange(&connectionState, WAITING, CONNECTED) == CONNECTED){
		//if so, then disconnect
		ThreadManager::endThread();
		return true;
	}
	//Are we disconnected?
	if (InterlockedCompareExchange(&connectionState, WAITING, DISCONNECTED) == DISCONNECTED) {
		//if so, then try to connect
		if (!ThreadManager::startThread(port, BAUDRATE_VAL_ARR[baudRate_i], DATABITS_VAL_ARR[dataBits_i], PARITY_VAL_ARR[parity_i], STOPBITS_VAL_ARR[stopBits_i], errorMsg)) {
			//if we fail to connect, then redisconnect
			InterlockedExchange(&connectionState, DISCONNECTED);
			return false;
		}
		return true;
	}
	//otherwise, we are waiting for an outstanding connect, disconnect, or kill and we do nothing:
	strcpy(errorMsg, "Must be CONNECTED to disconnect, or DISCONNECTED to connect");
	return false;

}

bool PaneInfo::startThread(char* errorMsg) {
	if (InterlockedCompareExchange(&connectionState, WAITING, DISCONNECTED) != DISCONNECTED) {
		strcpy(errorMsg, "Must be DISCONNECTED to connect");
		return;
	}
	if (!ThreadManager::startThread(port, BAUDRATE_VAL_ARR[baudRate_i], DATABITS_VAL_ARR[dataBits_i], PARITY_VAL_ARR[parity_i], STOPBITS_VAL_ARR[stopBits_i], errorMsg)) {
		InterlockedExchange(&connectionState, DISCONNECTED);
		return false;
	}
	return true;
}

bool PaneInfo::write(char* errorMsg) {
	if (connectionState != CONNECTED) {
		strcpy(errorMsg, "Must be CONNECTED before writing.");
		return false;
	}

	//retrieve buffers
	if (!dest || !command) {
		strcpy(errorMsg, "Must populate 'dest' and 'command' fields before writing.");
		return false;
	}
	int commandSize = command->getCount();

	//retrieve message
	ByteBuffer* message = ByteBuffer::getMessage(dest, command);
	if (!message) {
		strcpy(errorMsg, ByteBuffer::getErrorMsg());
		return false;
	}

	//perform the write
	bool retVal = ThreadManager::write(message);
	delete(message);
	if (!retVal) {
		strcpy(errorMsg, "ThreadManager Queue is full.");
		return false;
	}

	return true;
}

void PaneInfo::endThread() {
	if (InterlockedCompareExchange(&connectionState, WAITING, CONNECTED) != CONNECTED)
		return;
	ThreadManager::endThread();
}

void PaneInfo::killPane() {
	LONG oldState = InterlockedExchange(&connectionState, KILL);
	switch (oldState) {
	//The thread is currently dead with no outstanding requests to wake it,
	//therefore there can be no race conditions for this case.
	case DISCONNECTED:
		if (!PostMessage(inputWindow, WM_DESTROYPANE, (WPARAM)port, (LPARAM)nullptr))
			throw GetLastError();
		break;
	//Thread could potentially encounter an error and move to WAITING, then DISCONNECTED as a race condition,
	// but either way the thread will transition to DISCONNECTED while doClose = true and the WM_DESTROYPANE logic will trigger.
	case CONNECTED:
		ThreadManager::endThread();
		break;
	//This could either be WAITING->DISCONNECTED or WAITING->CONNECTED. The WM_DESTROYPANE logic will trigger on either transition so we do nothing.
	case WAITING:
		break;
	default:
		break;
	}
}



void PaneInfo::receive(unsigned msgType, void* payload) {
	switch (msgType) {
	case WM_THREAD_UP:
		threadContext = init(this);
		if (!PostMessage(inputWindow, msgType, (WPARAM)port, (LPARAM)nullptr))
			throw GetLastError();
		if (InterlockedCompareExchange(&connectionState, CONNECTED, WAITING) == KILL)
			ThreadManager::endThread();
		break;
	case WM_THREAD_DOWN:
		term(threadContext);
		if (!PostMessage(inputWindow, msgType, (WPARAM)port, (LPARAM)nullptr))
			throw GetLastError();
		if (InterlockedCompareExchange(&connectionState, DISCONNECTED, WAITING) == KILL)
			if (!PostMessage(inputWindow, WM_DESTROYPANE, (WPARAM)port, (LPARAM)nullptr))
				throw GetLastError();
		break;
	case WM_THREAD_RECV:
		ByteBuffer* byteBuffer = (ByteBuffer*)payload;
		//Pass buffer to c code
		readHook(threadContext, (char*)byteBuffer->getBinary(), byteBuffer->getCount());

		//Write buffer to the file
		char* buffer = byteBuffer->getString(10, ' ');
		//if we fail to malloc, don't crash just continue
		if (buffer == nullptr) {
			strcpy(errorMsg, "Failed to getString from incoming thread message in call to receive(WM_THREAD_RECV)");
			if (!PostMessage(inputWindow, WM_THREAD_ERROR, (WPARAM)port, (LPARAM)errorMsg))
				throw GetLastError();
		}
		else {
			DWORD numWritten;
			if (!WriteFile(hFile, buffer, strlen(buffer), &numWritten, NULL)) {
				free(buffer);
				strcpy(errorMsg, "Failed to write Incoming thread message to hFile");
				if (!PostMessage(inputWindow, WM_THREAD_ERROR, (WPARAM)port, (LPARAM)errorMsg))
					throw GetLastError();
			}
			else
				free(buffer);
		}

		//If the dest hasn't been populated yet, attempt to populate it using the message we just received
		if (dest == nullptr) {
			ByteBuffer* address = byteBuffer->getAddress();
			//if we fail to malloc, don't crash just continue
			if (address == nullptr) {
				strcpy(errorMsg, "Failed to getAddress from incoming thread message");
				if (!PostMessage(inputWindow, WM_THREAD_ERROR, (WPARAM)port, (LPARAM)errorMsg))
					throw GetLastError();
			}
			//if we don't fail to malloc, only replace the previous dest value if it isn't populated already
			else
				if (InterlockedCompareExchangePointer((volatile PVOID*)&dest, address, nullptr) != nullptr)
					delete(address);
		}

		//Pass buffer to uiThread for further processing
		if (!PostMessage(inputWindow, msgType, (WPARAM)port, (LPARAM)payload))
			throw GetLastError();
		break;
	case WM_THREAD_SENT:
		if (!PostMessage(inputWindow, msgType, (WPARAM)port, (LPARAM)nullptr))
			throw GetLastError();
		break;
	case WM_THREAD_ERROR:
		strcpy(errorMsg, (char*)payload);
		if (!PostMessage(inputWindow, msgType, (WPARAM)port, (LPARAM)errorMsg))
			throw GetLastError();
		InterlockedCompareExchange(&connectionState, WAITING, CONNECTED);
		break;
	default:
		strcpy(errorMsg, "Unknown msgType in call to PaneInfo::receive()");
		if (!PostMessage(inputWindow, WM_THREAD_ERROR, (WPARAM)port, (LPARAM)errorMsg))
			throw GetLastError();
		break;
	};
}