#pragma once

#include "resource.h"

#define WM_THREAD_UP (WM_USER + 1)
#define WM_THREAD_DOWN (WM_USER + 2)
#define WM_THREAD_RECV (WM_USER + 3)
#define WM_THREAD_SENT (WM_USER + 4)
#define WM_DESTROYPANE (WM_USER + 5)//tells the inputwindow to destroy a pane.
#define WM_CREATEPANE (WM_USER + 6)	//tells the inputwindow to create a new pane.
#define WM_TEST (WM_USER + 7)
#define WM_KILLPANE (WM_USER+8) //tells an individual pane that it must prepare for destruction.
								//also represents the reply back from the pane that it is ready