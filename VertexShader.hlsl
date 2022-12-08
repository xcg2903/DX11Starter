#include "ShaderInclude.hlsli"

//Create our constant buffer and register it to a slot on the pipeline
cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	matrix world;
	matrix view;
	matrix projection;
	matrix worldInvTranspose;

	matrix shadowView;
	matrix shadowProj;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel main( VertexShaderInput input )
{
	// Set up output struct
	VertexToPixel output;

	// Here we're essentially passing the input position directly through to the next
	// stage (rasterizer), though it needs to be a 4-component vector now.  
	// - To be considered within the bounds of the screen, the X and Y components 
	//   must be between -1 and 1.  
	// - The Z component must be between 0 and 1.  
	// - Each of these components is then automatically divided by the W component, 
	//   which we're leaving at 1.0 for now (this is more useful when dealing with 
	//   a perspective projection matrix, which we'll get to in the future).
	// Multiply the three matrices together first
	matrix wvp = mul(projection, mul(view, world));
	output.screenPosition = mul(wvp, float4(input.localPosition, 1.0f));

	// Pass the color through 
	// - The values will be interpolated per-pixel by the rasterizer
	// - We don't need to alter it here, but we do need to send it to the pixel shader
	output.uv = input.uv; //colorTint from cbuffer

	//Update normal using the current world Matrix and pass it along
	//Using inverse transpose accounts for non-uniform scale
	output.normal = mul((float3x3)worldInvTranspose, input.normal);
	//Multiply local position by world matrix, pass along
	output.worldPosition = mul(world, float4(input.localPosition, 1)).xyz;
	//Rotate tangent by world matrix
	output.tangent = mul((float3x3)world, input.tangent);

	//Shadow Map
	//Calculate screen position of this pixel
	//This takes the shadow we created and applys the world position to it
	// Calculate where this position is from the light's point of view
	matrix shadowWVP = mul(shadowProj, mul(shadowView, world));
	output.shadowPos = mul(shadowWVP, float4(input.localPosition, 1.0f));

	// Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
	return output;
}