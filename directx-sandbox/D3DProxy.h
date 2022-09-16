#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <Windows.h>
#include <d3d11.h>
#include <DirectXMath.h>

class D3DProxy
{
public:
	D3DProxy();

	bool Init(int, int, bool, HWND, bool, float, float);
	void Shutdown();

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device* GetDevice();
	ID3D11DeviceContext* GetDeviceContext();

	void GetProjectionMatrix(DirectX::XMMATRIX&);
	void GetWorldMatrix(DirectX::XMMATRIX&);
	void GetOrthoMatrix(DirectX::XMMATRIX&);

	void GetVideoCardInfo(char*, int&);

private:
	bool vsyncEnabled;
	int videoCardMem;
	char videoCardDescription[128];
	IDXGISwapChain* pSwapChain;
	ID3D11Device* pDevice;
	ID3D11DeviceContext* pDeviceContext;
	ID3D11RenderTargetView* pRenderTargetView;
	ID3D11Texture2D* pDepthStencilBuffer;
	ID3D11DepthStencilState* pDepthStencilState;
	ID3D11DepthStencilView* pDepthStencilView;
	ID3D11RasterizerState* pRasterState;
	DirectX::XMMATRIX pProjectionMatrix;
	DirectX::XMMATRIX pWorldMatrix;
	DirectX::XMMATRIX pOrthoMatrix;
};