#pragma once
#include "Transform.h"

class Camera
{
public:
	Camera(float aspectRatio);
	~Camera();

	void UpdateProjMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float dt);

	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	Transform GetTransform();

private:
	Transform transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	float aspectRatio;
	float moveSpeed;
	float lookSpeed;
};

