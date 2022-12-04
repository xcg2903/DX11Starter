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

	//Set up material with texture
	material->PrepareMaterial();

	//Define what the shaders will do, now using SimpleShader and our Material!
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

	//Pixel Shader References
	ps->SetFloat4("colorTint", material->GetColorTint()); // Strings here MUST
	ps->SetFloat3("cameraPos", camera->GetTransform().GetPosition());
	ps->SetFloat("roughness", material->GetRoughness());
	ps->SetFloat2("uvScale", material->GetUVScale());

	//Vertex Shader References
	vs->SetMatrix4x4("world", transform.GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vs->SetMatrix4x4("worldInvTranspose", transform.GetInveseTranspose());

	//Map resource to the GPU itself
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	//Call draw functions on Mesh Class
	mesh->Draw();
}
