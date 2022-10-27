#pragma once

#include <DirectXMath.h>
#include <iostream>
#include <unordered_map>
#include "SimpleShader.h"

class Material
{
public:
	Material(DirectX::XMFLOAT4 colorTint,
		std::shared_ptr<SimplePixelShader> pixelShader,
		std::shared_ptr<SimpleVertexShader> vertexShader,
		float roughness);
	~Material();

	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	DirectX::XMFLOAT4 GetColorTint();
	float GetRoughness();

	void SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader);
	void SetColorTint(DirectX::XMFLOAT4 colorTint);

	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	void PrepareMaterial();

private:
	DirectX::XMFLOAT4 colorTint;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	float roughness;

	//Will use strings as keys to reference various textures/samplers a given material will need
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};

