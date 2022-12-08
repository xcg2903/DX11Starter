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

#include "WICTextureLoader.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include <iostream>
using namespace std;

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// Direct3D itself, and our window, are not ready at this point!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,			// The application's handle
		L"DirectX Game",	// Text for the window's title bar (as a wide-character string)
		1280,				// Width of the window's client area
		720,				// Height of the window's client area
		false,				// Sync the framerate to the monitor refresh? (lock framerate)
		true)				// Show extra stats (fps) in title bar?
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Delete all objects manually created within this class
//  - Release() all Direct3D objects created within this class
// --------------------------------------------------------
Game::~Game()
{
	// Call delete or delete[] on any objects or arrays you've
	// created using new or new[] within this class
	// - Note: this is unnecessary if using smart pointers

	// Call Release() on any Direct3D objects made within this class
	// - Note: this is unnecessary for D3D objects stored in ComPtrs

	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Called once per program, after Direct3D and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateGeometry();

	//Set up SpriteBatch
	spriteBatch = std::make_shared<SpriteBatch>(context.Get());
	
	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		context->IASetInputLayout(inputLayout.Get());

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		//context->VSSetShader(vertexShader.Get(), 0, 0);
		//context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	//Create Camera
	camera = std::make_shared<Camera>(float(windowWidth / windowHeight));

	//Lighting
	ambientColor = XMFLOAT3(0.2f, 0.1f, 0.1f); //Pink to match sky

	//Yellow Directional
	directional1 = {};
	directional1.type = LIGHT_TYPE_DIRECTIONAL;
	directional1.direction = XMFLOAT3(0.0, -1.0, 1.0);
	directional1.color = XMFLOAT3(1.0, 1.0, 0.0);
	directional1.intensity = 1.0f;
	//Magenta Directional
	directional2 = {};
	directional2.type = LIGHT_TYPE_DIRECTIONAL;
	directional2.direction = XMFLOAT3(1.0, -1.0, 0.0);
	directional2.color = XMFLOAT3(1.0, 0.0, 1.0);
	directional2.intensity = 1.0f;
	//Cyan Directional
	directional3 = {};
	directional3.type = LIGHT_TYPE_DIRECTIONAL;
	directional3.direction = XMFLOAT3(-1.0, -1.0, 0.0);
	directional3.color = XMFLOAT3(0.0, 1.0, 1.0);
	directional3.intensity = 1.0f;
	//Magenta Point
	point1 = {};
	point1.type = LIGHT_TYPE_POINT;
	point1.position = XMFLOAT3(-2.5, 3.0, -2.0);
	point1.color = XMFLOAT3(1.0, 0.0, 1.0);
	point1.range = 15.0f;
	point1.intensity = 1.0f;
	//Yellow Point
	point2 = {};
	point2.type = LIGHT_TYPE_POINT;
	point2.position = XMFLOAT3(4.0, -2.0, 0.0);
	point2.color = XMFLOAT3(1.0, 1.0, 0.0);
	point2.range = 20.0f;
	point2.intensity = 1.0f;
	//White Point
	point3 = {};
	point3.type = LIGHT_TYPE_POINT;
	point3.position = XMFLOAT3(6.0, 2.0, 0.0);
	point3.color = XMFLOAT3(1.0, 1.0, 1.0);
	point3.range = 20.0f;
	point3.intensity = 1.0f;

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();

	//Set up Shadow Map
	PrepareShadowMap();
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	//Create shader points using SimpleShader
	//Normal
	vertexShader = make_shared<SimpleVertexShader>(device, context,
		FixPath(L"VertexShader.cso").c_str());
	pixelShader = make_shared<SimplePixelShader>(device, context,
		FixPath(L"PixelShader.cso").c_str());
	//Cool Effect
	customPixelShader = make_shared<SimplePixelShader>(device, context,
		FixPath(L"CustomTestShader.cso").c_str());
	//Sky
	skyVertexShader = make_shared<SimpleVertexShader>(device, context,
		FixPath(L"SkyVertexShader.cso").c_str());
	skyPixelShader = make_shared<SimplePixelShader>(device, context,
		FixPath(L"SkyPixelShader.cso").c_str());
	//Shadows
	shadowVertexShader = make_shared<SimpleVertexShader>(device, context,
		FixPath(L"ShadowVertexShader.cso").c_str());

	
	//CREATE SKY TEXTURE
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV; //Reference for shader
	skySRV = CreateCubemap(
		FixPath(L"../../Assets/Texture/CloudsPink/right.png").c_str(),
		FixPath(L"../../Assets/Texture/CloudsPink/left.png").c_str(), 
		FixPath(L"../../Assets/Texture/CloudsPink/up.png").c_str(), 
		FixPath(L"../../Assets/Texture/CloudsPink/down.png").c_str(), 
		FixPath(L"../../Assets/Texture/CloudsPink/front.png").c_str(), 
		FixPath(L"../../Assets/Texture/CloudsPink/back.png").c_str());

	//LOAD TEXTURES
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeColorSRV; //Abledo
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(),FixPath(L"../../Assets/Texture/bronze_albedo.png").c_str(), 0,
		bronzeColorSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeNormalSRV; //Normals
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Texture/bronze_normals.png").c_str(), 0,
		bronzeNormalSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeRoughSRV; //Roughness
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Texture/bronze_roughness.png").c_str(), 0,
		bronzeRoughSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeMetalSRV; //Metal
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Texture/bronze_metal.png").c_str(), 0,
		bronzeMetalSRV.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintColorSRV; //Abledo
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Texture/paint_albedo.png").c_str(), 0,
		paintColorSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintNormalSRV; //Normals
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Texture/paint_normals.png").c_str(), 0,
		paintNormalSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintRoughSRV; //Roughness
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Texture/paint_roughness.png").c_str(), 0,
		paintRoughSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintMetalSRV; //Metal
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Texture/paint_metal.png").c_str(), 0,
		paintMetalSRV.GetAddressOf());

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleColorSRV; //Abledo
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Texture/cobblestone_albedo.png").c_str(), 0,
		cobbleColorSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleNormalSRV; //Normals
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Texture/cobblestone_normals.png").c_str(), 0,
		cobbleNormalSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleRoughSRV; //Roughness
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Texture/cobblestone_roughness.png").c_str(), 0,
		cobbleRoughSRV.GetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleMetalSRV; //Metal
	DirectX::CreateWICTextureFromFile(device.Get(), context.Get(), FixPath(L"../../Assets/Texture/cobblestone_metal.png").c_str(), 0,
		cobbleMetalSRV.GetAddressOf());


	//Define sampler state
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 8;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());

	//CREATE MATERIALS
	mat1 = make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), pixelShader, vertexShader, 0.9f, DirectX::XMFLOAT2(1, 1));
	mat1->AddTextureSRV("Albedo", bronzeColorSRV);
	mat1->AddTextureSRV("NormalMap", bronzeNormalSRV);
	mat1->AddTextureSRV("RoughnessMap", bronzeRoughSRV);
	mat1->AddTextureSRV("MetalnessMap", bronzeMetalSRV);
	mat1->AddSampler("BasicSampler", samplerState);

	mat2 = make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), pixelShader, vertexShader, 0.9f, DirectX::XMFLOAT2(1, 1));
	mat2->AddTextureSRV("Albedo", paintColorSRV);
	mat2->AddTextureSRV("NormalMap", paintNormalSRV);
	mat2->AddTextureSRV("RoughnessMap", paintRoughSRV);
	mat2->AddTextureSRV("MetalnessMap", paintMetalSRV);
	mat2->AddSampler("BasicSampler", samplerState);

	matFloor = make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), pixelShader, vertexShader, 0.9f, DirectX::XMFLOAT2(4, 4));
	matFloor->AddTextureSRV("Albedo", cobbleColorSRV);
	matFloor->AddTextureSRV("NormalMap", cobbleNormalSRV);
	matFloor->AddTextureSRV("RoughnessMap", cobbleRoughSRV);
	matFloor->AddTextureSRV("MetalnessMap", cobbleMetalSRV);
	matFloor->AddSampler("BasicSampler", samplerState);

	customMat = make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), customPixelShader, vertexShader, 0.8, DirectX::XMFLOAT2(1, 1));

	//Sky Object
	cube = std::make_shared<Mesh>(R"(Assets/Mesh/cube.obj)", device, context);
	sky = std::make_shared<Sky>(cube, skySRV, device, samplerState);
}



// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red	 = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green	 = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue	 = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 yellow	 = XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 magenta = XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 cyan	 = XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4 white	 = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	XMFLOAT3 normal = XMFLOAT3(0.0f, 0.0f, -1.0f);
	XMFLOAT2 uv = XMFLOAT2(0.0f, 0.0f);;

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself

	wstring test; 
	test = FixPath(L"../../Assets/Texture/bark_brown_02_diff_4k.jpg").c_str();
	std::cout << "" R"(test)" << std::endl;

	cube = std::make_shared<Mesh>(R"(Assets/Mesh/cube.obj)", device, context);
	cylinder = std::make_shared<Mesh>(R"(Assets/Mesh/cylinder.obj)", device, context);
	helix = std::make_shared<Mesh>(R"(Assets/Mesh/helix.obj)", device, context);
	quad = std::make_shared<Mesh>(R"(Assets/Mesh/quad.obj)", device, context);
	quaddouble = std::make_shared<Mesh>(R"(Assets/Mesh/quad_double_sided.obj)", device, context);
	sphere = std::make_shared<Mesh>(R"(Assets/Mesh/sphere.obj)", device, context);
	torus = std::make_shared<Mesh>(R"(Assets/Mesh/torus.obj)", device, context);

	//Game entities
	std::shared_ptr<GameEntity> entity1 = std::make_shared<GameEntity>(cube, mat2);
	std::shared_ptr<GameEntity> entity2 = std::make_shared<GameEntity>(cylinder, mat2);
	std::shared_ptr<GameEntity> entity3 = std::make_shared<GameEntity>(helix, mat1);
	std::shared_ptr<GameEntity> entity4 = std::make_shared<GameEntity>(quad, mat1);
	std::shared_ptr<GameEntity> entity5 = std::make_shared<GameEntity>(quaddouble, mat2);
	std::shared_ptr<GameEntity> entity6 = std::make_shared<GameEntity>(sphere, mat1);
	std::shared_ptr<GameEntity> entity7 = std::make_shared<GameEntity>(torus, mat2);
	std::shared_ptr<GameEntity> entityFloor = std::make_shared<GameEntity>(cube, matFloor);

	entity1->GetTransform()->SetPosition(-9, 0, 0);
	entity2->GetTransform()->SetPosition(-6, 0, 0);
	entity3->GetTransform()->SetPosition(-3, 0, 0);
	entity4->GetTransform()->SetPosition(0, 0, 0);
	entity5->GetTransform()->SetPosition(3, 0, 0);
	entity6->GetTransform()->SetPosition(6, 0, 0);
	entity7->GetTransform()->SetPosition(9, 0, 0);
	entityFloor->GetTransform()->SetPosition(0, -3, 0);
	entityFloor->GetTransform()->SetScale(16, 1, 16);

	entities.push_back(entity1);
	entities.push_back(entity2);
	entities.push_back(entity3);
	entities.push_back(entity4);
	entities.push_back(entity5);
	entities.push_back(entity6);
	entities.push_back(entity7);
	entities.push_back(entityFloor);
}


