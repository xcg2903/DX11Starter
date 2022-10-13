#include "GameEntity.h"
#include <DirectXMath.h>
using namespace DirectX;

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	this->mesh = mesh;
	this->material = material;
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

std::shared_ptr<Material> GameEntity::GetMaterial()
{
	return material;
}

void GameEntity::SetMaterial(std::shared_ptr<Material>)
{
	this->material = material;
}

void GameEntity::Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext,
	std::shared_ptr<Camera> camera)
{
	//Activate the Shaders for this material
	material->GetVertexShader()->SetShader();
	material->GetPixelShader()->SetShader();

	//Define what the shaders will do, now using SimpleShader and our Material!
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

	ps->SetFloat4("colorTint", material->GetColorTint()); // Strings here MUST
	vs->SetMatrix4x4("world", transform.GetWorldMatrix()); // match variable
	vs->SetMatrix4x4("view", camera->GetViewMatrix()); // names in the
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix()); // shader’s cbuffer!

	//Map resource to the GPU itself
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	//Call draw functions on Mesh Class
	mesh->Draw();
}
