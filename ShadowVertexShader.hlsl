#include "ShaderInclude.hlsli"

//Create our constant buffer and register it to a slot on the pipeline
cbuffer ExternalData : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
}

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	//Calculate position
	matrix wvp = mul(projection, mul(view, world));
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	return output;
}