// WindowsProject2.cpp : Defines the entry point for the application.
//

//TODO: finish add pane system
//TODO: finish delete pane system
//TODO: implement error popup.
//TODO: figure out how to use resizewindow() when the pane children don't have their own IDs. https://docs.microsoft.com/en-us/windows/win32/winmsg/window-features#child-windows

#include "framework.h"
#include "WindowsProject2.h"
#include "ComPortManager.h"
#include <stdint.h>

//constants
const uint16_t MAX_LOADSTRING = 100;
const WCHAR* OUTPUT_CLASSNAME = L"OutputScrollerClass";
const WCHAR* INPUT_CLASSNAME = L"InputScrollerClass";
const WCHAR* PANE_CLASSNAME = L"PaneClass";
const WCHAR* ADDPANE_CLASSNAME = L"AddPaneClass";
const WCHAR* REMOVEPANE_CLASSNAME = L"RemovePaneClass";
const uint16_t MAX_LINE_COUNT = 256;
const uint16_t MAX_PANE_COUNT = 6;
const uint16_t PANEWIDTH_IN_PIXELS = 300;
const uint16_t PANEHEIGHT_IN_PIXELS = 250;

const uint16_t ID_OUTPUT = 100;
const uint16_t ID_INPUT = 101;
const uint16_t ID_PANE1 = 102;
const uint16_t ID_PANE2 = 103;
const uint16_t ID_PANE3 = 104;
const uint16_t ID_PANE4 = 105;
const uint16_t ID_PANE5 = 106;
const uint16_t ID_PANE6 = 107;
const uint16_t ID_PORTNAME_LABEL = 108;
const uint16_t ID_FILENAME_LABEL = 109;
const uint16_t ID_STATUS_LABEL = 110;
const uint16_t ID_DEST_LABEL = 111;
const uint16_t ID_DEST_EDIT = 112;
const uint16_t ID_COMMAND_LABEL = 113;
const uint16_t ID_COMMAND_EDIT = 114;
const uint16_t ID_WRITE_BUTTON = 115;
const uint16_t ID_CONNECT_BUTTON = 116;


const uint16_t IDM_ADDPANE = 3;		//tells the mainwindow that the "add pane" button has been pressed, which will open a dialogbox
const uint16_t IDM_REMOVEPANE = 4;	//tells the mainwindow that the "remove pane" button has been pressed, which will open a dialogbox
const uint16_t IDM_TEST = 5;
const uint16_t IDM_ERROR = 6;

// Global Variables:
HWND	mainWnd;
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
wchar_t *linesArray[MAX_LINE_COUNT];
ComPortManager* ComPortPtrArr[MAX_PANE_COUNT] = { 0 };

unsigned startIndex;
int numLines;
int numPanes;

// Forward declarations of functions included in this code module:
ATOM                registerMainClass(HINSTANCE);
ATOM                registerOutputClass(HINSTANCE);
ATOM                registerInputClass(HINSTANCE);
ATOM				registerAddPaneClass(HINSTANCE);
ATOM				registerRemovePaneClass(HINSTANCE);
ATOM				registerPaneClass(HINSTANCE);
LRESULT CALLBACK    MainProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    OutputProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK    InputProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK	paneProc(HWND, UINT, WPARAM, LPARAM);
BOOL                InitInstance(HINSTANCE, int);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	addPaneProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	removePaneProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK		ResizeChildren(HWND, LPARAM);
void				createPane(int, wchar_t*, wchar_t*);
void				destroyPane(int);
void				addMenus(HWND);
void				addWindows(HWND);
void				populatePane(HWND, ComPortManager*, int, int);
void				depopulatePane(HWND);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Predefined functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT2, szWindowClass, MAX_LOADSTRING);
    registerMainClass(hInstance);
	registerOutputClass(hInstance);
	registerInputClass(hInstance);
	registerAddPaneClass(hInstance);
	registerRemovePaneClass(hInstance);
	registerPaneClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT2));


	//initialize the linesArray
	startIndex = 0;
	numLines = 0;
	numPanes = 0;

    // Main message loop:
	MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	mainWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

	if (!mainWnd)
	{
		return FALSE;
	}

	ShowWindow(mainWnd, nCmdShow);
	UpdateWindow(mainWnd);

	return TRUE;
}


// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Register functions
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ATOM registerMainClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = MainProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

