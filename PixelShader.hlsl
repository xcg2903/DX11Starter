#include "ShaderInclude.hlsli"

//Colortint cbuffer
cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float3 cameraPos;
	float roughness;
	float3 ambient;
}

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	//Normalize normals
	input.normal = normalize(input.normal);

	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	//return float4(ambient, 1);
	return float4(input.normal, 1); // This is temporary
	//return float4(roughness.rrr, 1); //Test Roughness
	//return float4(input.uv, 0, 1); //Test UV coordinates
}