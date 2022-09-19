#pragma once

#include <DirectXMath.h>

class Light {
public:
	Light();

	void SetDirection(float, float, float);
	void SetDiffuseColor(float, float, float, float);
	void SetAmbientColor(float, float, float, float);

	DirectX::XMFLOAT3 GetDirection();
	DirectX::XMFLOAT4 GetDiffuseColor();
	DirectX::XMFLOAT4 GetAmbientColor();

private:
	DirectX::XMFLOAT3 direction;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT4 ambientColor;
};