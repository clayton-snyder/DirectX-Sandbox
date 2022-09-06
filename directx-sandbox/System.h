#pragma once
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>

#include "Input.h"
#include "Graphics.h"

class System {
public:
	System();
	System(const System&);
	~System();

	bool Init();
	void Shutdown();
	void Run();

	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private:
	bool Frame();
	void InitializeWindows(int&, int&);
	void ShutdownWindows();

private:
	LPCWSTR ApplicationName;
	HINSTANCE hInstance;
	HWND hWnd;

	Input* pInput;
	Graphics* pGraphics;
};

// Included so we can re-direct Windows messages to our MessageHandler
static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static System* ApplicationHandle = 0;