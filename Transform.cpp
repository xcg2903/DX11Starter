#include "Transform.h"
#include <DirectXMath.h>
using namespace DirectX;

Transform::Transform() :
	position(0, 0, 0),
	rotation(0, 0, 0),
	scale(1, 1, 1),
	dirty(false)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixIdentity());
}

Transform::~Transform()
{
}

void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	dirty = true;
}

void Transform::Rotate(float x, float y, float z)
{
	rotation.x += x;
	rotation.y += y;
	rotation.z += z;
	dirty = true;
}

void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
	dirty = true;
}

void Transform::MoveRelative(float x, float y, float z)
{
	//Get Rotation
	XMVECTOR move = XMVectorSet(x, y, z, 0);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	//Add values
	XMVECTOR currentPos = XMLoadFloat3(&position);
	move = XMVector3Rotate(move, rotQuat);
	currentPos += move;

	//Store new position
	XMStoreFloat3(&position, currentPos);
}

void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
	dirty = true;
}

void Transform::SetRotation(float x, float y, float z)
{
	rotation.x = x;
	rotation.y = y;
	rotation.z = z;
	dirty = true;
}

void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
	dirty = true;
}

DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

DirectX::XMFLOAT3 Transform::GetRotation()
{
	return rotation;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

DirectX::XMFLOAT3 Transform::GetRight()
{
	XMFLOAT3 value;

	XMVECTOR vec = XMVectorSet(0, 0, 1, 0);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	//Apply rotation and update value
	XMStoreFloat3(&value, XMVector3Rotate(vec, rotQuat));

	return value;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	XMFLOAT3 value;

	XMVECTOR vec = XMVectorSet(0, 0, 1, 0);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	//Apply rotation and update value
	XMStoreFloat3(&value, XMVector3Rotate(vec, rotQuat));

	return value;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	XMFLOAT3 value;

	XMVECTOR vec = XMVectorSet(0, 0, 1, 0);
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	//Apply rotation and update value
	XMStoreFloat3(&value, XMVector3Rotate(vec, rotQuat));

	return value;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	UpdateMatrices();
	return worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetInveseTranspose()
{
	UpdateMatrices();
	return worldInverseTranspose;
}

void Transform::UpdateMatrices()
{
	//Check if we even need to be here
	if (!dirty) {
		return;
	}

	//Align matrices
	XMMATRIX translation = XMMatrixTranslation(position.x, position.y, position.z);
	XMMATRIX scaling = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX rotational = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	//Put the three matrices together
	XMMATRIX world = scaling * rotational * translation;
	
	//Save as storage type after math is completed
	XMStoreFloat4x4(&worldMatrix, world);
	XMStoreFloat4x4(&worldInverseTranspose, XMMatrixInverse(0, XMMatrixTranspose(world))); //Save the inverse
	dirty = false;
}
