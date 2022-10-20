#ifndef __SHADER_INCLUDES__ // unique identifier
#define __SHADER_INCLUDES__

// Struct representing the data we expect to receive from earlier pipeline stages
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 screenPosition	: SV_POSITION;
	float2 uv				: TEXCOORD;        // UV Coord
	float3 normal			: NORMAL;
	float3 worldPosition	: POSITION;
};

#endif