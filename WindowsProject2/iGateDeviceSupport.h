#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
	Defined by C code.
	Called by wrapper code to indicate that a new message has arrived over the com port.
*/
void readHook(void* threadContext, char* buffer, int size);

/*
	Defined by C code.
	Called by wrapper code upon startup. Provides the C code an opportunity to perform any of its own setup.
	Returns a threadContext object that must be passed to all other functions defined by the C code.
	The C code must save the argument passed to it in order to request a write.
*/
void* init(void* objectPtr);

/*
	Defined by C code.
	Called by thread code upon shutdown to provide the C code a termination hook.
*/
void term(void* threadContext);

/*
	Defined by C++ code.
	Called by C code to execute a write on the Comport.
	The first argument passed to writeRequest must be the argument saved by init().

	Writes complete asynchronously, so success or failure is reported via writeCallback() instead of this function.
*/
void writeRequest(void* objectPtr, char* buffer, int size);

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
	We're still debating whether or not to implement the following functions - it depends on whether the
	C code or the wrapper code will be responsible for managing internet connection state.

	If the internet connection logic is the same for every wrapper, then it would make sense to put that functionality into
	the C code, so that we don't have to write duplicate code every time.
	
	/
		Defined by C++ code.
		Called by C code to request that the wrapper open some sort of internet socket.
		The first argument passed to openRequest must be the argument saved by init().

		Open completes asynchronously, so success or failure is reported via openCallback() instead of this function.
	/
	void openRequest(void* objectPtr, other stuff here);

	/
		Defined by C code.
		Called by wrapper code to indicate the success or failure of a previous call to openRequest()
	/
	void openCallback(void* threadContext, bool succeeded, const char* errorMsg);

	/
		Defined by C code.
		Called by C code to request that the wrapper close the internet socket created by a previous call to open().
		The first argument passed to closeRequest must be the argument saved by init().

		Close completes asynchronously, so success or failure is reported via closeCallback() instead of this function.
	/
	void closeRequest(void* objectPtr, other stuff here);

	/
		Defined by C code
		Called by wrapper code to indicate the success or failure of a previous call to closeRequest()
		Also called by wrapper code to indicate that an error has forced the socket to close.
	/
	void closeCallback(void* threadContext, bool succeeded, const char* errorMsg);
*/
////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
}
#endif