ATOM registerClass(HINSTANCE hInstance, WNDPROC lpfnWndProc, LPCWSTR lpszClassName) {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = lpfnWndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT2));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = lpszClassName;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

ATOM registerOutputClass(HINSTANCE hInstance) {
	return registerClass(hInstance, OutputProc, OUTPUT_CLASSNAME);
}

ATOM registerInputClass(HINSTANCE hInstance) {
	return registerClass(hInstance, InputProc, INPUT_CLASSNAME);
}

ATOM registerAddPaneClass(HINSTANCE hInstance) {
	return registerClass(hInstance, addPaneProc, ADDPANE_CLASSNAME);
}

ATOM registerRemovePaneClass(HINSTANCE hInstance) {
	return registerClass(hInstance, removePaneProc, REMOVEPANE_CLASSNAME);
}

ATOM registerPaneClass(HINSTANCE hInstance) {
	return registerClass(hInstance, paneProc, PANE_CLASSNAME);
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//Proc Functions
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------------------------------
//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK MainProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		addWindows(hWnd);
		addMenus(hWnd);
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case IDM_TEST:
				break;
			case IDM_ADDPANE:
				if (numPanes >= MAX_PANE_COUNT) {
					PostMessage(hWnd, IDM_ERROR, NULL, NULL);
					break;
				}
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDPANE), hWnd, addPaneProc);
				break;
			case IDM_REMOVEPANE:
				if (numPanes == 0) {
					PostMessage(hWnd, IDM_ERROR, NULL, NULL);
					break;
				}
				DialogBox(hInst, MAKEINTRESOURCE(IDD_REMOVEPANE), hWnd, removePaneProc);
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_THREAD_UP:
	case WM_THREAD_DOWN:
	case WM_THREAD_SENT:
		SendDlgItemMessage(hWnd, ID_INPUT, message, wParam, lParam);
		break;
	case WM_THREAD_RECV:
		SendDlgItemMessage(hWnd, ID_OUTPUT, message, wParam, lParam);
		break;
	case WM_SIZE:
		RECT rcClient;
		GetClientRect(hWnd, &rcClient);
		EnumChildWindows(hWnd, ResizeChildren, (LPARAM)&rcClient);
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK OutputProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	SCROLLINFO si;
	static int yClient;     // height of client area 

	static int yChar;       // vertical scrolling unit 

	static int yPos;        // current vertical scrolling position 

	int i;                  // loop counter 
	int y;					//vertical coordinates

	int FirstLine;          // first line in the invalidated area 
	int LastLine;           // last line in the invalidated area



	switch (message) {
	case WM_CREATE:
		// Get the handle to the client area's device context. 
		hdc = GetDC(hWnd);

		// Extract font dimensions from the text metrics. 
		GetTextMetrics(hdc, &tm);
		yChar = tm.tmHeight + tm.tmExternalLeading;

		// Free the device context. 
		ReleaseDC(hWnd, hdc);

		break;
	case WM_SIZE:
		yClient = HIWORD(lParam);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = numLines - 1;
		si.nPage = yClient / yChar;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		break;
	case WM_VSCROLL:
		// Get all the vertial scroll bar information.
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &si);

		// Save the position for comparison later on.
		yPos = si.nPos;
		switch (LOWORD(wParam))
		{

			// User clicked the HOME keyboard key.
		case SB_TOP:
			si.nPos = si.nMin;
			break;

			// User clicked the END keyboard key.
		case SB_BOTTOM:
			si.nPos = si.nMax;
			break;

			// User clicked the top arrow.
		case SB_LINEUP:
			si.nPos -= 1;
			break;

			// User clicked the bottom arrow.
		case SB_LINEDOWN:
			si.nPos += 1;
			break;

			// User clicked the scroll bar shaft above the scroll box.
		case SB_PAGEUP:
			si.nPos -= si.nPage;
			break;

			// User clicked the scroll bar shaft below the scroll box.
		case SB_PAGEDOWN:
			si.nPos += si.nPage;
			break;

			// User dragged the scroll box.
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}

		// Set the position and then retrieve it.  Due to adjustments
		// by Windows it may not be the same as the value set.
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &si);

		// If the position has changed, scroll window and update it.
		if (si.nPos != yPos)
		{
			ScrollWindow(hWnd, 0, yChar * (yPos - si.nPos), NULL, NULL);
			UpdateWindow(hWnd);
		}
		break;
	case WM_MOUSEWHEEL:
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_VERT, &si);

		//Just check whether the scroll amount was positive or negative and scroll one line in the relevant direction
		//scroll amount (HIWORD(wParam)) is always + or - 120 (WHEEL_DELTA)
		if (((short)HIWORD(wParam)) > 0)
			si.nPos -= 1;
		else
			si.nPos += 1;

		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hWnd, SB_VERT, &si);
		ScrollWindow(hWnd, 0, yChar * (yPos - si.nPos), NULL, NULL);
		UpdateWindow(hWnd);
		break;
	case WM_THREAD_RECV:
		//only scroll if a line was added
		if ((BOOL)wParam) {
			//retrieve scrollinfo object
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			GetScrollInfo(hWnd, SB_VERT, &si);
			//if we're scrolled all the way down, update the position
			//always update the number of lines
			if (si.nPos + (int)(0x7fffffff & si.nPage) > si.nMax)
				si.nPos++;
			si.nMax = numLines - 1;
			si.fMask = SIF_RANGE | SIF_POS;
			SetScrollInfo(hWnd, SB_VERT, &si, TRUE);
		}

		//tell the system that the screen must be repainted
		InvalidateRect(hWnd, NULL, true);
		break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		// Get vertical scroll bar position.
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hWnd, SB_VERT, &si);
		yPos = si.nPos;

		// Find painting limits.
		FirstLine = max(0, yPos + ps.rcPaint.top / yChar);
		LastLine = min(numLines - 1, yPos + ps.rcPaint.bottom / yChar);
		for (i = FirstLine; i <= LastLine; i++)
		{
			y = yChar * (i - yPos);
			// Write a line of text to the client area.
			int index = (i + startIndex) % MAX_LINE_COUNT;
			TextOut(hdc, 0, y, linesArray[index], wcslen(linesArray[index])); //abc is an array of lines
		}

		// Indicate that painting is finished.
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


