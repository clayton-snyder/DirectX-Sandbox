#include "Graphics.h"

Graphics::Graphics() {
	this->pDirect3D = nullptr;
	this->pCamera = nullptr;
	this->pModel = nullptr;
	//this->pColorShader = nullptr;
	this->pTextureShader = nullptr;
}

Graphics::Graphics(const Graphics& other) {
}

Graphics::~Graphics() {
}

bool Graphics::Init(int screenW, int screenH, HWND hWnd) {
	// Create the Direct3D object.
	this->pDirect3D = new D3DProxy();

	// Initialize the Direct3D object.
	bool result = this->pDirect3D->Init(screenW, screenH, VSYNC_ENABLED, hWnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hWnd, L"Could not initialize Direct3D", L"D3D Init Error", MB_OK);
		return false;
	}

	this->pCamera = new Camera();
	this->pCamera->SetPosition(0.0f, 0.0f, -15.0f);
	this->pCamera->SetRotation(0.0f, 0.0f, 0.0f);

	this->pModel = new Model();
	result = this->pModel->Init(this->pDirect3D->GetDevice(), this->pDirect3D->GetDeviceContext(), "./data/stone01.tga");
	if (!result) {
		MessageBox(hWnd, L"Could not initialize the model object.", L"Model Init Error", MB_OK);
		return false;
	}

	//this->pColorShader = new ColorShader();
	//result = this->pColorShader->Init(this->pDirect3D->GetDevice(), hWnd);
	this->pTextureShader = new TextureShader();
	result = this->pTextureShader->Init(this->pDirect3D->GetDevice(), hWnd);
	if (!result) {
		MessageBox(hWnd, L"Could not initialize the TextureShader object.", L"TextureShader Init Error", MB_OK);
		return false;
	}

	return true;
}

void Graphics::Shutdown() {
	if (this->pTextureShader) {
		this->pTextureShader->Shutdown();
		delete this->pTextureShader;
		this->pTextureShader = nullptr;
	}

	if (this->pModel) {
		this->pModel->Shutdown();
		delete this->pModel;
		this->pModel = nullptr;
	}

	if (this->pCamera) {
		delete this->pCamera;
		this->pCamera = nullptr;
	}

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
	this->pDirect3D->BeginScene(0.0f, 0.0f, 0.0f, 1.0f); // plain black screen

	// Generate the view matrix based on camera's current position
	this->pCamera->Render();

	DirectX::XMMATRIX worldMatrix, viewMatrix, projectionMatrix; // populated by passing as refs
	this->pDirect3D->GetWorldMatrix(worldMatrix);
	this->pCamera->GetViewMatrix(viewMatrix);
	this->pDirect3D->GetProjectionMatrix(projectionMatrix);

	// Put the vertex/index buffers in the graphics pipeline to prepare for rendering
	this->pModel->Render(this->pDirect3D->GetDeviceContext()); 

	bool result = this->pTextureShader->Render(
		this->pDirect3D->GetDeviceContext(),
		this->pModel->GetIndexCount(),
		this->pModel->GetTexture(),
		worldMatrix, viewMatrix, projectionMatrix
	);
	if (!result) return false;

	this->pDirect3D->EndScene();

	return false;
}