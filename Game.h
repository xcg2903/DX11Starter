#pragma once
#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "Helpers.h"
#include "Mesh.h"
#include "GameEntity.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"
#include "Lights.h"

#include "DXCore.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <iostream>
#include <vector>
#include "SimpleShader.h"

class Game 
	: public DXCore
{

public:
	Game(HINSTANCE hInstance);
	~Game();

	// Overridden setup and game loop methods, which
	// will be called automatically
	void Init();
	void OnResize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void UpdateImGui(float deltaTime);

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders(); 
	void CreateGeometry();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	//Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	//Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
	
	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;


	//Variables for testing
	std::vector<std::shared_ptr<GameEntity>> entities;

	std::shared_ptr<Camera> camera;

	std::shared_ptr<Material> mat1;
	std::shared_ptr<Material> mat2;
	std::shared_ptr<Material> mat3;
	std::shared_ptr<Material> customMat;

	std::shared_ptr<Mesh> cube;
	std::shared_ptr<Mesh> cylinder;
	std::shared_ptr<Mesh> helix;
	std::shared_ptr<Mesh> quad;
	std::shared_ptr<Mesh> quaddouble;
	std::shared_ptr<Mesh> sphere;
	std::shared_ptr<Mesh> torus;

	std::shared_ptr<SimplePixelShader> customPixelShader;

	DirectX::XMFLOAT3 ambientColor;

	Light directional1;
	Light directional2;
	Light directional3;
	Light point1;
	Light point2;

};

