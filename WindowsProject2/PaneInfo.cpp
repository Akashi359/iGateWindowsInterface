#include "PaneInfo.h"
#include "cpyalloc.h"
#include "ControlIdentifiers.h"

////////////////////////////////////////////////////////////////
//Special functions
////////////////////////////////////////////////////////////////
PaneInfo::PaneInfo(unsigned comPortNum, const wchar_t* fileName, HWND paneWindow) {
	baudRate_i = 4;
	dataBits_i = 4;
	stopBits_i = 0;
	parity_i = 0;
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

////////////////////////////////////////////////////////////////
//Other
////////////////////////////////////////////////////////////////

const uint32_t PaneInfo::BAUDRATE_VAL_ARR[PaneInfo::BAUDRATE_I_COUNT] = { 9600,19200,38400,57600,115200 };
const uint32_t PaneInfo::DATABITS_VAL_ARR[PaneInfo::DATABITS_I_COUNT] = { 4,5,6,7,8 };
const uint32_t PaneInfo::STOPBITS_VAL_ARR[PaneInfo::STOPBITS_I_COUNT] = { ONESTOPBIT, ONE5STOPBITS, TWOSTOPBITS };
const uint32_t PaneInfo::PARITY_VAL_ARR[PaneInfo::PARITY_I_COUNT] = { NOPARITY, ODDPARITY, EVENPARITY, MARKPARITY, SPACEPARITY };

const wchar_t* PaneInfo::BAUDRATE_NAME_ARR[PaneInfo::BAUDRATE_I_COUNT] = {L"9600", L"19200", L"38400",L"57600",L"115200"};
const wchar_t* PaneInfo::DATABITS_NAME_ARR[PaneInfo::DATABITS_I_COUNT] = {L"Four", L"Five", L"Six",L"Seven",L"Eight"};
const wchar_t* PaneInfo::STOPBITS_NAME_ARR[PaneInfo::STOPBITS_I_COUNT] = { L"One", L"One and a Half", L"Two" };
const wchar_t* PaneInfo::PARITY_NAME_ARR[PaneInfo::PARITY_I_COUNT] = {L"None", L"Odd", L"Even",L"Mark",L"Space"};


void PaneInfo::initPane(HWND paneWnd, int paneWidth, int avgWidth, int height) {
	int hMargin = 2 * avgWidth;
	int labelWidth = 12 * avgWidth;
	int hTab = paneWidth / 2 - 3 * avgWidth;
	int contentWidth = paneWidth - hMargin - hTab;
	int buttonWidth = 60;
	int currHeight = 1; //vertical margin
	CreateWindow(L"Static", L"", WS_VISIBLE | WS_CHILD | SS_CENTER, paneWidth / 2 - 6 * avgWidth, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_PORTNAME_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"Status:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_STATUS_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_STATUS_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"FileName:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_FILENAME_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_FILENAME_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Button", L"test", WS_VISIBLE | WS_CHILD, paneWidth - hMargin - buttonWidth, currHeight*height, buttonWidth, height, paneWnd, (HMENU)ID_CONNECT_BUTTON, 0, 0);
	currHeight += 2;
	CreateWindow(L"Static", L"Dest:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_DEST_LABEL, 0, 0);
	currHeight++;
	CreateWindow(L"Edit", L"Enter text:", WS_VISIBLE | WS_CHILD | WS_BORDER, hMargin, currHeight*height, paneWidth - 2 * hMargin, height, paneWnd, (HMENU)ID_DEST_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"Command:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_COMMAND_LABEL, 0, 0);
	currHeight++;
	CreateWindow(L"Edit", L"Enter text:", WS_VISIBLE | WS_CHILD | WS_BORDER, hMargin, currHeight*height, paneWidth - 2 * hMargin, height, paneWnd, (HMENU)ID_COMMAND_LABEL, 0, 0);
	currHeight++;
	CreateWindow(L"Button", L"Write", WS_VISIBLE | WS_CHILD, paneWidth - hMargin - buttonWidth, currHeight*height, buttonWidth, height, paneWnd, (HMENU)ID_WRITE_BUTTON, 0, 0);
	currHeight += 2;
	CreateWindow(L"Static", L"BaudRate:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_BAUDRATE_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_BAUDRATE_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"DataBits:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_DATABITS_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_DATABITS_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"StopBits:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_STOPBITS_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_STOPBITS_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Static", L"Parity:", WS_VISIBLE | WS_CHILD, hMargin, currHeight*height, labelWidth, height, paneWnd, (HMENU)ID_PARITY_LABEL, 0, 0);
	CreateWindow(L"Static", L"test", WS_VISIBLE | WS_CHILD, hTab, currHeight*height, contentWidth, height, paneWnd, (HMENU)ID_PARITY_CONTENT, 0, 0);
	currHeight++;
	CreateWindow(L"Button", L"Settings", WS_VISIBLE | WS_CHILD, paneWidth - hMargin - buttonWidth, currHeight*height, buttonWidth, height, paneWnd, (HMENU)ID_SETTINGS_BUTTON, 0, 0);
}

void PaneInfo::populatePane(HWND paneWnd) {
	size_t itowSize = 0;
	for (unsigned k = port; k > 0; k /= 10)
		itowSize++;
	wchar_t* temp = (wchar_t*)malloc(sizeof(wchar_t)*(4 + itowSize));
	wcscpy(temp, L"COM");
	_itow(port, temp + 3, 10);
	SetDlgItemText(paneWnd, ID_PORTNAME_CONTENT, temp);
	free(temp);

	temp = (wchar_t*)malloc(sizeof(wchar_t)*(wcslen(file) + 1));
	wcscpy(temp, file);
	SetDlgItemText(paneWnd, ID_FILENAME_CONTENT, temp);
	free(temp);

	switch (connectionState) {
	case CONNECTED:
		SetDlgItemText(paneWnd, ID_STATUS_CONTENT, L"CONNECTED");
		SetDlgItemText(paneWnd, ID_CONNECT_BUTTON, L"Disconnect");
		break;
	case DISCONNECTED:
		SetDlgItemText(paneWnd, ID_STATUS_CONTENT, L"DISCONNECTED");
		SetDlgItemText(paneWnd, ID_CONNECT_BUTTON, L"Connect");
		break;
	case WAITING:
		SetDlgItemText(paneWnd, ID_STATUS_CONTENT, L"WAITING");
		SetDlgItemText(paneWnd, ID_CONNECT_BUTTON, L"Connect");
		break;
	default:
		SetDlgItemText(paneWnd, ID_STATUS_CONTENT, L"status not recognized");
		SetDlgItemText(paneWnd, ID_CONNECT_BUTTON, L"????");
	}

	SetDlgItemText(paneWnd, ID_BAUDRATE_CONTENT, BAUDRATE_NAME_ARR[baudRate_i]);
	SetDlgItemText(paneWnd, ID_DATABITS_CONTENT, DATABITS_NAME_ARR[dataBits_i]);
	SetDlgItemText(paneWnd, ID_STOPBITS_CONTENT, STOPBITS_NAME_ARR[stopBits_i]);
	SetDlgItemText(paneWnd, ID_PARITY_CONTENT, PARITY_NAME_ARR[parity_i]);
}