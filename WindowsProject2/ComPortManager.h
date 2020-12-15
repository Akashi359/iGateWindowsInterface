#pragma once
#include <Windows.h>
#include <stdint.h>
#include "Queue.h"

class WriteArg {
public:
	char* buffer;
	int numChars;
	WriteArg(const char* buffer, int numChars);
	WriteArg(const WriteArg &src);
	WriteArg& operator=(const WriteArg &src);
	~WriteArg();
};

class ComPortManager {
private:
	static const char* TAG;
	Queue<WriteArg> messageQueue;
	HWND mPaneWindow;
	unsigned comPortNum;
	wchar_t* mComPortNameW;
	char* mComPortName;
	wchar_t* mFilePathW;
	char* mFilePath;
	HANDLE killEvent, writeRequestEvent;
	HANDLE hCom, hFile;
	uint32_t baudRate_i, dataBits_i, parity_i, stopBits_i;
	void* threadContext;

	bool connect();
	void loop();
	void disconnect();
	void handleWrite(int, const char*);

	static void threadFtn(void*);
	static void log(const char* errorMsg);
	static bool logAndQuit(const char* errorMsg);

	ComPortManager(const ComPortManager &src);
	ComPortManager& operator=(const ComPortManager &src);

public:
	void handleRead(const BYTE*, DWORD);
	ComPortManager(unsigned comPortNum, const wchar_t* fileName, HWND uiWindow);
	~ComPortManager();
	void startThread(uint32_t baudRate_i, uint32_t dataBits_i, uint32_t parity_i, uint32_t stopBits_i);
	bool endThread();
	/*
		It is the caller's responsibility to free the buffer.
		The buffer may be freed immediately.
	*/
	void writeRaw(const char* buffer, int numChars);
	bool writeHex(const char* buffer, int numChars);

	int getPortNum();
	void getPortName(char* buffer);
	void getFileName(char* buffer);

	HWND* getPaneWindowPtr();
};