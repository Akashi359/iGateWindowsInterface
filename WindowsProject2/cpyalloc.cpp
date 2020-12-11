#include <string.h>
#include <stdlib.h>
#include "cpyalloc.h"

char* cpyalloc(const char* src, size_t maxChars) {
	char* newBuffer = (char*)malloc(sizeof(char)*(maxChars + 1));
	newBuffer[maxChars] = '\0';
	strncpy(newBuffer, src, maxChars);
	return newBuffer;
}

wchar_t* cpyalloc(const wchar_t* src, size_t maxChars) {
	wchar_t* newBuffer = (wchar_t*)malloc(sizeof(wchar_t)*(maxChars + 1));
	newBuffer[maxChars] = '\0';
	wcsncpy(newBuffer, src, maxChars);
	return newBuffer;
}

char* cpyalloc(const char* src) {
	size_t len = strlen(src);
	char* newBuffer = (char*)malloc(sizeof(char)*(len + 1));
	strcpy(newBuffer, src);
	return newBuffer;
}

wchar_t* cpyalloc(const wchar_t* src) {
	size_t len = wcslen(src);
	wchar_t* newBuffer = (wchar_t*)malloc(sizeof(wchar_t)*(len + 1));
	wcscpy(newBuffer, src);
	return newBuffer;
}