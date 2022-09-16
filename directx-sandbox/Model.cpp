#include "Model.h"
#include <stdio.h>

Model::Model() {
	this->pVertexBuffer = nullptr;
	this->pIndexBuffer = nullptr;
	this->pTexture = nullptr;
	this->vertexCount = 0;
	this->indexCount = 0;
}

bool Model::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const char* textureFilename) {
	bool result = InitBuffers(pDevice);
	if (!result) {
		printf("ERROR: InitBuffers returned false.\n");
		return false;
	}
	printf("Model buffers initialized...\n");
	// Load this model's texture
	result = LoadTexture(pDevice, pDeviceContext, textureFilename);
	if (!result) printf("ERROR: LoadTexture returned false.\n");
	
	return result;
}

void Model::Shutdown() {
	ReleaseTexture();
	ShutdownBuffers();
}

void Model::Render(ID3D11DeviceContext* pDeviceContext) {
	RenderBuffers(pDeviceContext);
}

int Model::GetIndexCount() {
	return this->indexCount;
}

ID3D11ShaderResourceView* Model::GetTexture() {
	return this->pTexture->GetTexture();
}

// This is where the vertex and index buffers are created. Typically you would read in a model 
// data file and create the buffers from that, but for now we are just making one triangle.
bool Model::InitBuffers(ID3D11Device* device) {
	this->vertexCount = 4;
	this->indexCount = 12;
	Vertex* vertices = new Vertex[this->vertexCount];
	unsigned long* indices = new unsigned long[this->indexCount];

	// NOTE: vertices are created CLOCKWISE. Somehow this determines where the GPU thinks the
	// object is facing, so wrong order could result in unintentional face culling. Need to learn
	// more about this.
	// NOTE2: notice how Textures are represented with XMFLOAT2 because it's using UV (texel). So
	// here we are mapping a texture coordinate to a polygon vertex. Kind of like an anchor and
	// interpolation will be done by looking at the texture rather than lerping a color.
	vertices[0].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);  // bot left
	vertices[0].texture = DirectX::XMFLOAT2(0.0f, 1.0f);
	vertices[0].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertices[1].position = DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f);  // top left
	vertices[1].texture = DirectX::XMFLOAT2(0.0f, 0.0f);
	vertices[1].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertices[2].position = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);  // top right
	vertices[2].texture = DirectX::XMFLOAT2(1.0f, 0.0f);
	vertices[2].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	vertices[3].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f); // bot right
	vertices[3].texture = DirectX::XMFLOAT2(1.0f, 1.0f);
	vertices[3].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);


	// Front face
	indices[0] = 0;  // t1 bot left
	indices[1] = 1;  // t1 top left
	indices[2] = 3;  // t1 bot right
	indices[3] = 3;  // t2 bot right
	indices[4] = 1;  // t2 top left
	indices[5] = 2;  // t2 top right

	// Back face
	indices[6] = 2;  // t2 top right
	indices[7] = 1;  // t2 top left
	indices[8] = 3;  // t2 bot right
	indices[9] = 3;  // t1 bot right
	indices[10] = 1;  // t1 top left
	indices[11] = 0;  // t1 bot left



	D3D11_BUFFER_DESC vertexBufferDesc;
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(Vertex) * this->vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data
	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Create the vertex buffer!
	HRESULT result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &this->pVertexBuffer);
	if (FAILED(result))
	{
		printf("ERROR: Failed to create vertex buffer.\n");
		return false;
	}

	D3D11_BUFFER_DESC indexBufferDesc;
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * this->indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data
	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &this->pIndexBuffer);
	if (FAILED(result))
	{
		printf("ERROR:Failed to create index buffer.\n");
		return false;
	}

	delete[] vertices;
	vertices = nullptr;

	delete[] indices;
	indices = nullptr;

	return true;
}

void Model::ShutdownBuffers() {
	if (this->pVertexBuffer) {
		this->pVertexBuffer->Release();
		this->pVertexBuffer = nullptr;
	}

	if (this->pIndexBuffer) {
		this->pIndexBuffer->Release();
		this->pIndexBuffer = nullptr;
	}
}

void Model::RenderBuffers(ID3D11DeviceContext* deviceContext) {
	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;

	// Set the vertex and index buffers to active in the input assembler so they can be rendered
	deviceContext->IASetVertexBuffers(0, 1, &this->pVertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(this->pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// This tells it to draw triangles. This might be fun to play around with.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

bool Model::LoadTexture(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, const char* filename) {
	this->pTexture = new Texture();
	return this->pTexture->Init(pDevice, pDeviceContext, filename);
}

void Model::ReleaseTexture() {
	if (this->pTexture) {
		this->pTexture->Shutdown();
		delete this->pTexture;
		this->pTexture = nullptr;
	}
}