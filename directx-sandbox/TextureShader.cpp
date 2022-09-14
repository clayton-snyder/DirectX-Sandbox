#include "TextureShader.h"

TextureShader::TextureShader() {
	this->pVertexShader = nullptr;
	this->pPixelShader = nullptr;
	this->pLayout = nullptr;
	this->pMatrixBuffer = nullptr;
	this->pSamplerState = nullptr;
}

TextureShader::TextureShader(const TextureShader& other) {
}

TextureShader::~TextureShader() {
}

bool TextureShader::Init(ID3D11Device* pDevice, HWND hWnd) {
	return this->InitShader(pDevice, hWnd, L"./TextureVs.hlsl", L"./TexturePs.hlsl");
}

void TextureShader::Shutdown() {
	this->ShutdownShader();
}

bool TextureShader::Render(ID3D11DeviceContext* dCtx, int indexCount, ID3D11ShaderResourceView* pTexture,
	DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix)
{
	// Setting the shader paramteres externally like we talked about in Color.vs
	bool result = this->SetShaderParams(dCtx, pTexture, worldMatrix, viewMatrix, projectionMatrix);

	// Now render the prepared buffers with the shader
	if (result) RenderShader(dCtx, indexCount);

	return result;
}

bool TextureShader::InitShader(ID3D11Device* pDevice, HWND hWnd, const wchar_t* vsFilename, const wchar_t* psFilename) {
	ID3D10Blob* pErrorMessage = nullptr;
	ID3D10Blob* pVertexShaderBuffer = nullptr; // compiled vertex shader
	ID3D10Blob* pPixelShaderBuffer = nullptr;  // compiled shader buffer
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];

	// Now we attempt to load and compile the shaders into a buffer. Interesting.
	HRESULT result = D3DCompileFromFile(vsFilename, nullptr, nullptr, "TextureVertexShader", "vs_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &pVertexShaderBuffer, &pErrorMessage);
	if (FAILED(result)) {
		// If the file loaded but just compilation failed, errorMessage should be populated
		if (pErrorMessage)
			this->OutputShaderErrorMessage(pErrorMessage, hWnd, vsFilename);
		else
			// If there's no error message, probably it couldn't find the file
			MessageBox(hWnd, vsFilename, L"Missing shader file?", MB_OK);
		return false;
	}
	printf("Compiled vertex shader.\n");

	result = D3DCompileFromFile(psFilename, nullptr, nullptr, "TexturePixelShader", "ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPixelShaderBuffer, &pErrorMessage);
	if (FAILED(result)) {
		if (pErrorMessage)
			this->OutputShaderErrorMessage(pErrorMessage, hWnd, vsFilename);
		else
			MessageBox(hWnd, psFilename, L"Missing shader file?", MB_OK);
		return false;
	}
	printf("Compiled pixel shader.\n");

	// Now that the shader source files are compiled, we can construct objects from their buffers
	result = pDevice->CreateVertexShader(
		pVertexShaderBuffer->GetBufferPointer(), pVertexShaderBuffer->GetBufferSize(), NULL, &(this->pVertexShader));
	if (FAILED(result)) {
		printf("ERROR: Failed to create vertex shader from buffer.\n");
		return false;
	}

	result = pDevice->CreatePixelShader(
		pPixelShaderBuffer->GetBufferPointer(), pPixelShaderBuffer->GetBufferSize(), NULL, &(this->pPixelShader));
	if (FAILED(result)) {
		printf("ERROR: Failed to create pixel shader from buffer.\n");
		return false;
	}
	printf("Vertex and pixel shaders instantiated from compiled bytes!\n");

	// Create the vertex input layout description
	// ***->This setup needs to match the Vertex stucture in the Model class and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD"; // Notice this matches the semantic in the HLSL files
	polygonLayout[1].SemanticIndex = 0; // And this is the index placed after the semantic name!
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT; //DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = pDevice->CreateInputLayout(polygonLayout, numElements, pVertexShaderBuffer->GetBufferPointer(),
		pVertexShaderBuffer->GetBufferSize(), &(this->pLayout));
	if (FAILED(result)) return false;

	pVertexShaderBuffer->Release();
	pVertexShaderBuffer = nullptr;

	pPixelShaderBuffer->Release();
	pPixelShaderBuffer = nullptr;

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBuffer);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = pDevice->CreateBuffer(&matrixBufferDesc, nullptr, &(this->pMatrixBuffer));
	if (FAILED(result)) return false;

	// Create the texture SamplerState. Filter = LERP for minification and magnification. Wrap 
	// texel coord so that > 1.0 loops back to 0.0.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = pDevice->CreateSamplerState(&samplerDesc, &this->pSamplerState);

	return !FAILED(result);
}

