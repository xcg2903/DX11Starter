#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"
#include <iostream>

class GameEntity
{
public:
	GameEntity(std::shared_ptr<Mesh> mesh);
	~GameEntity();

	std::shared_ptr<Mesh> GetMesh();
	Transform* GetTransform();

	void Draw(Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext, 
		Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer,
		std::shared_ptr<Camera> camera);

private:
	Transform transform;
	std::shared_ptr<Mesh> mesh;
};