// --------------------------------------------------------
// Handle resizing to match the new window size.
//  - DXCore needs to resize the back buffer
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();

	//Update aspect ratio on camera
	if (camera != NULL)
	{
		camera->UpdateProjMatrix(float(windowWidth / windowHeight));
	}

}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	//Call ImGui update
	UpdateImGui(deltaTime);

	//Call camera update
	camera->Update(deltaTime);

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();

	//Move
	float speed = 1.0f;
	entities[0]->GetTransform()->SetPosition(-9, (sin(totalTime + 0.1) * speed), 0);
	entities[1]->GetTransform()->SetPosition(-6, (sin(totalTime + 0.2) * speed), 0);
	entities[2]->GetTransform()->SetPosition(-3, (sin(totalTime + 0.3) * speed), 0);
	entities[3]->GetTransform()->SetPosition(0,  (sin(totalTime + 0.4) * speed), 0);
	entities[4]->GetTransform()->SetPosition(3,  (sin(totalTime + 0.5) * speed), 0);
	entities[5]->GetTransform()->SetPosition(6,  (sin(totalTime + 0.6) * speed), 0);
	entities[6]->GetTransform()->SetPosition(9,  (sin(totalTime + 0.7) * speed), 0);

	//Move the Sun (Update the Shadow-Casting Light's View Matrix)
	XMMATRIX shView = XMMatrixLookAtLH(
		XMVectorSet(0, 20 * sin(totalTime + 0.1), 20 * cos(totalTime + 0.1), 0),
		XMVectorSet(0, 0, 0, 0),
		XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&shadowView, shView);
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erases what's on the screen)
		const float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f }; // Cornflower Blue
		context->ClearRenderTargetView(backBufferRTV.Get(), bgColor);

		// Clear the depth buffer (resets per-pixel occlusion information)
		context->ClearDepthStencilView(depthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	//Render a shadow map before any other objects
	RenderShadowMap();

	for (auto& i : entities)
	{
		i->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor); //Send world ambient to shader

		//Send ShadowMap matricies to the vertex shader for calculations
		vertexShader->SetMatrix4x4("shadowView", shadowView);
		vertexShader->SetMatrix4x4("shadowProj", shadowProj);

		//Send ShadowMap resources to pixel shader for sampling
		pixelShader->SetShaderResourceView("ShadowMap", shadowSRV);
		pixelShader->SetSamplerState("ShadowSampler", shadowSampler);

		i->GetMaterial()->GetPixelShader()->SetData(
			"dirLight1", // The name of the (eventual) variable in the shader
			&directional1, // The address of the data to set
			sizeof(Light)); // The size of the data (the whole struct!) to set
		i->GetMaterial()->GetPixelShader()->SetData(
			"dirLight2", // The name of the (eventual) variable in the shader
			&directional2, // The address of the data to set
			sizeof(Light)); // The size of the data (the whole struct!) to set
		i->GetMaterial()->GetPixelShader()->SetData(
			"dirLight3", // The name of the (eventual) variable in the shader
			&directional3, // The address of the data to set
			sizeof(Light)); // The size of the data (the whole struct!) to set
		i->GetMaterial()->GetPixelShader()->SetData(
			"pointLight1", // The name of the (eventual) variable in the shader
			&point1, // The address of the data to set
			sizeof(Light)); // The size of the data (the whole struct!) to set
		i->GetMaterial()->GetPixelShader()->SetData(
			"pointLight2", // The name of the (eventual) variable in the shader
			&point2, // The address of the data to set
			sizeof(Light)); // The size of the data (the whole struct!) to set
		i->GetMaterial()->GetPixelShader()->SetData(
			"pointLight3", // The name of the (eventual) variable in the shader
			&point3, // The address of the data to set
			sizeof(Light)); // The size of the data (the whole struct!) to set

		i->Draw(context, camera);
	}

	//Draw Sky
	sky->Draw(context, skyVertexShader, skyPixelShader, camera);

	// Draw ImGui
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present the back buffer to the user
		//  - Puts the results of what we've drawn onto the window
		//  - Without this, the user never sees anything
		swapChain->Present(vsync ? 1 : 0, 0);

		// Must re-bind buffers after presenting, as they become unbound
		context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	}
}

