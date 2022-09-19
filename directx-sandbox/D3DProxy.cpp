#include "D3DProxy.h"

D3DProxy::D3DProxy() {
	this->pSwapChain = nullptr;
	this->pDevice = nullptr;
	this->pDeviceContext = nullptr;
	this->pRenderTargetView = nullptr;
	this->pDepthStencilBuffer = nullptr;
	this->pDepthStencilState = nullptr;
	this->pDepthStencilView = nullptr;
	this->pRasterState = nullptr;
}

bool D3DProxy::Init(int screenW, int screenH, bool vsync, HWND hWnd, 
	bool fullscreen, float screenDepth, float screenNear)
{
	HRESULT result;
	IDXGIFactory* factory;
	IDXGIAdapter* adapter;
	IDXGIOutput* adapterOutput;
	unsigned int numModes, i, numerator, denominator;
	unsigned long long stringLength;
	DXGI_MODE_DESC* displayModeList;
	DXGI_ADAPTER_DESC adapterDesc;
	int error;
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	D3D_FEATURE_LEVEL featureLevel;
	ID3D11Texture2D* backBufferPtr;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_RASTERIZER_DESC rasterDesc;
	D3D11_VIEWPORT viewport;
	float fieldOfView, screenAspect;

	this->vsyncEnabled = vsync;

	// DGXI = DirectX Graphics Interface
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
	if (FAILED(result)) return false;

	// Use factory to create adapter for primary graphics interface (video card)
	result = factory->EnumAdapters(0, &adapter);
	if (FAILED(result)) return false;

	// Enumerate the primary adapter output (monitor)
	result = adapter->EnumOutputs(0, &adapterOutput);
	if (FAILED(result)) return false;

	// Get the number of modes that fit the DXGI_FORMAT_R8G8B8A8_UNORM 
	// display format for the adapter output (monitor)
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 
		DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
	if (FAILED(result))
	{
		return false;
	}

	// List to hold all possible display modes for this monitor + video card
	displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList)
	{
		return false;
	}

	// Now fill the display mode list structures
	result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 
		DXGI_ENUM_MODES_INTERLACED, &numModes, displayModeList);
	if (FAILED(result))
	{
		return false;
	}

	// Now go through all the display modes and find the one that matches the 
	// screen width and height. When a match is found store the numerator and 
	// denominator of the refresh rate for that monitor
	for (i = 0; i < numModes; i++)
	{
		if (displayModeList[i].Width == (unsigned int)screenW)
		{
			if (displayModeList[i].Height == (unsigned int)screenH)
			{
				numerator = displayModeList[i].RefreshRate.Numerator;
				denominator = displayModeList[i].RefreshRate.Denominator;
			}
		}
	}

	// Get the adapter (video card) description
	result = adapter->GetDesc(&adapterDesc);
	if (FAILED(result))
	{
		return false;
	}

	// Store the dedicated video card memory in megabytes
	this->videoCardMem = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// Convert the name of the video card to a character array and store it.
	error = wcstombs_s(&stringLength, this->videoCardDescription, 128, 
						adapterDesc.Description, 128);
	if (error != 0) return false;

	// Clean up now that we have monitor/video card info
	delete[] displayModeList;
	displayModeList = nullptr;
	adapterOutput->Release();
	adapterOutput = nullptr;
	adapter->Release();
	adapter = nullptr;
	factory->Release();
	factory = nullptr;

	// Swap chain is the front and back buffer to which graphics will be drawn. 
	// I.e., draw to back buffer, then swap to the front to display to the 
	// screen to avoid redrawing
	ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
	swapChainDesc.BufferCount = 1; // single back buffer
	swapChainDesc.BufferDesc.Width = screenW;
	swapChainDesc.BufferDesc.Height = screenH;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; 

	// Set the refresh rate of the back buffer.
	if (this->vsyncEnabled) {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else {
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.OutputWindow = hWnd;

	// Multisampling off
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.Windowed = fullscreen;

	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// Discard the back buffer contents after presenting
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	swapChainDesc.Flags = 0;
	featureLevel = D3D_FEATURE_LEVEL_11_0;
	
	// This can fail if the primary video card is not DirectX 11 compatible.
	result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, 
		NULL, 0, &featureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, 
		&(this->pSwapChain), &(this->pDevice), NULL, &(this->pDeviceContext));
	if (FAILED(result)) return false;


	result = this->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
											(LPVOID*)&backBufferPtr);
	if (FAILED(result)) return false;

	// This attaches the back buffer pointer to the swap chain
	result = this->pDevice->CreateRenderTargetView(
		backBufferPtr, NULL, &(this->pRenderTargetView));
	if (FAILED(result)) return false;

	// Don't need anymore now that it's been used to create the RenderTargetView
	backBufferPtr->Release();
	backBufferPtr = nullptr;

	// Depth buffer description init and setup
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
	depthBufferDesc.Width = screenW;
	depthBufferDesc.Height = screenH;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// This 2D texture (depth buffer) is where sorted and rasterized 
	// polygons are placed to then be drawn
	result = this->pDevice->CreateTexture2D(
		&depthBufferDesc, NULL, &(this->pDepthStencilBuffer));
	if (FAILED(result)) return false;

	// Depth stencil setup (allows us to control type of depth test Direct3D 
	// does for each pixel)
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	result = this->pDevice->CreateDepthStencilState(
		&depthStencilDesc, &(this->pDepthStencilState));
	if (FAILED(result)) return false;

	this->pDeviceContext->OMSetDepthStencilState(this->pDepthStencilState, 1);

	// Create the view of the depth stencil buffer so Direct3D knows to use the 
	// depth buffer as a depth stencil texture.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;
	result = this->pDevice->CreateDepthStencilView(
		this->pDepthStencilBuffer, &depthStencilViewDesc, 
		&(this->pDepthStencilView));
	if (FAILED(result)) return false;

	// Bind render target view + depth stencil buffer to output render pipeline.
	this->pDeviceContext->OMSetRenderTargets(
		1, &(this->pRenderTargetView), this->pDepthStencilView);

	// Set up custom rasterizer state so we can override control of how 
	// polygons are rendered. There is a default rasterizer state but we 
	// need this if we want to have any custom control
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	result = this->pDevice->CreateRasterizerState(
		&rasterDesc, &(this->pRasterState));
	if (FAILED(result)) return false;

	this->pDeviceContext->RSSetState(this->pRasterState);

	// Set up the viewport for rendering
	viewport.Width = (float)screenW;
	viewport.Height = (float)screenH;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	this->pDeviceContext->RSSetViewports(1, &viewport);

	// Projection matrix used to translate 3D scene into 2D viewport space. 
	// We need to keep a copy to pass to shaders used to render scenes
	fieldOfView = 3.141592654f / 4.0f;
	screenAspect = (float)screenW / (float)screenH;
	this->pProjectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
		fieldOfView, screenAspect, screenNear, screenDepth);

	// World matrix used to convert vertices of objects into vertices in the 3D 
	// scene, and to rotate, translate, and scale objects in 3D space. 
	// Also will be passed to shaders for rendering
	this->pWorldMatrix = DirectX::XMMatrixIdentity();

	// Orthographic projection matrix for 2D rendering 
	// (e.g., UI/HUD/text skipping 3D rendering)
	this->pOrthoMatrix = DirectX::XMMatrixOrthographicLH(
		(float)screenW, (float)screenH, screenNear, screenDepth);

	return true;
}

