#include "ShaderInclude.hlsli"

//Create our constant buffer and register it to a slot on the pipeline
cbuffer ExternalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
}

// VStoPS struct for shadow map creation
struct VertexToPixelShadow
{
	float4 screenPosition	: SV_POSITION;
};


VertexToPixelShadow main(VertexShaderInput input)
{
	VertexToPixelShadow output;

	//Calculate screen position of this pixel
	//This is the projection of the shadow from the light's point of view
	matrix wvp = mul(projection, mul(view, world));
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	return output;
}