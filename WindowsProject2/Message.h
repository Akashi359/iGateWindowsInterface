#pragma once
#pragma once
#include <stdint.h>

size_t getMessage(const char* dest, const char* command, int commandLen, uint8_t*&);

char* getAddress(const char* msg);

int count(const char*, char, unsigned);