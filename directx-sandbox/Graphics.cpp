#include "Graphics.h"

Graphics::Graphics() {
	this->pDirect3D = nullptr;
	this->pCamera = nullptr;
	this->pModel = nullptr;
	//this->pColorShader = nullptr;
	//this->pTextureShader = nullptr;
	this->pLightShader = nullptr;
	this->pLight = nullptr;
}


bool Graphics::Init(int screenW, int screenH, HWND hWnd) {
	// Create the Direct3D object.
	this->pDirect3D = new D3DProxy();

	// Initialize the Direct3D object.
	bool result = this->pDirect3D->Init(screenW, screenH, VSYNC_ENABLED, hWnd, 
		FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hWnd, L"Could not initialize Direct3D", L"D3D Init Error", MB_OK);
		return false;
	}

	this->pCamera = new Camera();
	this->pCamera->SetPosition(0.0f, 0.0f, -25.0f);
	this->pCamera->SetRotation(0.0f, 0.0f, 0.0f);

	this->pModel = new Model();
	result = this->pModel->Init(pDirect3D->GetDevice(), pDirect3D->GetDeviceContext(),
		//"./data/stone01.tga", "./data/sq_cubes.txt");
		"./data/aluminum.tga", "./data/sq_cubes.txt");
		//"./data/danger.tga", "./data/sphere.txt");
	if (!result) {
		MessageBox(hWnd, L"Could not initialize model object.", L"Model Init Error", MB_OK);
		return false;
	}

	this->pLightShader = new LightShader();
	result = pLightShader->Init(pDirect3D->GetDevice(), hWnd);
	if (!result) {
		MessageBox(hWnd, L"Could not initialize TextureShader object.", 
			L"TextureShader Init Error", MB_OK);
		return false;
	}

	this->pLight = new Light();
	pLight->SetDiffuseColor(0.7f, 0.7f, 0.7f, 1.0f);
	pLight->SetAmbientColor(0.15f, 0.15f, 0.15f, 1.0f);
	pLight->SetSpecularColor(0.8f, 0.8f, 0.8f, 1.0f);
	pLight->SetSpecularExp(50.0f);
	//pLight->SetDirection(1.0f, -1.0f, 1.0f);
	pLight->SetDirection(0.0f, 0.0f, 1.0f);

	return true;
}

void Graphics::Shutdown() {
	if (this->pLightShader) {
		pLightShader->Shutdown();
		delete pLightShader;
		pLightShader = nullptr;
	}

	if (this->pLight) {
		delete pLight;
		pLight = nullptr;
	}

	if (this->pModel) {
		pModel->Shutdown();
		delete pModel;
		pModel = nullptr;
	}

	if (this->pCamera) {
		delete pCamera;
		pCamera = nullptr;
	}

	if (this->pDirect3D) {
		pDirect3D->Shutdown();
		delete pDirect3D;
		pDirect3D = nullptr;
	}
}

// Returning 'true' will exit the program
bool Graphics::Frame() {
	static float rotation = 0.0f; // is this the right place to declare this?

	rotation += DirectX::XM_PI * 0.002f;
	if (rotation > 360.0f) rotation = 0;

	return Render(rotation);
}

bool Graphics::Render(float rotation) {
	//pDirect3D->BeginScene(1.0f, 1.0f, 0.85f, 1.0f); // background
	pDirect3D->BeginScene(0.07f, 0.0f, 0.34f, 1.0f);

	// Generate the view matrix based on camera's current position
	pCamera->Render();

	DirectX::XMMATRIX worldMatrix, viewMatrix, projectionMatrix; // populated by passing as refs
	pDirect3D->GetWorldMatrix(worldMatrix);
	pCamera->GetViewMatrix(viewMatrix);
	pDirect3D->GetProjectionMatrix(projectionMatrix);

	// Put the vertex/index buffers in the graphics pipeline to prepare for rendering
	pModel->Render(this->pDirect3D->GetDeviceContext()); 

	bool result = this->pLightShader->Render(
		pDirect3D->GetDeviceContext(), pModel->GetIndexCount(), pModel->GetTexture(),
		worldMatrix * DirectX::XMMatrixRotationY(rotation)
		* DirectX::XMMatrixRotationX(rotation), viewMatrix, projectionMatrix,
		pLight->GetDirection(), pLight->GetDiffuseColor(), pLight->GetAmbientColor(),
		pLight->GetSpecularColor(), pLight->GetSpecularExp(), pCamera->GetPosition()
	);
	if (!result) return false;

	pDirect3D->EndScene();

	return false;
}