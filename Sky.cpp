#include "Sky.h"

Sky::Sky(
	std::shared_ptr<Mesh> geometry,
	ComPtr<ID3D11ShaderResourceView> shaderResourceView,
	ComPtr<ID3D11ShaderResourceView> shaderResourceViewNight,
	ComPtr<ID3D11Device> device,
	ComPtr<ID3D11SamplerState> samplerState)
{
	this->shaderResourceView = shaderResourceView;
	this->shaderResourceViewNight = shaderResourceViewNight;
	this->samplerState = samplerState;
	this->geometry = geometry;

	//Define Rasterizer Options
	D3D11_RASTERIZER_DESC rastDesc = {};
	rastDesc.FillMode = D3D11_FILL_SOLID;
	rastDesc.CullMode = D3D11_CULL_FRONT;
	device->CreateRasterizerState(&rastDesc, rasterizer.GetAddressOf());

	//Define Depth Stencil Options
	D3D11_DEPTH_STENCIL_DESC depthStenDesc = {};
	depthStenDesc.DepthEnable = true;
	depthStenDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	device->CreateDepthStencilState(&depthStenDesc, depthStencil.GetAddressOf());
}

Sky::~Sky()
{
}

void Sky::Draw(
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext,
	std::shared_ptr<SimpleVertexShader> vs,
	std::shared_ptr<SimplePixelShader> ps,
	std::shared_ptr<Camera> camera,
	float totalTime)
{
	//Set states
	deviceContext->RSSetState(rasterizer.Get());
	deviceContext->OMSetDepthStencilState(depthStencil.Get(), 0);

	//Activate shaders
	vs->SetShader();
	ps->SetShader();

	//Vertex Shader References
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vs->SetFloat("totalTime", totalTime);

	//Pixel Shader References
	ps->SetShaderResourceView("SkyTexture", shaderResourceView);
	ps->SetShaderResourceView("SkyTextureNight", shaderResourceViewNight); //Send both textures to pixel shader
	ps->SetSamplerState("BasicSampler", samplerState);

	//Map resource to the GPU itself
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	//Draw Box
	geometry->Draw();

	//Reset render states
	deviceContext->RSSetState(0);
	deviceContext->OMSetDepthStencilState(0, 0);

}
