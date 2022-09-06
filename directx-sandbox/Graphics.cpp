#include "Graphics.h"

Graphics::Graphics() {
	this->pDirect3D = nullptr;
}

Graphics::Graphics(const Graphics& other) {
}

Graphics::~Graphics() {
}

bool Graphics::Init(int screenW, int screenH, HWND hWnd) {
	// Create the Direct3D object.
	this->pDirect3D = new D3DProxy();
	if (!(this->pDirect3D))
	{
		return false;
	}

	// Initialize the Direct3D object.
	bool result = this->pDirect3D->Init(screenW, screenH, VSYNC_ENABLED, hWnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hWnd, L"Could not initialize Direct3D", L"Error", MB_OK);
	}
	return result;
}

void Graphics::Shutdown() {
	if (this->pDirect3D)
	{
		this->pDirect3D->Shutdown();
		delete this->pDirect3D;
		this->pDirect3D = nullptr;
	}
}

// Returning 'true' will exit the program
bool Graphics::Frame() {
	return Render();
}

bool Graphics::Render() {
	this->pDirect3D->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);
	this->pDirect3D->EndScene();
	return false;
}