void Game::PrepareShadowMap()
{
	//Values
	shadowResolution = 1024;
	shadowProjSize = 30.0f;

	//Define the texture
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowResolution;
	shadowDesc.Height = shadowResolution;
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	//Define Depth/Stencil
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	device->CreateDepthStencilView(shadowTexture.Get(), &shadowDSDesc, shadowDSV.GetAddressOf());

	// Define Shadow Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc = {};
	shadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRVDesc.Texture2D.MipLevels = 1;
	shadowSRVDesc.Texture2D.MostDetailedMip = 0;
	device->CreateShaderResourceView(shadowTexture.Get(), &shadowSRVDesc, shadowSRV.GetAddressOf());

	// Define Comparison State
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR; // This is the comparison filter
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f;
	shadowSampDesc.BorderColor[1] = 1.0f;
	shadowSampDesc.BorderColor[2] = 1.0f;
	shadowSampDesc.BorderColor[3] = 1.0f;
	device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	//Define Rasterizer state
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000;
	shadowRastDesc.DepthBiasClamp = 0.0f;
	shadowRastDesc.SlopeScaledDepthBias = 1.0f;
	device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);


	//View (Starting Value)
	XMMATRIX shView = XMMatrixLookAtLH(
		XMVectorSet(0, 20, -20, 0),
		XMVectorSet(0, 0, 0, 0),
		XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&shadowView, shView);

	//Projection
	XMMATRIX shProj = XMMatrixOrthographicLH(shadowProjSize, shadowProjSize, 0.1f, 100.0f);
	XMStoreFloat4x4(&shadowProj, shProj);
}

