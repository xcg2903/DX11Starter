#include <wrl/client.h>
#include <d3d11.h>
#include "Vertex.h"
#include "Mesh.h"

//Constructor
Mesh::Mesh(
	Vertex* vertexArray,
	int verticies,
	unsigned int* indexArray,
	int indices,
	Microsoft::WRL::ComPtr<ID3D11Device> device,
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContext) 
{
	this->indices = indices;
	this->deviceContext = deviceContext;

	// Create a VERTEX BUFFER
	// - This holds the vertex data of triangles for a single object
	// - This buffer is created on the GPU, which is where the data needs to
	//    be if we want the GPU to act on it (as in: draw it to the screen)
	D3D11_BUFFER_DESC vbd = {};
	vbd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
	vbd.ByteWidth = sizeof(Vertex) * verticies;       //number of vertices in the buffer
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER; // Tells Direct3D this is a vertex buffer
	vbd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;
	// Create the proper struct to hold the initial vertex data
	D3D11_SUBRESOURCE_DATA initialVertexData = {};
	initialVertexData.pSysMem = vertexArray; // pSysMem = Pointer to System Memory
	// Actually create the buffer on the GPU with the initial data
	device->CreateBuffer(&vbd, &initialVertexData, vertexBuffer.GetAddressOf());

	// Create an INDEX BUFFER
	// - This holds indices to elements in the vertex buffer
	// - This is most useful when vertices are shared among neighboring triangles
	// - This buffer is created on the GPU, which is where the data needs to
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage = D3D11_USAGE_IMMUTABLE;	// Will NEVER change
	ibd.ByteWidth = sizeof(unsigned int) * indices;	//number of indices in the buffer
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;	// Tells Direct3D this is an index buffer
	ibd.CPUAccessFlags = 0;	// Note: We cannot access the data from C++ (this is good)
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;
	// Specify the initial data for this buffer, similar to above
	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indexArray; // pSysMem = Pointer to System Memory
	// Actually create the buffer with the initial data
	device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
}

//Destructor
Mesh::~Mesh() {

}

//Return Methods
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetVertexBuffer() {
	return vertexBuffer;
}
Microsoft::WRL::ComPtr<ID3D11Buffer> Mesh::GetIndexBuffer() {
	return indexBuffer;
}
unsigned int Mesh::GetIndexCount() {
	return indices;
};

//Draw Meshes
void Mesh::Draw() {

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	{
		// Set buffers in the input assembler (IA) stage
		//  - Do this ONCE PER OBJECT, since each object may have different geometry
		//  - For this demo, this step *could* simply be done once during Init()
		//  - However, this needs to be done between EACH DrawIndexed() call
		//     when drawing different geometry, so it's here as an example
		deviceContext->IASetVertexBuffers(0, 1, GetVertexBuffer().GetAddressOf(), &stride, &offset);
		deviceContext->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// Tell Direct3D to draw
		//  - Begins the rendering pipeline on the GPU
		//  - Do this ONCE PER OBJECT you intend to draw
		//  - This will use all currently set Direct3D resources (shaders, buffers, etc)
		//  - DrawIndexed() uses the currently set INDEX BUFFER to look up corresponding
		//     vertices in the currently set VERTEX BUFFER
		deviceContext->DrawIndexed(
			GetIndexCount(),     // The number of indices to use (we could draw a subset if we wanted)
			0,     // Offset to the first index we want to use
			0);    // Offset to add to each index when looking up vertices
	}
};

