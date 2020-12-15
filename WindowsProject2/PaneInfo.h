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
		uint32_t baudRate_i; //indexes, 0 through COUNT -1
		uint32_t dataBits_i;
		uint32_t stopBits_i;
		uint32_t parity_i;

		static const int CONNECTED = 0;
		static const int DISCONNECTED = 1;
		static const int WAITING = 2;
		//must also change the WindowsProject2.rc file dropdown menu height, or else options will not show up.
		static const uint32_t BAUDRATE_I_COUNT = 5;
		static const uint32_t DATABITS_I_COUNT = 5;
		static const uint32_t STOPBITS_I_COUNT = 3;
		static const uint32_t PARITY_I_COUNT = 5;

		static const uint32_t BAUDRATE_VAL_ARR[BAUDRATE_I_COUNT];
		static const uint32_t DATABITS_VAL_ARR[DATABITS_I_COUNT];
		static const uint32_t STOPBITS_VAL_ARR[STOPBITS_I_COUNT];
		static const uint32_t PARITY_VAL_ARR[PARITY_I_COUNT];

		static const wchar_t* BAUDRATE_NAME_ARR[BAUDRATE_I_COUNT];
		static const wchar_t* DATABITS_NAME_ARR[DATABITS_I_COUNT];
		static const wchar_t* STOPBITS_NAME_ARR[STOPBITS_I_COUNT];
		static const wchar_t* PARITY_NAME_ARR[PARITY_I_COUNT];

		ComPortManager* getCPMptr();
		unsigned getPortNum();
		wchar_t* getFileName();
		char* getDest();
		char* getCommand();

		void setDest(char*);
		void setCommand(char*);

		static void initPane(HWND, int, int, int);
		void populatePane(HWND);
		
		~PaneInfo();
		PaneInfo(unsigned comPortNum, const wchar_t* fileName, HWND);
};