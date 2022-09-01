#pragma once

#include <wrl/client.h>
#include <d3d11.h>
#include "Vertex.h"

class Mesh
{
private:
	//Buffers to hold vertex and index data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	//Used for draw commands
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext;

	//Number of indices in index buffer
	int indices;

public:
	Mesh(
		Vertex* vertexArray,		//My verticies for this mesh
		int verticies,				//Vertex count
		unsigned int* indexArray,	//My indices for this mesh (order of points)
		int indicies,				//Index count
		Microsoft::WRL::ComPtr<ID3D11Device> device,
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext
		);
	~Mesh();

	//Methods
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();
	unsigned int GetIndexCount();
	void Draw();
};

