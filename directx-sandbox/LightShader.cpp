#include "LightShader.h"

LightShader::LightShader() {
	this->pVertexShader = nullptr;
	this->pPixelShader = nullptr;
	this->pLayout = nullptr;
	this->pMxBuf = nullptr;
	this->pLightBuf = nullptr;
	this->pSamplerState = nullptr;
}

bool LightShader::Init(ID3D11Device* pDevice, HWND hWnd) {
	return this->InitShader(pDevice, hWnd, L"./LightVs.hlsl", L"./LightPs.hlsl");
}

void LightShader::Shutdown() {
	this->ShutdownShader();
}

bool LightShader::Render(ID3D11DeviceContext* dCtx, int idxCt, ID3D11ShaderResourceView* pTex,
	DirectX::XMMATRIX worldMx, DirectX::XMMATRIX viewMx, DirectX::XMMATRIX projMx,
	DirectX::XMFLOAT3 lightDir, DirectX::XMFLOAT4 diffuseClr, DirectX::XMFLOAT4 ambientClr)
{
	// Setting the shader paramteres externally like we talked about in Color.vs
	bool result = this->SetShaderParams(dCtx, pTex, 
										worldMx, viewMx, projMx, 
										lightDir, diffuseClr, ambientClr);

	// Now render the prepared buffers with the shader
	if (result) RenderShader(dCtx, idxCt);

	return result;
}

