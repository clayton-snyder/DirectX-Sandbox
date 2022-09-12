#pragma once

#include <DirectXMath.h>

// Keeps track of the camera position and rotation and creates the view matrix passed to shaders.
class Camera {
public:
	Camera();
	Camera(const Camera&);
	~Camera();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();

	void Render(); // Creates the view matrix from position and rotation
	void GetViewMatrix(DirectX::XMMATRIX&);

private:
	float positionX, positionY, positionZ;
	float rotationX, rotationY, rotationZ;
	DirectX::XMMATRIX viewMatrix;
};