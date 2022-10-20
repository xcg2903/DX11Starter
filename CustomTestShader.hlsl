#include "ShaderInclude.hlsli"

//Colortint cbuffer
cbuffer ExternalData : register(b0)
{
	float4 colorTint;
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
	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	//return colorTint;
	return float4(cos(input.screenPosition.x / 8) + sin(input.uv.x * 8), sin(input.screenPosition.x / 8) + cos(input.uv.x * 8), 0.5, 1); //Test UV coordinates
}