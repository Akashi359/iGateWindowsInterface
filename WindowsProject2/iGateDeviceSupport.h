#pragma once

/*
	Defined by C code.
	Called by thread code to indicate that a new message has arrived over the com port.
*/
void readHook(void* threadContext, char* buffer, size_t size);

/*
	Defined by C code.
	Called by thread code to indicate the success or failure of a previous call to writeRequest()
*/
void writeCallback(void* threadContext, bool succeeded, const char* errorMsg);

/*
	Defined by C code.
	Called by thread code upon startup to provide the C code an initialization hook.
	Receives a function which the thread code may call to write to the com port.
	Returns a threadContext object that must be passed to all other functions defined by the C code.
*/
void* init();

/*
	Defined by C code.
	Called by thread code upon shutdown to provide the C code a termination hook.
*/
void term(void* threadContext);

/*
	Defined by C++ code.
	Called by C code to execute a write on the Comport.
*/
void writeRequest(void* objectPtr, char* buffer, int size);
