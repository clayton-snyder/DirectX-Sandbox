#include "Model.h"
#include <stdio.h>

Model::Model() {
	this->pVertexBuffer = nullptr;
	this->pIndexBuffer = nullptr;
	this->pTexture = nullptr;
	this->vertexCount = 0;
	this->indexCount = 0;
}

bool Model::Init(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, 
	const char* textureFilename, std::string modelFilename) 
{
	// Load this model's texture
	bool result = LoadTexture(pDevice, pDeviceContext, textureFilename);
	if (!result) {
		printf("ERROR: LoadTexture returned false.\n");
		return false;
	}

	result = LoadModel(modelFilename);
	if (!result) printf("ERROR: LoadModel returned false.\n");

	for (int i = 0; i < this->vertexCount; i++) {
		ModelFileRow vx = this->fileRows[i];
		printf("VERTEX %d=%.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f, %.1f\n", i,
			vx.posX, vx.posY, vx.posZ, vx.texU, 
			vx.texV, vx.normX, vx.normY, vx.normZ);
	}

	result = InitBuffers(pDevice);
	if (!result) {
		printf("ERROR: InitBuffers returned false.\n");
		return false;
	}
	printf("Model buffers initialized...\n");
	
	return result;
}

bool Model::LoadModel(std::string modelFilename) {
	std::ifstream fin;
	fin.open(modelFilename);
	if (!fin.is_open()) {
		printf("ERROR: Could not open file '%s'. Does it exist?\n", 
			modelFilename.c_str());
		return false;
	}
	std::string line = "";
	while (line.empty() && fin.good()) {
		std::getline(fin, line);
	}
	if (line.empty()) {
		printf("ERROR: Could not find first line. Empty file?\n");
		return false;
	}

	try {
		this->vertexCount = std::stoi(line);
	}
	catch (...) {
		printf("ERROR: could not parse first line for vertex count: '%s'." \
			"Expected a single int.\n", line.c_str());
		return false;
	}

	int lineCount = 1, tokensPerRow = sizeof(ModelFileRow) / sizeof(float);
	this->fileRows = new ModelFileRow[vertexCount];
	for (line; std::getline(fin, line); lineCount++) {
		printf("lineCount=%d, Processing line: %s\n", lineCount, line.c_str());
		if (line.empty()) {
			lineCount--;
			continue;
		}

		if (lineCount > vertexCount) {
			printf("ERROR: More vertices in file than specified. Expected %d\n", vertexCount);
			return false;
		}

		char* token = strtok(_strdup(line.c_str()), " ");
		float coords[TOKENS_PER_ROW] = { };
		int index = 0;
		while (token != nullptr && index < TOKENS_PER_ROW) {
			try {
				coords[index++] = std::stof(token);
			}
			catch (...) {
				printf("Error converting token '%s' to float. Check " \
					"your model file.\n", token);
				return false;
			}
			token = strtok(nullptr, " ");
		}

		this->fileRows[lineCount - 1].posX = coords[0];
		this->fileRows[lineCount - 1].posY = coords[1];
		this->fileRows[lineCount - 1].posZ = coords[2];
		this->fileRows[lineCount - 1].texU = coords[3];
		this->fileRows[lineCount - 1].texV = coords[4];
		this->fileRows[lineCount - 1].normX = coords[5];
		this->fileRows[lineCount - 1].normY = coords[6];
		this->fileRows[lineCount - 1].normZ = coords[7];
	}
	// predec lineCount because it gets an extra on the last loop
	if (--lineCount < vertexCount) {
		printf("ERROR: parsed less lines (%d) than vertex count specified (%d).\n", 
			lineCount, vertexCount);
		return false;
	}

	printf("Loaded file.\n");
	fin.close();
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

// This is where the vertex and index buffers are loaded from the model file that was read in.
bool Model::InitBuffers(ID3D11Device* device) {
	this->indexCount = this->vertexCount;
	//this->vertexCount = 4;
	//this->indexCount = 4;
	Vertex* vertices = new Vertex[this->vertexCount];
	unsigned long* indices = new unsigned long[this->indexCount];

	// NOTE: vertices are created CLOCKWISE. Somehow this determines where the GPU thinks the
	// object is facing, so wrong order could result in unintentional face culling. Need to 
	// learn more about this.
	// NOTE2: notice how Textures are represented with XMFLOAT2 because it's using UV (texel).
	// So here we are mapping a texture coordinate to a polygon vertex. Kind of like an 
	// anchor and interp will be done by looking at the texture rather than lerping a color.
	for (int i = 0; i < vertexCount; i++) {
		ModelFileRow fr = this->fileRows[i];
		vertices[i].position = DirectX::XMFLOAT3(fr.posX, fr.posY, fr.posZ);  // bot left
		vertices[i].texture = DirectX::XMFLOAT2(fr.texU, fr.texV);
		vertices[i].normal = DirectX::XMFLOAT3(fr.normX, fr.normY, fr.normZ);
		indices[i] = i;
	}
	//vertices[0].position = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);  // bot left
	//vertices[0].texture = DirectX::XMFLOAT2(0.0f, 1.0f);
	//vertices[0].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	//vertices[1].position = DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f);  // top left
	//vertices[1].texture = DirectX::XMFLOAT2(0.0f, 0.0f);
	//vertices[1].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	//vertices[2].position = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f);  // top right
	//vertices[2].texture = DirectX::XMFLOAT2(1.0f, 0.0f);
	//vertices[2].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	//vertices[3].position = DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f); // bot right
	//vertices[3].texture = DirectX::XMFLOAT2(1.0f, 1.0f);
	//vertices[3].normal = DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f);

	//indices[0] = 0;
	//indices[1] = 1;
	//indices[2] = 2;
	//indices[3] = 3;



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
	HRESULT result = device->CreateBuffer(
		&vertexBufferDesc, &vertexData, &this->pVertexBuffer);
	if (FAILED(result))
	{
		printf("ERROR: Failed to create vertex buffer: %s\n",
			std::system_category().message(result).c_str());
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

void Model::ReleaseModel() {
	if (this->fileRows) {
		delete[] fileRows;
		this->fileRows = nullptr;
	}
}