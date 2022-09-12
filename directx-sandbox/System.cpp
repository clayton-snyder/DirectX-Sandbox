#include <stdio.h>

#include "System.h"
#include "Input.h"
#include "Graphics.h"

System::System() {
	this->pInput = NULL;
	this->pGraphics = NULL;
}

System::System(const System& other) {

}

// Object cleanup is done in Shutdown to mitigate scenarios when destructor is not called 
System::~System() {

}

// Create and initialize windows, input, and graphics objects
bool System::Init() {
	int screenWidth = 0, screenHeight = 0;

	InitializeWindows(screenWidth, screenHeight);

	this->pInput = new Input();
	if (!(this->pInput))
	{
		printf("ERROR! Failed to create Input.\n");
		return false;
	}

	pInput->Init();


	this->pGraphics = new Graphics();
	if (!this->pGraphics)
	{
		printf("ERROR! Failed to create Graphics.\n");
		return false;
	}
	
	bool result = this->pGraphics->Init(screenWidth, screenHeight, this->hWnd);
	return result;
}

void System::Shutdown() {
	// Release the graphics object.
	if (this->pGraphics)
	{
		this->pGraphics->Shutdown();
		delete this->pGraphics;
		this->pGraphics = NULL;
	}

	// Release the input object.
	if (this->pInput)
	{
		delete this->pInput;
		this->pInput = NULL;
	}

	ShutdownWindows();
}

void System::Run() {
	MSG msg;
	bool quit = false;

	ZeroMemory(&msg, sizeof(MSG));

	// Loop until there is a quit message from the window or the user.
	while (!quit) {
		// Check for system messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		// Windows-signalled quit
		if (msg.message == WM_QUIT)
		{
			quit = true;
			continue;
		}
		quit = Frame();
	}
}

bool System::Frame() {
	if (this->pInput->IsKeyDown(VK_ESCAPE)) {
		return true;
	}

	// Pass off to Graphics processing
	return this->pGraphics->Frame();
}

LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam) {
	switch (umsg)
	{
	case WM_KEYDOWN: {
		// Send key presses to input object to record the state
		this->pInput->KeyDown((unsigned int)wparam);
		return 0;
	}

	case WM_KEYUP: {
		// Send to input object to clear the state for that key
		this->pInput->KeyUp((unsigned int)wparam);
		return 0;
	}

	// Send messages we don't use to the default message handler
	default:
		return DefWindowProc(hwnd, umsg, wparam, lparam);
	}
}

void System::InitializeWindows(int& screenWidth, int& screenHeight) {
	WNDCLASSEX wc;
	DEVMODE dmScreenSettings;
	int posX, posY;

	ApplicationHandle = this;

	this->hInstance = GetModuleHandle(NULL);
	this->ApplicationName = L"DXSandbox";

	// Set up the windows class with default settings
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;  // Bitwise OR to combine settings
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = this->hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hIconSm = wc.hIcon;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = this->ApplicationName;
	wc.cbSize = sizeof(WNDCLASSEX);

	RegisterClassEx(&wc);

	// NOTE: This is writing to the variables passed to this function. They were init to 0
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// Setup the screen settings depending on whether it is running in full screen or in windowed mode.
	if (FULL_SCREEN) {
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = (unsigned long)screenWidth;
		dmScreenSettings.dmPelsHeight = (unsigned long)screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

		posX = 0;
		posY = 0;
	}
	else {
		// Default windowed mode to 800x600
		screenWidth = 800;
		screenHeight = 600;

		// Place in middle of the screen
		posX = (GetSystemMetrics(SM_CXSCREEN) - screenWidth) / 2;
		posY = (GetSystemMetrics(SM_CYSCREEN) - screenHeight) / 2;
	}

	// Create the window with the screen settings and get the handle to it.
	this->hWnd = CreateWindowEx(WS_EX_APPWINDOW, this->ApplicationName, this->ApplicationName,
		WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP,
		posX, posY, screenWidth, screenHeight, NULL, NULL, this->hInstance, NULL);

	ShowWindow(this->hWnd, SW_SHOW);
	SetForegroundWindow(this->hWnd);
	SetFocus(this->hWnd);

	//ShowCursor(false);
}

void System::ShutdownWindows() {
	ShowCursor(true);

	// Fix the display settings if leaving full-screen mode
	if (FULL_SCREEN)
	{
		ChangeDisplaySettings(NULL, 0);
	}

	DestroyWindow(this->hWnd);
	this->hWnd = NULL;

	UnregisterClass(this->ApplicationName, this->hInstance);
	this->hInstance = NULL;

	ApplicationHandle = NULL;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam) {
	switch (umessage) {
	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}

	case WM_CLOSE: {
		PostQuitMessage(0);
		return 0;
	}

	// All other messages pass to our own MessageHandler in System
	default:
		return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
	}
}