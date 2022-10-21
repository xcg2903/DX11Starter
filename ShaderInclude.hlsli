#ifndef __SHADER_INCLUDES__ // unique identifier
#define __SHADER_INCLUDES__

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2
#define MAX_SPECULAR_EXPONENT 128.0f

struct Light
{
	int type; //0, 1, 2
	float3 direction; // For directional/spot
	float range; //For spot/point
	float3 position; // For spot/point
	float intensity; //How bright
	float3 color; //The RGB value of the light
	float spotFalloff; // cone size for spot lights
	float3 padding; // padding to hit the 16-byte boundar
};

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