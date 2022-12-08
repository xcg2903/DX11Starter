#pragma once
#include "Mesh.h"
#include "Camera.h"
#include "SimpleShader.h"
#include <wrl/client.h>
#include <d3d11.h>
#include <iostream>

using namespace Microsoft::WRL;

class Sky
{
public:
	Sky(
	std::shared_ptr<Mesh> geometry,
	ComPtr<ID3D11ShaderResourceView> shaderResourceView,
	ComPtr<ID3D11ShaderResourceView> shaderResourceViewNight,
	ComPtr<ID3D11Device> device,
	ComPtr<ID3D11SamplerState> samplerState);
	~Sky();
	void Draw(
		ComPtr<ID3D11DeviceContext> deviceContext,
		std::shared_ptr<SimpleVertexShader> vs,
		std::shared_ptr<SimplePixelShader> ps,
		std::shared_ptr<Camera> camera,
		float totalTime);
private:
	ComPtr<ID3D11SamplerState> samplerState;
	ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	ComPtr<ID3D11ShaderResourceView> shaderResourceViewNight;
	ComPtr<ID3D11DepthStencilState> depthStencil;
	ComPtr<ID3D11RasterizerState> rasterizer;
	std::shared_ptr<Mesh> geometry;
	std::shared_ptr<SimplePixelShader> ps;
	std::shared_ptr<SimpleVertexShader> vs;
};

