#pragma once

#include <DirectXMath.h>

class Light {
public:
	Light();

	void SetDirection(float, float, float);
	void SetDiffuseColor(float, float, float, float);

	DirectX::XMFLOAT3 GetDirection();
	DirectX::XMFLOAT4 GetDiffuseColor();

private:
	DirectX::XMFLOAT3 direction;
	DirectX::XMFLOAT4 diffuseColor;
};