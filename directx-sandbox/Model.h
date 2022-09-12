#pragma once

#include <d3d11.h>
#include <DirectXMath.h>

#include "Texture.h"

/* This class is responsible for encapsulating the 3D geometry for models. */
class Model {
private:
	struct Vertex {
		DirectX::XMFLOAT3 position; // x,y,z
		//DirectX::XMFLOAT4 color;    // r,g,b,a
		DirectX::XMFLOAT2 texture;
	};

	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;
	int vertexCount, indexCount;
	Texture* pTexture;

	// These functions handle init and shutdown of the model's vertex and index buffers.
	bool InitBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void ReleaseTexture();

public:
	Model();
	Model(const Model&);
	~Model();

	// These functions handle init and shutdown of the model's vertex and index buffers.
	bool Init(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();
};