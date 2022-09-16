#include "Light.h"

Light::Light() {
	this->direction = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	this->diffuseColor = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);
}

DirectX::XMFLOAT3 Light::GetDirection() {
	return this->direction;
}

DirectX::XMFLOAT4 Light::GetDiffuseColor() {
	return this->diffuseColor;
}

void Light::SetDirection(float x, float y, float z) {
	this->direction = DirectX::XMFLOAT3(x, y, z);
}

void Light::SetDiffuseColor(float r, float g, float b, float a) {
	this->diffuseColor = DirectX::XMFLOAT4(r, g, b, a);
}