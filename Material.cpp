#include "Material.h"
#include "SimpleShader.h"

using namespace std;

Material::Material(DirectX::XMFLOAT4 colorTint, 
	shared_ptr<SimplePixelShader> pixelShader,
	shared_ptr<SimpleVertexShader> vertexShader,
	float roughness)
{
	this->colorTint = colorTint;
	this->pixelShader = pixelShader;
	this->vertexShader = vertexShader;
	this->roughness = roughness;
}

Material::~Material()
{
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

DirectX::XMFLOAT4 Material::GetColorTint()
{
	return colorTint;
}

float Material::GetRoughness()
{
	return roughness;
}

void Material::SetPixelShader(shared_ptr<SimplePixelShader> pixelShader)
{
	this->pixelShader = pixelShader;
}

void Material::SetVertexShader(shared_ptr<SimpleVertexShader> vertexShader)
{
	this->vertexShader = vertexShader;
}

void Material::SetColorTint(DirectX::XMFLOAT4 colorTint)
{
	this->colorTint = colorTint;
}

void Material::AddTextureSRV(string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({name, srv});
}

void Material::AddSampler(string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({name, sampler});
}
//Bind these resources to the pixel shader
void Material::PrepareMaterial()
{
	for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : samplers) { pixelShader->SetSamplerState(s.first.c_str(), s.second); }
}