void Game::RenderShadowMap()
{
	// Set up render pipeline
	context->OMSetRenderTargets(0, 0, shadowDSV.Get());
	context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	context->RSSetState(shadowRasterizer.Get());

	//Create viewport using the defined resolution
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)shadowResolution;
	viewport.Height = (float)shadowResolution;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	context->RSSetViewports(1, &viewport);

	//Activate the NEW vertex shader and send data to it
	shadowVertexShader->SetShader();
	shadowVertexShader->SetMatrix4x4("view", shadowView);
	shadowVertexShader->SetMatrix4x4("projection", shadowProj);
	context->PSSetShader(0, 0, 0); //Don't use pixel shader

	// Loop and draw all entities
	for (auto& e : entities)
	{
		shadowVertexShader->SetMatrix4x4("world", e->GetTransform()->GetWorldMatrix());
		shadowVertexShader->CopyAllBufferData();

		// Draw the mesh
		e->GetMesh()->Draw();
	}

	//SpriteBatch TESTING
	/*
	spriteBatch->Begin();
	spriteBatch->Draw(shadowSRV.Get(), XMFLOAT2(0, 0));
	spriteBatch->End();
	*/

	//Return to the normal screen
	context->OMSetRenderTargets(1, backBufferRTV.GetAddressOf(), depthBufferDSV.Get());
	viewport.Width = (float)this->windowWidth;
	viewport.Height = (float)this->windowHeight;
	context->RSSetViewports(1, &viewport);
	context->RSSetState(0);
}


