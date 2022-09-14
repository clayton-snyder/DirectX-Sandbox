#pragma once

#include "D3DProxy.h"
#include "Model.h"
#include "Camera.h"
//#include "ColorShader.h"
//#include "TextureShader.h"
#include "LightShader.h"
#include "Light.h"

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
	D3DProxy* pDirect3D;
	Camera* pCamera;
	Model* pModel;
	//ColorShader* pColorShader;
	//TextureShader* pTextureShader;
	LightShader* pLightShader;
	Light* pLight;

	bool Render(float);
};