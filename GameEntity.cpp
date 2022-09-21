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
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer)
{
	//Define what the shaders will do
	VertexShaderData vsData;
	vsData.colorTint = XMFLOAT4(0.5f, 1.0f, 1.0f, 1.0f);
	vsData.worldMatrix = transform.GetWorldMatrix();

	//Map resource to the GPU itself
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	deviceContext->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
	deviceContext->Unmap(vsConstantBuffer.Get(), 0);

	//Call draw functions on Mesh Class
	mesh->Draw();

	//Hook up resource to the cBuffer in our shader
	deviceContext->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we activating? Can do multiple at once
		vsConstantBuffer.GetAddressOf()); // Array of buffers (or the address of one)
}
