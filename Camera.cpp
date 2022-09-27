#include "Camera.h"
#include "Input.h"
#include <DirectXMath.h>
#include <iostream>
using namespace DirectX;

Camera::Camera(float aspectRatio)
{
	moveSpeed = 5.0f;
	lookSpeed = 1.0f;
	this->aspectRatio = aspectRatio;

	transform.SetPosition(0, 0, -5.0f);
	UpdateProjMatrix(aspectRatio);
	UpdateViewMatrix();
}

Camera::~Camera()
{
}

void Camera::UpdateProjMatrix(float aspectRatio)
{
	//Create and store Projection
	XMMATRIX proj = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, 0.1f, 100.0f);
	XMStoreFloat4x4(&projMatrix, proj);
}

void Camera::UpdateViewMatrix()
{
	//Grab values from transform methods
	XMFLOAT3 pos = transform.GetPosition();
	XMFLOAT3 forward = transform.GetForward();

	//Load values and create View
	XMMATRIX view = XMMatrixLookToLH(XMLoadFloat3(&pos), XMLoadFloat3(&forward), XMVectorSet(0, 1, 0 ,0));
	XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::Update(float dt)
{
	//Reference Singleton
	Input& input = Input::GetInstance();

	//Key Movement
	//Q IS UP
	//E IS DOWN
	if (input.KeyDown('W'))
	{ 
		transform.MoveRelative(0, 0, 1 * moveSpeed * dt);
	}
	if (input.KeyDown('S'))
	{
		transform.MoveRelative(0, 0, -1 * moveSpeed * dt);
	}
	if (input.KeyDown('D'))
	{
		transform.MoveRelative(1 * moveSpeed * dt, 0, 0);
	}
	if (input.KeyDown('A'))
	{
		transform.MoveRelative(-1 * moveSpeed * dt, 0, 0);
	}
	if (input.KeyDown('Q'))
	{
		transform.MoveAbsolute(0, 1 * moveSpeed * dt, 0);
	}
	if (input.KeyDown('E'))
	{
		transform.MoveAbsolute(0, -1 * moveSpeed * dt, 0);
	}

	//Mouse Rotation
	if (input.MouseLeftDown())
	{
		//How much the mouse has moved since last frame
		int cursorMovementX = input.GetMouseXDelta();
		int cursorMovementY = input.GetMouseYDelta();

		//Apply to rotation
		transform.Rotate(cursorMovementY * dt * lookSpeed, cursorMovementX * dt * lookSpeed, 0);
	}

	//Apply movement to view
	UpdateViewMatrix();
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	return viewMatrix;
}
DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projMatrix;
}
