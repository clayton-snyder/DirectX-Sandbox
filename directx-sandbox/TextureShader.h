#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <fstream>

/* This class invokes the HLSL shaders for drawing 3D models on the GPU */
// Mostly the same as ColorShader with changes to render Textures instead of plain colors.
class TextureShader {
private:
	// This type must match exactly the cbuffer type in the vertex shader
	struct MatrixBuffer {
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	};

	bool InitShader(ID3D11Device*, HWND, const wchar_t*, const wchar_t*);
	void ShutdownShader();
	void OutputShaderErrorMessage(ID3D10Blob*, HWND, const wchar_t*);
	bool SetShaderParams(ID3D11DeviceContext*, ID3D11ShaderResourceView*, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX);
	void RenderShader(ID3D11DeviceContext*, int);

	ID3D11VertexShader* pVertexShader;
	ID3D11PixelShader* pPixelShader;
	ID3D11InputLayout* pLayout;
	ID3D11Buffer* pMatrixBuffer;
	ID3D11SamplerState* pSamplerState; // This will get passed to the texture shader.

public:
	TextureShader();

	bool Init(ID3D11Device*, HWND);
	void Shutdown();
	bool Render(ID3D11DeviceContext*, int, ID3D11ShaderResourceView*, DirectX::XMMATRIX, DirectX::XMMATRIX, DirectX::XMMATRIX);
};