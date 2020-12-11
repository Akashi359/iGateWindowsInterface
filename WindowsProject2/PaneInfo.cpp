#include "PaneInfo.h"
#include "cpyalloc.h"

////////////////////////////////////////////////////////////////
//Special functions
////////////////////////////////////////////////////////////////
PaneInfo::PaneInfo(unsigned comPortNum, const wchar_t* fileName, HWND paneWindow) {
	connectionState = DISCONNECTED;
	doClose = false;
	port = comPortNum;
	cpmPtr = new ComPortManager(comPortNum, fileName, paneWindow);
	dest = nullptr;
	command = nullptr;
	file = cpyalloc(fileName);
}

PaneInfo::~PaneInfo() {
	delete(cpmPtr);
	free(file);
	free(dest);
	free(command);
}

////////////////////////////////////////////////////////////////
//Getters and Setters
////////////////////////////////////////////////////////////////
ComPortManager* PaneInfo::getCPMptr() { return cpmPtr; }

unsigned PaneInfo::getPortNum() { return port; }

wchar_t* PaneInfo::getFileName() { return file; }

char* PaneInfo::getDest() { return dest; }

char* PaneInfo::getCommand() { return command; }

void PaneInfo::setDest(char* newDest) {
	free(dest);
	dest = cpyalloc(newDest);
}

void PaneInfo::setCommand(char* newCommand) {
	free(command);
	command = cpyalloc(newCommand);
}