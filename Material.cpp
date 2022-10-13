#include "Material.h"
#include "SimpleShader.h"

using namespace std;

Material::Material(DirectX::XMFLOAT4 colorTint, 
	shared_ptr<SimplePixelShader> pixelShader,
	shared_ptr<SimpleVertexShader> vertexShader)
{
	this->colorTint = colorTint;
	this->pixelShader = pixelShader;
	this->vertexShader = vertexShader;
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
