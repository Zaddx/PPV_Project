// Engine Development Win32.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Engine Development Win32.h"

#include <iostream>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

// Declariation of HWND
HWND hWnd;

// Instance Initializing and Interfacing class
Init_and_Inter _initializer;
User_Input _input;
Timer _timer;

int messageloop()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	// Main message loop:
	while (true)
	{
		// Process all messages stop on WM_QUIT
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// WM_QUIT does no need to be
			// translated or dispatched
			if (msg.message == WM_QUIT)
				break;

			// Translates messages and sends them
			// to WndProc
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			_timer.frameCount++;
			if (_timer.GetTime() > 1.0f)
			{
				_timer.fps = _timer.frameCount;
				_timer.frameCount = 0;
				_timer.StartTimer();
			}
			_timer.frameTime = _timer.GetFrameTime();

			// Check for user input
			_input.DetectInput(_timer.frameTime, _initializer);

			// Run graphics code
			_initializer.UpdateScene(_timer.frameTime, _input);
			_initializer.DrawScene();
		}
	}
	return msg.wParam;
}

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
    LoadStringW(hInstance, IDC_ENGINEDEVELOPMENTWIN32, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

#ifndef NDEBUG

	AllocConsole();
	FILE* new_std_in_out;
	freopen_s(&new_std_in_out, "CONOUT$", "w", stdout);
	freopen_s(&new_std_in_out, "CONIN$", "r", stdin);
	std::cout << "Hello world!\n";

#endif // !NDEBUG

	if (!_initializer.InitializeDirect3d11App(hInstance, hWnd))	// Initialize Direct3D
	{
		MessageBox(0, L"Direct3D Initialization - Failed", L"Error", MB_OK);
		return 0;
	}

	if (!_input.InitDirectInput(hInstance, hWnd))
	{
		MessageBox(0, L"Direct Input Initialization - Failed",L"Error", MB_OK);
		return false;
	}

	if (!_initializer.InitScene(_input))    //Initialize our scene
	{
		MessageBox(0, L"Scene Initialization - Failed",L"Error", MB_OK);
		return 0;
	}

    //HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ENGINEDEVELOPMENTWIN32));

	messageloop();
	_initializer.ReleaseObjects();

#ifndef NDEBUG

	FreeConsole();

#endif // !NDEBUG

	return 0;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ENGINEDEVELOPMENTWIN32));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX,
	   CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   RECT rect;
   GetClientRect(hWnd, &rect);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

