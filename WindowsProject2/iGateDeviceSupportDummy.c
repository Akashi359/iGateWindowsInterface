/*
	This file does nothing and exists so that the project compiles.
	Replace with C file that defines the four C prototypes declared in iGatDeviceSupport.h
*/
#include "iGateDeviceSupport.h"

void readHook(void* threadContext, char* buffer, int size) {}

void writeCallback(void* threadContext, int errorCode, const char* errorMsg) {}

void* init() {
	return 0;
}

void term(void* threadContext) {}