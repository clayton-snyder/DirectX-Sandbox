#pragma once

#include "D3DProxy.h"

const bool FULL_SCREEN = true;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class Graphics {
public:
	Graphics();
	Graphics(const Graphics&);
	~Graphics();

	bool Init(int, int, HWND);
	void Shutdown();
	bool Frame();

private:
	bool Render();
	D3DProxy* pDirect3D;
};