void D3DProxy::Shutdown()
{
	// Set to windowed mode before releasing swap chain to avoid an exception
	if (this->pSwapChain)
	{
		this->pSwapChain->SetFullscreenState(false, NULL);
	}

	if (this->pRasterState)
	{
		this->pRasterState->Release();
		this->pRasterState = nullptr;
	}

	if (this->pDepthStencilView)
	{
		this->pDepthStencilView->Release();
		this->pDepthStencilView = nullptr;
	}

	if (this->pDepthStencilState)
	{
		this->pDepthStencilState->Release();
		this->pDepthStencilState = nullptr;
	}

	if (this->pDepthStencilBuffer)
	{
		this->pDepthStencilBuffer->Release();
		this->pDepthStencilBuffer = nullptr;
	}

	if (this->pRenderTargetView)
	{
		this->pRenderTargetView->Release();
		this->pRenderTargetView = nullptr;
	}

	if (this->pDeviceContext)
	{
		this->pDeviceContext->Release();
		this->pDeviceContext = nullptr;
	}

	if (this->pDevice)
	{
		this->pDevice->Release();
		this->pDevice = nullptr;
	}

	if (this->pSwapChain)
	{
		this->pSwapChain->Release();
		this->pSwapChain = nullptr;
	}
}

// Call when drawing a new 3D scene at the beginning of a frame; inits buffers
void D3DProxy::BeginScene(float red, float green, float blue, float alpha) {
	float color[4];

	// Setup the color to clear the buffer to.
	color[0] = red;
	color[1] = green;
	color[2] = blue;
	color[3] = alpha;

	this->pDeviceContext->ClearRenderTargetView(this->pRenderTargetView, color); // back buffer
	this->pDeviceContext->ClearDepthStencilView(
		this->pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0); // depth buffer
}

// Display the back buffer once drawing is complete
void D3DProxy::EndScene() {
	// Present the back buffer to the screen since rendering is complete.
	if (this->vsyncEnabled) {
		// Lock to screen refresh rate.
		this->pSwapChain->Present(1, 0);
	}
	else {
		// Present as fast as possible.
		this->pSwapChain->Present(0, 0);
	}
}

ID3D11Device* D3DProxy::GetDevice() {
	return this->pDevice;
}


ID3D11DeviceContext* D3DProxy::GetDeviceContext() {
	return this->pDeviceContext;
}

void D3DProxy::GetProjectionMatrix(DirectX::XMMATRIX& projectionMatrix) {
	projectionMatrix = this->pProjectionMatrix;
}


void D3DProxy::GetWorldMatrix(DirectX::XMMATRIX& worldMatrix) {
	worldMatrix = this->pWorldMatrix;
}


void D3DProxy::GetOrthoMatrix(DirectX::XMMATRIX& orthoMatrix) {
	orthoMatrix = this->pOrthoMatrix;
}

void D3DProxy::GetVideoCardInfo(char* cardName, int& memory) {
	strcpy_s(cardName, 128, this->videoCardDescription);
	memory = this->videoCardMem;
}