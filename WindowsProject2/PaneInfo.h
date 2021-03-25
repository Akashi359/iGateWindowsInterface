#pragma once
#include "Threadmanager.h"
#include "ByteBuffer.h"

//TODO: Replace comportmanager with threadmanager

/*
	Throws GetLastError() as an exception if inter-thread communication functions fail,
	which can occur in startThread(), write(), endThread(), or in receive() when posting messages to the ui thread.

	Com port errors are reported via PostMessage() with type WM_THREAD_ERROR.
*/
class PaneInfo : public ThreadManager {
	private:
		static const LONG CONNECTED = 0;
		static const LONG DISCONNECTED = 1;
		static const LONG WAITING = 2;
		static const LONG KILL = 3;

		thread_local static char errorMsg[128];	//static
		thread_local static int errorNum;		//static
		void* threadContext;					//allocated/deallocated by the c code
		HWND inputWindow;						//passed in as an argument and never deallocated
		unsigned port;							//primitive
		LONG volatile connectionState;			//primitive
		wchar_t* file;							//allocated/deallocated by static initialize() and destructor
		HANDLE hFile;							//allocated/deallocated by static initialize() and destructor
		ByteBuffer* command;					//allocated/deallocated by setCommand, deallocated by destructor
		ByteBuffer volatile*  dest;				//allocated/deallocated by setDest or receive(), deallocated by destructor

		void receive(unsigned, void*);

		PaneInfo(const PaneInfo &src);
		PaneInfo& operator=(const PaneInfo &src);

	protected:
		PaneInfo();

	public:
		//errornum kinda useless right now because every error I report this way is due to bad allocation
		const static int BAD_ALLOC = 1;

		uint32_t baudRate_i; //indices, 0 through COUNT -1
		uint32_t dataBits_i;
		uint32_t stopBits_i;
		uint32_t parity_i;

		//changing these numbers also requires a change to the WindowsProject2.rc file dropdown menu height, or else options will not show up.
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

		static const char* getErrorMsg();
		static int getErrorNum();
		unsigned getPortNum();
		const wchar_t* getFileName();
		ByteBuffer* getCommand();
		ByteBuffer* getDest();

		void setCommand(ByteBuffer* newCommand);
		void setDest(ByteBuffer* newDestinationValue);

		/*
			Returns a reference to a new PaneInfo object.
			Can fail if allocation fails or if the file fails to open.
		*/
		static PaneInfo* initialize(unsigned comPortNum, const wchar_t* fileName, HWND);

		/*
			Takes an HWND and formats it for use as a pane window
		*/
		static void initPane(HWND paneWnd, int paneWidth, int avgWidth, int height);

		/*
			Takes a pane window and loads it up with up-to-date contents.
		*/
		bool drawPane(HWND);

		/*
			Attempts to start the thread if its stopped.
			Attempts to end the thread if its running.
			Returns false on failure to execute and populates the argument with an error message.
			Could still fail asynchronously even when it returns true, via calls to receive().
		*/
		bool connect(char*);

		/*
			Attempts to start the thread.
			Returns false on failure and populates the argument with an error message.
		*/
		bool startThread(char*);

		/*
			Attempts to write to the port.
			Returns false on failure and populates the argument with an error message.
			May fail silently if the thread changes state while this function is executed.
		*/
		bool write(char*);

		/*
			Attempts to terminate the thread.
		*/
		void endThread();

		/*
			Checks to see if the thread is still running. If it is, kills it.
			Once the thread is confirmed to be not running, tells the uithread that the pane should be deallocated.
		*/
		void killPane();

		~PaneInfo();
};