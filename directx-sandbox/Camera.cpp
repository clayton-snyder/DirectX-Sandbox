#include "Camera.h"

const double DEG_TO_RAD = 0.0174532925;

Camera::Camera() {
	this->positionX = 0.0f;
	this->positionY = 0.0f;
	this->positionZ = 0.0f;

	this->rotationX = 0.0f;
	this->rotationY = 0.0f;
	this->rotationZ = 0.0f;
}

Camera::Camera(const Camera& other) {
}

Camera::~Camera() {
}

void Camera::SetPosition(float x, float y, float z) {
	this->positionX = x;
	this->positionY = y;
	this->positionZ = z;
}

void Camera::SetRotation(float x, float y, float z) {
	this->rotationX = x;
	this->rotationY = y;
	this->rotationZ = z;
}

DirectX::XMFLOAT3 Camera::GetPosition() {
	return DirectX::XMFLOAT3(this->positionX, this->positionY, this->positionZ);
}

DirectX::XMFLOAT3 Camera::GetRotation() {
	return DirectX::XMFLOAT3(this->rotationX, this->rotationY, this->rotationZ);
}

void Camera::Render() {
	DirectX::XMFLOAT3 up = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);
	DirectX::XMVECTOR upVector = DirectX::XMLoadFloat3(&up);

	DirectX::XMFLOAT3 position = DirectX::XMFLOAT3(this->positionX, this->positionY, this->positionZ);
	DirectX::XMVECTOR positionVector = DirectX::XMLoadFloat3(&position);

	DirectX::XMFLOAT3 lookAt = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	DirectX::XMVECTOR lookAtVector = DirectX::XMLoadFloat3(&lookAt);

	float pitch = rotationX * DEG_TO_RAD;
	float yaw = rotationY * DEG_TO_RAD;
	float roll = rotationZ * DEG_TO_RAD;
	DirectX::XMMATRIX rotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	// Transform lookAt and up based on rotation matrix
	lookAtVector = DirectX::XMVector3TransformCoord(lookAtVector, rotationMatrix);
	upVector = DirectX::XMVector3TransformCoord(upVector, rotationMatrix);

	// Move rotated camera to viewer position and finally set the view matrix
	lookAtVector = DirectX::XMVectorAdd(positionVector, lookAtVector);
	viewMatrix = DirectX::XMMatrixLookAtLH(positionVector, lookAtVector, upVector);
}

// Note how we're returning by setting the passed reference. Yucky tbh.
void Camera::GetViewMatrix(DirectX::XMMATRIX& viewMatrix) {
	viewMatrix = this->viewMatrix;
}