LRESULT CALLBACK InputProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC hdc;
	PAINTSTRUCT ps;
	TEXTMETRIC tm;
	SCROLLINFO si;
	static int xClient;     // width of client area 

	static int xChar;       // horizontal scrolling unit 

	static int xPos;        // current horizontal scrolling position 
	static int oldClient;		// current size of window in scrolling units

	switch (message) {
	case WM_CREATE:
		// Get the handle to the client area's device context. 
		hdc = GetDC(hWnd);

		// Extract font dimensions from the text metrics. 
		GetTextMetrics(hdc, &tm);
		xChar = tm.tmMaxCharWidth;

		// Free the device context. 
		ReleaseDC(hWnd, hdc);

		break;
	case WM_SIZE:
		oldClient = xClient;
		xClient = LOWORD(lParam);
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hWnd, SB_HORZ, &si);

		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = numPanes*PANEWIDTH_IN_PIXELS - 1;
		si.nPage = xClient;

		//if the page grew, AND there's no more room on the right, AND there's room on the left,
		//then scroll to the left one pixel
		if (xClient > oldClient) {
			if ((si.nMax - si.nPos) < (int)si.nPage) {
				if (si.nPos != 0) {
					xPos = si.nPos;
					si.nPos += oldClient - xClient;
					si.fMask |= SIF_POS;
					ScrollWindow(hWnd, xPos - max(0, si.nPos), 0, NULL, NULL);
					UpdateWindow(hWnd);
				}
			}
		}
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
		break;
	case WM_HSCROLL:
		// Get all the horizontal scroll bar information.
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_HORZ, &si);

		// Save the position for comparison later on.
		xPos = si.nPos;
		switch (LOWORD(wParam))
		{

			// User Scrolls to the upper left.
		case SB_LEFT:
			si.nPos = si.nMin;
			break;

			// User Scrolls to the lower right.
		case SB_RIGHT:
			si.nPos = si.nMax;
			break;

			// User Scrolls left by one unit.
		case SB_LINELEFT:
			si.nPos -= 1;
			break;

			// User Scrolls right by one unit.
		case SB_LINERIGHT:
			si.nPos += 1;
			break;

			// User Scrolls left by the width of the window.
		case SB_PAGELEFT:
			si.nPos -= PANEWIDTH_IN_PIXELS;
			break;

			// User Scrolls right by the width of the window.
		case SB_PAGERIGHT:
			si.nPos += PANEWIDTH_IN_PIXELS;
			break;

			// User dragged the scroll box.
		case SB_THUMBTRACK:
			si.nPos = si.nTrackPos;
			break;

		default:
			break;
		}

		// Set the position and then retrieve it.  Due to adjustments
		// by Windows it may not be the same as the value set.
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hWnd, SB_HORZ, &si);

		// If the position has changed, scroll window and update it.
		if (si.nPos != xPos)
		{
			ScrollWindow(hWnd, xPos - si.nPos, 0, NULL, NULL);
			UpdateWindow(hWnd);
		}
		break;
	case WM_MOUSEWHEEL:
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hWnd, SB_HORZ, &si);
		xPos = si.nPos;

		//Just check whether the scroll amount was positive or negative and scroll one line in the relevant direction
		//scroll amount (HIWORD(wParam)) is always + or - 120 (WHEEL_DELTA)
		if (((short)HIWORD(wParam)) > 0)
			si.nPos -= PANEWIDTH_IN_PIXELS;
		else
			si.nPos += PANEWIDTH_IN_PIXELS;
		si.fMask = SIF_POS;
		SetScrollInfo(hWnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hWnd, SB_HORZ, &si);
		if (si.nPos != xPos){
			ScrollWindow(hWnd, xPos - max(0, si.nPos), 0, NULL, NULL);
			UpdateWindow(hWnd);
		}
		break;
	case WM_PAINT:
	{
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CREATEPANE:
	{
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hWnd, SB_HORZ, &si);
		createPane(si.nPos, (wchar_t*)wParam, (wchar_t*)lParam);
	}
		break;
	case WM_DESTROYPANE:
		//A pane should be destroyed.
		//index of pane is located in wParam.
		//the thread is guaranteed to be not running
		//comes from paneProc's WM_DESTROYPANE or WM_THREADDOWN case statements.
	{
		int paneIndex = (int)wParam;
		delete(ComPortPtrArr[paneIndex]);
		DestroyWindow(GetDlgItem(hWnd, ID_PANE1 + paneIndex));
		for (int k = paneIndex; k < numPanes; k++)
			ComPortPtrArr[k] = ComPortPtrArr[k + 1];

		/*
			2. free the comportmanager //can occur anywhere, globally
			3. destroy the window	//occurs in input_proc
			4. re-arrange the internal storage so that the comportmanager array is lined up correctly. //can occur anywhere, globally
			5. re-arrange the input window so that everything is lined up correctly. 	//occurs in input_proc
		*/
	}
		break;
	case WM_THREAD_UP:
	case WM_THREAD_DOWN:
	case WM_THREAD_SENT:
		SendDlgItemMessage(hWnd, ID_PANE1 + (int)wParam, message, 0, 0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK paneProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	//Indicates whether the pane should be closed and is waiting on some outstanding cleanup operations
	static bool doClose;
	//Indicates whether the thread is currently connected, disconnected, or transitioning.
	static short connectionState;
	const short CONNECTED = 0;
	const short DISCONNECTED = 1;
	const short WAITING = 2;

	switch (message) {
	case WM_CREATE:
	{
		doClose = false;
		connectionState = DISCONNECTED;
		TEXTMETRIC tm;
		// Get the handle to the client area's device context. 
		HDC hdc = GetDC(hWnd);

		// Extract font dimensions from the text metrics. 
		GetTextMetrics(hdc, &tm);
		int avgWidth = tm.tmAveCharWidth;
		int height = tm.tmHeight;

		// Free the device context. 
		ReleaseDC(hWnd, hdc);

		populatePane(hWnd, ComPortPtrArr[GetDlgCtrlID(hWnd) - ID_PANE1], avgWidth, height);
	}
		break;
	case WM_THREAD_UP:
		connectionState = CONNECTED;
		SetDlgItemTextA(hWnd, ID_STATUS_LABEL, "Connected");
		break;
	case WM_THREAD_DOWN:
		connectionState = DISCONNECTED;
		SetDlgItemTextA(hWnd, ID_STATUS_LABEL, "Disconnected");
		if(doClose)
			SendMessage(GetParent(hWnd), WM_DESTROYPANE, GetDlgCtrlID(hWnd) - ID_PANE1, 0);
		break;
	case WM_THREAD_SENT:
		break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_WRITE_BUTTON:
			OutputDebugStringA("write");
			break;
		case ID_CONNECT_BUTTON:
			OutputDebugStringA("connect");
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		EndPaint(hWnd, &ps);
	}
		break;
	case WM_DESTROYPANE:
		//This pane should be destroyed.
		//Comes from removeProc's ID_OK case statement.
		if (connectionState == CONNECTED || connectionState == WAITING) {
			doClose = true;
			ComPortPtrArr[GetDlgCtrlID(hWnd) - ID_PANE1]->endThread();
			break;
		}
		SendMessage(GetParent(hWnd), WM_DESTROYPANE, GetDlgCtrlID(hWnd) - ID_PANE1, 0);
		break;
	case WM_DESTROY:
		depopulatePane(hWnd);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK addPaneProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			wchar_t portNameBuffer[256];
			if (!GetDlgItemText(hWnd, IDC_ADDPANE_PORTNAME, portNameBuffer, 255)) {
				PostMessage(GetParent(hWnd), IDM_ERROR, NULL, NULL);
				break;
			}
			wchar_t fileNameBuffer[256];
			if (!GetDlgItemText(hWnd, IDC_ADDPANE_FILENAME, fileNameBuffer, 255)) {
				PostMessage(GetParent(hWnd), IDM_ERROR, NULL, NULL);
				break;
			}
			SendDlgItemMessage(GetParent(hWnd), ID_INPUT, WM_CREATEPANE, (WPARAM)portNameBuffer, (LPARAM)fileNameBuffer);
		case IDCANCEL:
			EndDialog(hWnd, wParam);
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK removePaneProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static char buffer[256];
	static LRESULT paneIndex;
	switch (message) {
	case WM_INITDIALOG:
		for (int k = 0; k < numPanes; k++) {
			OutputDebugStringA(buffer);
			ComPortPtrArr[k]->getPortName(buffer);
			SendDlgItemMessageA(hWnd, IDC_REMOVEPANE_PORTNAME, CB_ADDSTRING, 0, (LPARAM) buffer);
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			paneIndex = SendDlgItemMessage(hWnd, IDC_REMOVEPANE_PORTNAME, CB_GETCURSEL, 0, 0);
			if (paneIndex != CB_ERR)
				SendDlgItemMessage(GetDlgItem(mainWnd, ID_INPUT), ID_PANE1+paneIndex, WM_DESTROYPANE, 0, 0);
		case IDCANCEL:
			EndDialog(hWnd, wParam);
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Other functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void addMenus(HWND hWnd) {
	HMENU hMenu = GetMenu(hWnd);

	AppendMenu(hMenu, MF_STRING, IDM_TEST, L"Test Button");
	AppendMenu(hMenu, MF_STRING, IDM_ADDPANE, L"Add New Pane");
	AppendMenu(hMenu, MF_STRING, IDM_REMOVEPANE, L"Remove Pane");

	SetMenu(hWnd, hMenu);
}

void addWindows(HWND hWnd) {
	CreateWindow(OUTPUT_CLASSNAME,
		(LPCTSTR)NULL,
		WS_CHILD | WS_BORDER | WS_VSCROLL,
		0, 0, 0, 0,
		hWnd,
		(HMENU)(int)(ID_OUTPUT),
		hInst,
		NULL);

	CreateWindow(INPUT_CLASSNAME,
		(LPCTSTR)NULL,
		WS_CHILD | WS_BORDER | WS_HSCROLL,
		0, 0, 0, 0,
		hWnd,
		(HMENU)(int)(ID_INPUT),
		hInst,
		NULL);
}

BOOL CALLBACK ResizeChildren(HWND hWndChild, LPARAM lparam) {
	LPRECT rcParent;
	int idChild;
	idChild = GetWindowLong(hWndChild, GWL_ID);
	rcParent = (LPRECT)lparam;
	int x, y, width, height, temp;
	temp = rcParent->bottom - PANEHEIGHT_IN_PIXELS;
	switch (idChild) {
	case ID_INPUT:
		x = 0;
		y = temp;
		width = rcParent->right;
		height = PANEHEIGHT_IN_PIXELS;
		break;
	case ID_OUTPUT:
		x = y = 0;
		width = rcParent->right;
		height = temp;
		break;
	default:
		return TRUE;
	}
	MoveWindow(hWndChild,
		x, y, width, height,
		TRUE);
	ShowWindow(hWndChild, SW_SHOW);
	return TRUE;
}

void createPane(int nPos, wchar_t* portBuff, wchar_t* fileBuff) {
	ComPortPtrArr[numPanes] = new ComPortManager(portBuff, fileBuff, mainWnd, numPanes);

	CreateWindow(PANE_CLASSNAME,
		(LPCTSTR)NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		PANEWIDTH_IN_PIXELS*numPanes-nPos, 0, PANEWIDTH_IN_PIXELS, PANEHEIGHT_IN_PIXELS,
		GetDlgItem(mainWnd, ID_INPUT),
		(HMENU)(uint16_t)(ID_PANE1+numPanes),
		hInst,
		NULL);
	numPanes++;
}

void populatePane(HWND paneWnd, ComPortManager* ComPortManagerPtr, int avgWidth, int height) {
	char temp[256];
	int hMargin = 2 * avgWidth;
	int currHeight = 2; //vertical margin
	CreateWindow(L"Static", L"", WS_VISIBLE | WS_CHILD, hMargin, (currHeight++) * height, 20 * avgWidth, height, paneWnd, (HMENU)ID_PORTNAME_LABEL, NULL, NULL);
	strcpy(temp, "PortName: ");
	ComPortManagerPtr->getPortName(temp + 10);
	SetDlgItemTextA(paneWnd, ID_PORTNAME_LABEL, temp);
	CreateWindow(L"Static", L"", WS_VISIBLE | WS_CHILD, hMargin, (currHeight++) * height, 20 * avgWidth, height, paneWnd, (HMENU)ID_FILENAME_LABEL, NULL, NULL);
	strcpy(temp, "FilePath: ");
	ComPortManagerPtr->getPortName(temp + 10);
	SetDlgItemTextA(paneWnd, ID_FILENAME_LABEL, temp);
	CreateWindow(L"Static", L"", WS_VISIBLE | WS_CHILD, hMargin, (currHeight++) * height, 20 * avgWidth, height, paneWnd, (HMENU)ID_STATUS_LABEL, NULL, NULL);
	strcpy(temp, "Status: Disconnected");
	SetDlgItemTextA(paneWnd, ID_STATUS_LABEL, temp);
	CreateWindow(L"Button", L"Connect", WS_VISIBLE|WS_CHILD, PANEWIDTH_IN_PIXELS - 2 * hMargin - 10 * avgWidth, (currHeight++) * height, 10 * avgWidth, height, paneWnd, (HMENU)ID_CONNECT_BUTTON, NULL, NULL);
	currHeight++;
	CreateWindow(L"Static", L"Destination:", WS_VISIBLE | WS_CHILD, hMargin, (currHeight++) * height, 20 * avgWidth, height, paneWnd, (HMENU)ID_DEST_LABEL, NULL, NULL);
	CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, hMargin, (currHeight++) * height, PANEWIDTH_IN_PIXELS - 4 * avgWidth, height, paneWnd, (HMENU)ID_DEST_EDIT, NULL, NULL);
	currHeight++;
	CreateWindow(L"Static", L"Command:", WS_VISIBLE | WS_CHILD, hMargin, (currHeight++) * height, 20 * avgWidth, height, paneWnd, (HMENU)ID_COMMAND_LABEL, NULL, NULL);
	CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER, hMargin, (currHeight++) * height, PANEWIDTH_IN_PIXELS - 4 * avgWidth, height, paneWnd, (HMENU)ID_COMMAND_EDIT, NULL, NULL);
	currHeight++;
	CreateWindow(L"Button", L"Write", WS_VISIBLE | WS_CHILD, PANEWIDTH_IN_PIXELS - 2 * hMargin - 10*avgWidth, (currHeight++) * height, 10 * avgWidth, height, paneWnd, (HMENU)ID_WRITE_BUTTON, NULL, NULL);
}

void depopulatePane(HWND paneWnd) {
	DestroyWindow(GetDlgItem(paneWnd, ID_PORTNAME_LABEL));
	DestroyWindow(GetDlgItem(paneWnd, ID_FILENAME_LABEL));
	DestroyWindow(GetDlgItem(paneWnd, ID_STATUS_LABEL));
	DestroyWindow(GetDlgItem(paneWnd, ID_DEST_LABEL));
	DestroyWindow(GetDlgItem(paneWnd, ID_DEST_EDIT));
	DestroyWindow(GetDlgItem(paneWnd, ID_COMMAND_LABEL));
	DestroyWindow(GetDlgItem(paneWnd, ID_COMMAND_EDIT));
	DestroyWindow(GetDlgItem(paneWnd, ID_WRITE_BUTTON));
}

void destroyPane(int paneIndex) {
	//TODO: Kill the thread, wait on the kill
	//free the ComPortManager
	//destroy the window
}