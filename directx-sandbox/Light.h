#pragma once

#include <DirectXMath.h>

class Light {
public:
	Light();

	void SetDirection(float, float, float);
	void SetDiffuseColor(float, float, float, float);
	void SetAmbientColor(float, float, float, float);
	void SetSpecularColor(float, float, float, float);
	void SetSpecularExp(float);

	DirectX::XMFLOAT3 GetDirection();
	DirectX::XMFLOAT4 GetDiffuseColor();
	DirectX::XMFLOAT4 GetAmbientColor();
	DirectX::XMFLOAT4 GetSpecularColor();
	float GetSpecularExp();

private:
	DirectX::XMFLOAT3 direction;
	DirectX::XMFLOAT4 diffuseColor;
	DirectX::XMFLOAT4 ambientColor;
	DirectX::XMFLOAT4 specularColor;
	float specularExp;
};