bool LightShader::InitShader(ID3D11Device* pDevice, HWND hWnd, 
	const wchar_t* vsFilename, const wchar_t* psFilename)
{
	ID3D10Blob* pErrorMsg = nullptr;
	ID3D10Blob* pVertexShaderBuf = nullptr; // compiled vertex shader
	ID3D10Blob* pPixelShaderBuf = nullptr;  // compiled shader buffer
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3]; // position, texel, normal

	// Now we attempt to load and compile the shaders into a buffer. Interesting.
	HRESULT result = D3DCompileFromFile(vsFilename, nullptr, nullptr, "LightVertexShader", 
		"vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pVertexShaderBuf, &pErrorMsg);
	if (FAILED(result)) {
		// If the file loaded but just compilation failed, errorMessage should be populated
		if (pErrorMsg)
			this->OutputShaderErrorMessage(pErrorMsg, hWnd, vsFilename);
		else
			// If there's no error message, probably it couldn't find the file
			MessageBox(hWnd, vsFilename, L"Missing shader file?", MB_OK);
		return false;
	}
	printf("Compiled vertex shader.\n");

	result = D3DCompileFromFile(psFilename, nullptr, nullptr, "LightPixelShader",
		"ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pPixelShaderBuf, &pErrorMsg);
	if (FAILED(result)) {
		if (pErrorMsg)
			this->OutputShaderErrorMessage(pErrorMsg, hWnd, vsFilename);
		else
			MessageBox(hWnd, psFilename, L"Missing shader file?", MB_OK);
		return false;
	}
	printf("Compiled pixel shader.\n");

	// Now that the shader source files are compiled, construct objects from their buffers
	result = pDevice->CreateVertexShader(
		pVertexShaderBuf->GetBufferPointer(), pVertexShaderBuf->GetBufferSize(), 
		NULL, &(this->pVertexShader));
	if (FAILED(result)) {
		printf("ERROR: Failed to create vertex shader from buffer.\n");
		return false;
	}

	result = pDevice->CreatePixelShader(
		pPixelShaderBuf->GetBufferPointer(), pPixelShaderBuf->GetBufferSize(), 
		NULL, &(this->pPixelShader));
	if (FAILED(result)) {
		printf("ERROR: Failed to create pixel shader from buffer.\n");
		return false;
	}
	printf("Vertex and pixel shaders instantiated from compiled bytes!\n");

	// Create the vertex input layout description
	// **->This setup needs to match the Vertex stucture in the Model class and in the shader.
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD"; // Matches the semantic in the HLSL files
	polygonLayout[1].SemanticIndex = 0; // Index placed after the semantic name
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT; // Texel coords
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = pDevice->CreateInputLayout(polygonLayout, numElements, 
		pVertexShaderBuf->GetBufferPointer(), pVertexShaderBuf->GetBufferSize(), 
		&(this->pLayout));
	if (FAILED(result)) return false;

	pVertexShaderBuf->Release();
	pVertexShaderBuf = nullptr;

	pPixelShaderBuf->Release();
	pPixelShaderBuf = nullptr;

	// Set up description of the dynamic matrix constant buffer that is in the vertex shader
	D3D11_BUFFER_DESC matrixBufDesc;
	matrixBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufDesc.ByteWidth = sizeof(MatrixBuffer);
	matrixBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufDesc.MiscFlags = 0;
	matrixBufDesc.StructureByteStride = 0;

	// Create buffer pointer to access the vertex shader buffer from within this class
	result = pDevice->CreateBuffer(&matrixBufDesc, nullptr, &(this->pMxBuf));
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
	if (FAILED(result)) return false;


	D3D11_BUFFER_DESC lightBufDesc;
	lightBufDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufDesc.ByteWidth = sizeof(LightBuffer);
	lightBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufDesc.MiscFlags = 0;
	lightBufDesc.StructureByteStride = 0;

	result = pDevice->CreateBuffer(&lightBufDesc, nullptr, &this->pLightBuf);
	return !FAILED(result);
}

void LightShader::ShutdownShader() {
	if (this->pMxBuf) {
		this->pMxBuf->Release();
		this->pMxBuf = nullptr;
	}

	if (this->pLightBuf) {
		this->pLightBuf->Release();
		this->pLightBuf = nullptr;
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

void LightShader::OutputShaderErrorMessage(
	ID3D10Blob* errorMessage, HWND hwnd, const wchar_t* shaderFilename)
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

	MessageBox(hwnd, L"Error compiling shader.  Check shader-error.txt for message.", 
		shaderFilename, MB_OK);
}

// The matrices are passed in here from the Graphics class, and this function sends them into
// VertexShader (after transposing them!) during the Render call. Also pass the texture data.
bool LightShader::SetShaderParams(ID3D11DeviceContext* pDvCtx, ID3D11ShaderResourceView* pTex,
	DirectX::XMMATRIX worldMx, DirectX::XMMATRIX viewMx, DirectX::XMMATRIX projMx,
	DirectX::XMFLOAT3 lightDir, DirectX::XMFLOAT4 diffuseClr, DirectX::XMFLOAT4 ambientClr)
{
	unsigned int bufferNumber;

	// Transpose the matrices to prepare them for shader
	worldMx = DirectX::XMMatrixTranspose(worldMx);
	viewMx = DirectX::XMMatrixTranspose(viewMx);
	projMx = DirectX::XMMatrixTranspose(projMx);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT result = pDvCtx->Map(pMxBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		printf("ERROR: Locking matrix cbuffer failed.\n");
		return false;
	}
	// This is a pointer to the actual matrix data in the constant buffer
	MatrixBuffer* pMxBufData = (MatrixBuffer*)mappedResource.pData;
	pMxBufData->world = worldMx;
	pMxBufData->view = viewMx;
	pMxBufData->projection = projMx;
	pDvCtx->Unmap(pMxBuf, 0);
	pDvCtx->VSSetConstantBuffers(0, 1, &pMxBuf); // update the vertex shader

	result = pDvCtx->Map(pLightBuf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) {
		printf("ERROR: Locking light cbuffer failed.\n");
		return false;
	}
	LightBuffer* pLightBufData = (LightBuffer*)mappedResource.pData;
	pLightBufData->direction = lightDir;
	pLightBufData->diffuseColor = diffuseClr;
	pLightBufData->ambientColor = ambientClr;
	pLightBufData->padding = 0.0f;
	pDvCtx->Unmap(pLightBuf, 0);
	pDvCtx->PSSetConstantBuffers(0, 1, &pLightBuf); // update the pixel shader

	pDvCtx->PSSetShaderResources(0, 1, &pTex);

	return true;
}

// SetShaderParams should have been called before this
void LightShader::RenderShader(ID3D11DeviceContext* pDeviceContext, int indexCount) {
	// First set the layout for vertex input
	pDeviceContext->IASetInputLayout(this->pLayout);

	// Set the vertex and pixel shaders to be used for rendering
	pDeviceContext->VSSetShader(pVertexShader, nullptr, 0);
	pDeviceContext->PSSetShader(pPixelShader, nullptr, 0);
	pDeviceContext->PSSetSamplers(0, 1, &pSamplerState);

	// Render it!
	pDeviceContext->DrawIndexed(indexCount, 0, 0);
}