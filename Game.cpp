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
	ambientColor = XMFLOAT3(0.0f, 0.1f, 0.25f);

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
	//Red Point
	point1 = {};
	point1.type = LIGHT_TYPE_POINT;
	point1.position = XMFLOAT3(-2.5, -1.0, 4.0);
	point1.color = XMFLOAT3(1.0, 0.0, 0.0);
	point1.range = 15.0f;
	point1.intensity = 2.0f;
	//Green Point
	point2 = {};
	point2.type = LIGHT_TYPE_POINT;
	point2.position = XMFLOAT3(6.0, -3.0, 0.0);
	point2.color = XMFLOAT3(0.0, 1.0, 0.0);
	point2.range = 20.0f;
	point2.intensity = 2.0f;

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(device.Get(), context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();
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
	vertexShader = make_shared<SimpleVertexShader>(device, context,
		FixPath(L"VertexShader.cso").c_str());
	pixelShader = make_shared<SimplePixelShader>(device, context,
		FixPath(L"PixelShader.cso").c_str());

	customPixelShader = make_shared<SimplePixelShader>(device, context,
		FixPath(L"CustomTestShader.cso").c_str());

	//CREATE MATERIALS
	mat1 = make_shared<Material>(DirectX::XMFLOAT4(1, 1, 0, 1), pixelShader, vertexShader, 0.2);
	mat2 = make_shared<Material>(DirectX::XMFLOAT4(0, 1, 1, 1), pixelShader, vertexShader, 0.4);
	mat3 = make_shared<Material>(DirectX::XMFLOAT4(1, 0, 1, 1), pixelShader, vertexShader, 0.6);
	customMat = make_shared<Material>(DirectX::XMFLOAT4(1, 1, 1, 1), customPixelShader, vertexShader, 0.8);
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
	
	//Initalize Vertex Arrays
	Vertex verticesTriangle[] =
	{
		{ XMFLOAT3(+0.5f, +0.7f, +0.0f), normal, uv },
		{ XMFLOAT3(+0.7f, +0.2f, +0.0f), normal, uv },
		{ XMFLOAT3(+0.3f, +0.2f, +0.0f), normal, uv },
	};
	/*
	Vertex verticesSquare[] =
	{
		{ XMFLOAT3(-0.7f, +0.7f, +0.0f), yellow },
		{ XMFLOAT3(-0.2f, +0.7f, +0.0f), cyan },
		{ XMFLOAT3(-0.2f, +0.4f, +0.0f), yellow },
		{ XMFLOAT3(-0.7f, +0.4f, +0.0f), magenta },
	};
	Vertex verticesOct[] =
	{
		{ XMFLOAT3(+0.0f, +0.0f, +0.0f), white },
		{ XMFLOAT3(+0.0f, +0.3f, +0.0f), cyan },
		{ XMFLOAT3(+0.2f, +0.2f, +0.0f), white },
		{ XMFLOAT3(+0.3f, +0.0f, +0.0f), magenta },
		{ XMFLOAT3(+0.2f, -0.2f, +0.0f), white },
		{ XMFLOAT3(+0.0f, -0.3f, +0.0f), yellow },
		{ XMFLOAT3(-0.2f, -0.2f, +0.0f), white },
		{ XMFLOAT3(-0.3f, +0.0f, +0.0f), red },
		{ XMFLOAT3(-0.2f, +0.2f, +0.0f), white },
	};
	*/

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...

	//Initalize Index Arrays
	unsigned int indicesTriangle[] = { 0, 1, 2 };
	unsigned int indicesSquare[] = { 0, 1, 2, 0, 2, 3 };
	unsigned int indicesOct[] = { 0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 5, 0, 5, 6, 0, 6, 7, 0, 7, 8, 0, 8, 1 };

	//Initalize Shared Pointers to meshes
	//triangle = std::make_shared<Mesh>(verticesTriangle, ARRAYSIZE(verticesTriangle), indicesTriangle, ARRAYSIZE(indicesTriangle), device, context);
	//square = std::make_shared<Mesh>(verticesSquare, ARRAYSIZE(verticesSquare), indicesSquare, ARRAYSIZE(indicesSquare), device, context);
	//octagon = std::make_shared<Mesh>(verticesOct, ARRAYSIZE(verticesOct), indicesOct, ARRAYSIZE(indicesOct), device, context);

	cube = std::make_shared<Mesh>(R"(Assets/cube.obj)", device, context);
	cylinder = std::make_shared<Mesh>(R"(Assets/cylinder.obj)", device, context);
	helix = std::make_shared<Mesh>(R"(Assets/helix.obj)", device, context);
	quad = std::make_shared<Mesh>(R"(Assets/quad.obj)", device, context);
	quaddouble = std::make_shared<Mesh>(R"(Assets/quad_double_sided.obj)", device, context);
	sphere = std::make_shared<Mesh>(R"(Assets/sphere.obj)", device, context);
	torus = std::make_shared<Mesh>(R"(Assets/torus.obj)", device, context);

	//Game entities
	std::shared_ptr<GameEntity> entity1 = std::make_shared<GameEntity>(cube, mat1);
	std::shared_ptr<GameEntity> entity2 = std::make_shared<GameEntity>(cylinder, mat2);
	std::shared_ptr<GameEntity> entity3 = std::make_shared<GameEntity>(helix, mat3);
	std::shared_ptr<GameEntity> entity4 = std::make_shared<GameEntity>(quad, mat2);
	std::shared_ptr<GameEntity> entity5 = std::make_shared<GameEntity>(quaddouble, mat2);
	std::shared_ptr<GameEntity> entity6 = std::make_shared<GameEntity>(sphere, mat1);
	std::shared_ptr<GameEntity> entity7 = std::make_shared<GameEntity>(torus, mat1);

	entity1->GetTransform()->SetPosition(-9, 0, 0);
	entity2->GetTransform()->SetPosition(-6, 0, 0);
	entity3->GetTransform()->SetPosition(-3, 0, 0);
	entity4->GetTransform()->SetPosition(0, 0, 0);
	entity5->GetTransform()->SetPosition(3, 0, 0);
	entity6->GetTransform()->SetPosition(6, 0, 0);
	entity7->GetTransform()->SetPosition(9, 0, 0);

	entities.push_back(entity1);
	entities.push_back(entity2);
	entities.push_back(entity3);
	entities.push_back(entity4);
	entities.push_back(entity5);
	entities.push_back(entity6);
	entities.push_back(entity7);
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
	entities[0]->GetTransform()->SetPosition(-9, (sin(totalTime + 0.1) * 4), 0);
	entities[1]->GetTransform()->SetPosition(-6, (sin(totalTime + 0.2) * 4), 0);
	entities[2]->GetTransform()->SetPosition(-3, (sin(totalTime + 0.3) * 4), 0);
	entities[3]->GetTransform()->SetPosition(0,  (sin(totalTime + 0.4) * 4), 0);
	entities[4]->GetTransform()->SetPosition(3,  (sin(totalTime + 0.5) * 4), 0);
	entities[5]->GetTransform()->SetPosition(6,  (sin(totalTime + 0.6) * 4), 0);
	entities[6]->GetTransform()->SetPosition(9,  (sin(totalTime + 0.7) * 4), 0);

	//std::cout << entities[0]->GetTransform()->GetPosition().x << "   " << entities[0]->GetTransform()->GetPosition().y << "   " << entities[0]->GetTransform()->GetPosition().z << std::endl;
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

	for (auto& i : entities)
	{
		i->GetMaterial()->GetPixelShader()->SetFloat3("ambient", ambientColor); //Send world ambient to shader

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

		i->Draw(context, camera);
	}

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