#pragma once
#include "ComPortManager.h"

class PaneInfo {
	private:
		ComPortManager* cpmPtr;
		unsigned port;
		wchar_t* file;
		char* dest;
		char* command;

		PaneInfo(const PaneInfo &src);
		PaneInfo& operator=(const PaneInfo &src);

	public:
		int connectionState;
		bool doClose;

		static const int CONNECTED = 0;
		static const int DISCONNECTED = 1;
		static const int WAITING = 2;

		ComPortManager* getCPMptr();
		unsigned getPortNum();
		wchar_t* getFileName();
		char* getDest();
		char* getCommand();

		void setDest(char*);
		void setCommand(char*);

		~PaneInfo();
		PaneInfo(unsigned comPortNum, const wchar_t* fileName, HWND);
};