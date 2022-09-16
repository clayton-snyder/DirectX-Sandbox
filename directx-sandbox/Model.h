#pragma once

#include <string>
#include <fstream>
#include <d3d11.h>
#include <DirectXMath.h>
#include <system_error>

#include "Texture.h"

static const int TOKENS_PER_ROW = 8;

/* This class is responsible for encapsulating the 3D geometry for models. */
class Model {
private:
	struct Vertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texture;
		DirectX::XMFLOAT3 normal;
	};

	struct ModelFileRow {
		float posX, posY, posZ;
		float texU, texV;
		float normX, normY, normZ;
	};

	ID3D11Buffer* pVertexBuffer;
	ID3D11Buffer* pIndexBuffer;
	int vertexCount, indexCount;
	Texture* pTexture;
	ModelFileRow* fileRows;

	// These functions handle init and shutdown of the model's vertex and index buffers.
	bool InitBuffers(ID3D11Device*);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);
	bool LoadTexture(ID3D11Device*, ID3D11DeviceContext*, const char*);
	void ReleaseTexture();
	bool LoadModel(std::string);
	void ReleaseModel();

public:
	Model();

	// These functions handle init and shutdown of the model's vertex and index buffers.
	bool Init(ID3D11Device*, ID3D11DeviceContext*, const char*, std::string);
	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetTexture();
};