void TextureShader::ShutdownShader() {
	if (this->pMatrixBuffer) {
		this->pMatrixBuffer->Release();
		this->pMatrixBuffer = nullptr;
	}

	if (this->pLayout) {
		this->pLayout->Release();
		this->pLayout = nullptr;
	}

	if (this->pPixelShader) {
		this->pPixelShader->Release();
		this->pPixelShader = nullptr;
	}

	if (this->pVertexShader) {
		this->pVertexShader->Release();
		this->pVertexShader = nullptr;
	}

	if (this->pSamplerState) {
		this->pSamplerState->Release();
		this->pSamplerState = nullptr;
	}
}

void TextureShader::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, const wchar_t* shaderFilename)
{
	char* compileErrors;
	unsigned long long bufferSize, i;
	std::ofstream fout;

	compileErrors = (char*)(errorMessage->GetBufferPointer());
	bufferSize = errorMessage->GetBufferSize();
	fout.open("shader-error.txt");

	for (i = 0; i < bufferSize; i++) fout << compileErrors[i];

	fout.close();
	errorMessage->Release();
	errorMessage = nullptr;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", shaderFilename, MB_OK);
}

// The matrices are passed in here from the Graphics class, and this function sends them into the
// VertexShader (after transposing them!) during the Render call. Also pass the texture data.
bool TextureShader::SetShaderParams(ID3D11DeviceContext* pDeviceContext, ID3D11ShaderResourceView* pTexture,
	DirectX::XMMATRIX worldMatrix, DirectX::XMMATRIX viewMatrix, DirectX::XMMATRIX projectionMatrix)
{
	unsigned int bufferNumber;

	// Transpose the matrices to prepare them for shader
	worldMatrix = DirectX::XMMatrixTranspose(worldMatrix);
	viewMatrix = DirectX::XMMatrixTranspose(viewMatrix);
	projectionMatrix = DirectX::XMMatrixTranspose(projectionMatrix);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = pDeviceContext->Map(pMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		printf("ERROR: Locking constant buffer failed.\n");
		return false;
	}

	// This is a pointer to the actual matrix data in the constant buffer
	MatrixBuffer* dataPtr = (MatrixBuffer*)mappedResource.pData;
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;
	pDeviceContext->Unmap(pMatrixBuffer, 0);

	// Finally set the updated matrix buffer in the vertex shader with the transposed data
	bufferNumber = 0;
	pDeviceContext->VSSetConstantBuffers(bufferNumber, 1, &this->pMatrixBuffer);
	pDeviceContext->PSSetShaderResources(0, 1, &pTexture);

	return true;
}

// SetShaderParams should have been called before this
void TextureShader::RenderShader(ID3D11DeviceContext* pDeviceContext, int indexCount) {
	// First set the layout for vertex input
	pDeviceContext->IASetInputLayout(this->pLayout);

	// Set the vertex and pixel shaders to be used for rendering
	pDeviceContext->VSSetShader(pVertexShader, nullptr, 0);
	pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);
	pDeviceContext->PSSetSamplers(0, 1, &pSamplerState);

	// Render it!
	// (I'm guessing that DrawIndexed is the version of Draw that uses an index map for faster
	// vertex lookup)
	pDeviceContext->DrawIndexed(indexCount, 0, 0);
}