void Game::UpdateImGui(float deltaTime)
{
	// Get a reference to our custom input manager
	Input& input = Input::GetInstance();

	// Reset input manager's gui state so we don’t
	// taint our own input (you’ll uncomment later)
	input.SetKeyboardCapture(false);
	input.SetMouseCapture(false);

	// Feed fresh input data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)this->windowWidth;
	io.DisplaySize.y = (float)this->windowHeight;
	io.KeyCtrl = input.KeyDown(VK_CONTROL);
	io.KeyShift = input.KeyDown(VK_SHIFT);
	io.KeyAlt = input.KeyDown(VK_MENU);
	io.MousePos.x = (float)input.GetMouseX();
	io.MousePos.y = (float)input.GetMouseY();
	io.MouseDown[0] = input.MouseLeftDown();
	io.MouseDown[1] = input.MouseRightDown();
	io.MouseDown[2] = input.MouseMiddleDown();
	io.MouseWheel = input.GetMouseWheel();
	input.GetKeyArray(io.KeysDown, 256);

	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture (you’ll uncomment later)
	input.SetKeyboardCapture(io.WantCaptureKeyboard);
	input.SetMouseCapture(io.WantCaptureMouse);

	// Show the demo window
	//ImGui::ShowDemoWindow();
	
	
	//My ImGui Stuff
	
	//Controls Window
	ImGui::Begin("Control Window");
	ImGui::Text("Below are the controls I am implementing for this simulation. Yippeee!!!");
	ImGui::Text("");

	/*
	//Auto generate controls for every entity in the scene
	if (ImGui::CollapsingHeader("Entity Controls"))
	{
		for (int i = 0; i < entities.size(); i++)
		{
			ImGui::Text("Entity %d", i);
			ImGui::PushID(i);

			//Edit position
			XMFLOAT3 pos = entities[i]->GetTransform()->GetPosition();
			if (ImGui::DragFloat3("Position", &pos.x, 0.05f))
			{
				// Something changed, so overwrite the transform’s data
				entities[i]->GetTransform()->SetPosition(pos.x, pos.y, pos.z);
			}

			ImGui::PopID();
			ImGui::Text("");
		}
	}
	*/

	ImGui::Text("");
	if (ImGui::CollapsingHeader("Point Light Controls"))
	{
		ImGui::Text("Point Light 1");
		XMFLOAT3 pos1 = point1.position;
		if (ImGui::DragFloat3("Position##1", &pos1.x, 0.05f))
		{
			// Something changed, so overwrite the transform’s data
			point1.position = XMFLOAT3(pos1.x, pos1.y, pos1.z);
		}
		XMFLOAT3 col1 = point1.color;
		if (ImGui::DragFloat3("Color##1", &col1.x, 0.01f, 0.0f, 1.0f))
		{
			// Something changed, so overwrite the transform’s data
			point1.color = XMFLOAT3(col1.x, col1.y, col1.z);
		}

		ImGui::Text("Point Light 2");
		XMFLOAT3 pos2 = point2.position;
		if (ImGui::DragFloat3("Position##2", &pos2.x, 0.05f))
		{
			// Something changed, so overwrite the transform’s data
			point2.position = XMFLOAT3(pos2.x, pos2.y, pos2.z);
		}
		XMFLOAT3 col2 = point2.color;
		if (ImGui::DragFloat3("Color##2", &col2.x, 0.01f, 0.0f, 1.0f))
		{
			// Something changed, so overwrite the transform’s data
			point2.color = XMFLOAT3(col2.x, col2.y, col2.z);
		}
	}

	ImGui::End();

	ImGui::Begin("The INFO Window");
	ImGui::Text("Framerate: (%1.0f)", io.Framerate);
	ImGui::Text("Number of Entities: (%d)", entities.size());
	ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
	ImGui::End();
}

// --------------------------------------------------------
// Author: Chris Cascioli
// Purpose: Creates a cube map on the GPU from 6 individual textures
// 
// - You are allowed to directly copy/paste this into your code base
//   for assignments, given that you clearly cite that this is not
//   code of your own design.
//
// - Note: This code assumes you’re putting the function in Game.cpp, 
//   you’ve included WICTextureLoader.h and you have an ID3D11Device 
//   ComPtr called “device”.  Make any adjustments necessary for
//   your own implementation.
// --------------------------------------------------------

