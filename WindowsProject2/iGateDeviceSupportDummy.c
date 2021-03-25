/*
	This file does nothing and exists so that the project compiles.
	Replace with C file that defines the four C prototypes declared in iGatDeviceSupport.h
*/
#include "iGateDeviceSupport.h"

void readHook(void* threadContext, char* buffer, int size) {}

void* init(void* objectPtr) {
	return 0;
}

void term(void* threadContext) {}