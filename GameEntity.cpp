#include "GameEntity.h"
#include "BufferStructs.h"
#include <DirectXMath.h>
using namespace DirectX;

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh)
{
	this->mesh = mesh;
}

GameEntity::~GameEntity()
{
}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
	return mesh;
}

Transform* GameEntity::GetTransform()
{
	return &transform;
}

void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext, 
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer,
	std::shared_ptr<Camera> camera)
{
	//Define what the shaders will do
	VertexShaderData vsData;
	vsData.colorTint = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	vsData.worldMatrix = transform.GetWorldMatrix();
	vsData.view = camera->GetViewMatrix();
	vsData.projection = camera->GetProjectionMatrix();

	//Map resource to the GPU itself
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	deviceContext->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	deviceContext->Unmap(vsConstantBuffer.Get(), 0);

	//Call draw functions on Mesh Class
	mesh->Draw();
}