// --------------------------------------------------------
// Loads six individual textures (the six faces of a cube map), then
// creates a blank cube map and copies each of the six textures to
// another face.  Afterwards, creates a shader resource view for
// the cube map and cleans up all of the temporary resources.
// --------------------------------------------------------
Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> Game::CreateCubemap(
	const wchar_t* right,
	const wchar_t* left,
	const wchar_t* up,
	const wchar_t* down,
	const wchar_t* front,
	const wchar_t* back)
{
	// Load the 6 textures into an array.
	// - We need references to the TEXTURES, not SHADER RESOURCE VIEWS!
	// - Explicitly NOT generating mipmaps, as we don't need them for the sky!
	// - Order matters here!  +X, -X, +Y, -Y, +Z, -Z
	Microsoft::WRL::ComPtr<ID3D11Texture2D> textures[6] = {};
	CreateWICTextureFromFile(device.Get(), right, (ID3D11Resource**)textures[0].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), left, (ID3D11Resource**)textures[1].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), up, (ID3D11Resource**)textures[2].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), down, (ID3D11Resource**)textures[3].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), front, (ID3D11Resource**)textures[4].GetAddressOf(), 0);
	CreateWICTextureFromFile(device.Get(), back, (ID3D11Resource**)textures[5].GetAddressOf(), 0);

	// We'll assume all of the textures are the same color format and resolution,
	// so get the description of the first shader resource view
	D3D11_TEXTURE2D_DESC faceDesc = {};
	textures[0]->GetDesc(&faceDesc);

	// Describe the resource for the cube map, which is simply 
	// a "texture 2d array" with the TEXTURECUBE flag set.  
	// This is a special GPU resource format, NOT just a 
	// C++ array of textures!!!
	D3D11_TEXTURE2D_DESC cubeDesc = {};
	cubeDesc.ArraySize = 6;            // Cube map!
	cubeDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE; // We'll be using as a texture in a shader
	cubeDesc.CPUAccessFlags = 0;       // No read back
	cubeDesc.Format = faceDesc.Format; // Match the loaded texture's color format
	cubeDesc.Width = faceDesc.Width;   // Match the size
	cubeDesc.Height = faceDesc.Height; // Match the size
	cubeDesc.MipLevels = 1;            // Only need 1
	cubeDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE; // This should be treated as a CUBE, not 6 separate textures
	cubeDesc.Usage = D3D11_USAGE_DEFAULT; // Standard usage
	cubeDesc.SampleDesc.Count = 1;
	cubeDesc.SampleDesc.Quality = 0;

	// Create the final texture resource to hold the cube map
	Microsoft::WRL::ComPtr<ID3D11Texture2D> cubeMapTexture;
	device->CreateTexture2D(&cubeDesc, 0, cubeMapTexture.GetAddressOf());

	// Loop through the individual face textures and copy them,
	// one at a time, to the cube map texure
	for (int i = 0; i < 6; i++)
	{
		// Calculate the subresource position to copy into
		unsigned int subresource = D3D11CalcSubresource(
			0,  // Which mip (zero, since there's only one)
			i,  // Which array element?
			1); // How many mip levels are in the texture?

		// Copy from one resource (texture) to another
		context->CopySubresourceRegion(
			cubeMapTexture.Get(),  // Destination resource
			subresource,           // Dest subresource index (one of the array elements)
			0, 0, 0,               // XYZ location of copy
			textures[i].Get(),     // Source resource
			0,                     // Source subresource index (we're assuming there's only one)
			0);                    // Source subresource "box" of data to copy (zero means the whole thing)
	}

	// At this point, all of the faces have been copied into the 
	// cube map texture, so we can describe a shader resource view for it
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = cubeDesc.Format;         // Same format as texture
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE; // Treat this as a cube!
	srvDesc.TextureCube.MipLevels = 1;        // Only need access to 1 mip
	srvDesc.TextureCube.MostDetailedMip = 0;  // Index of the first mip we want to see

	// Make the SRV
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cubeSRV;
	device->CreateShaderResourceView(cubeMapTexture.Get(), &srvDesc, cubeSRV.GetAddressOf());

	// Send back the SRV, which is what we need for our shaders
	return cubeSRV;
}