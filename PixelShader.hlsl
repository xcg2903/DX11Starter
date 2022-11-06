#include "ShaderInclude.hlsli"

//Colortint cbuffer
cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float3 cameraPos;
	float roughness;
	float3 ambient;
	Light dirLight1;
	Light dirLight2;
	Light dirLight3;
	Light pointLight1;
	Light pointLight2;
	Light pointLight3;
}

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D AmbientOcclusion : register(t1);
Texture2D NormalMap : register(t2); 
SamplerState BasicSampler : register(s0); // "s" registers for samplers

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
	//Initial Variable Calculations
	float3 finalColor;
	float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
	//float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb * AmbientOcclusion.Sample(BasicSampler, input.uv).r; //Sample current pixel of the texture
	float expWithRoughness = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1; //Sample normal map texture using RGB

	//Create TBN Matrix
	float3 N = normalize(input.normal); // Must be normalized here or before
	float3 T = normalize(input.tangent); // Must be normalized here or before
	T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);

	//Apply TBN matrix to normal input
	input.normal = mul(unpackedNormal, TBN); // Note multiplication order!

	//Negate light direction
	float3 dirToDirLight1 = normalize(dirLight1.direction * -1);
	float3 dirToDirLight2 = normalize(dirLight2.direction * -1);
	float3 dirToDirLight3 = normalize(dirLight3.direction * -1);

	//DIRECTIONAL LIGHTS
	//Light1
	float3 phong1 = phong(normalize(dirLight1.direction), expWithRoughness, input.normal, input.worldPosition, cameraPos);
	float3 diffuse1 = diffuse(input.normal, dirToDirLight1);
	float3 light1 = (surfaceColor * (diffuse1 + phong1)) * dirLight1.color;
	//Light2
	float3 phong2 = phong(normalize(dirLight2.direction), expWithRoughness, input.normal, input.worldPosition, cameraPos);
	float3 diffuse2 = diffuse(input.normal, dirToDirLight2);
	float3 light2 = (surfaceColor * (diffuse2 + phong2)) * dirLight2.color;
	//Light3
	float3 phong3 = phong(normalize(dirLight3.direction), expWithRoughness, input.normal, input.worldPosition, cameraPos);
	float3 diffuse3 = diffuse(input.normal, dirToDirLight3);
	float3 light3 = (surfaceColor * (diffuse3 + phong3)) * dirLight3.color;

	//POINT LIGHTS
	//Light 4
	float3 point1Dir = input.worldPosition - pointLight1.position; //Calulate from this point to point light
	float3 dirToPointLight1 = normalize(point1Dir * -1);
	float3 phong4 = phong(normalize(point1Dir), expWithRoughness, input.normal, input.worldPosition, cameraPos);
	float3 diffuse4 = diffuse(input.normal, dirToPointLight1);
	float3 light4 = (surfaceColor * (diffuse4 + phong4)) * pointLight1.color;
	light4 *= attenuate(pointLight1, input.worldPosition);
	//Light 5
	float3 point2Dir = input.worldPosition - pointLight2.position; //Calulate from this point to point light
	float3 dirToPointLight2 = normalize(point2Dir * -1);
	float3 phong5 = phong(normalize(point2Dir), expWithRoughness, input.normal, input.worldPosition, cameraPos);
	float3 diffuse5 = diffuse(input.normal, dirToPointLight2);
	float3 light5 = (surfaceColor * (diffuse5 + phong5)) * pointLight2.color;
	light5 *= attenuate(pointLight2, input.worldPosition);
	//Light 6
	float3 point3Dir = input.worldPosition - pointLight3.position; //Calulate from this point to point light
	float3 dirToPointLight3 = normalize(point3Dir * -1);
	float3 phong6 = phong(normalize(point3Dir), expWithRoughness, input.normal, input.worldPosition, cameraPos);
	float3 diffuse6 = diffuse(input.normal, dirToPointLight3);
	float3 light6 = (surfaceColor * (diffuse6 + phong6)) * pointLight3.color;
	light6 *= attenuate(pointLight3, input.worldPosition);

	//Rendering equation
	finalColor = light1 + light4 + light5 + light6 + (ambient * surfaceColor);

	// Just return the input color
	// - This color (like most values passing through the rasterizer) is 
	//   interpolated for each pixel between the corresponding vertices 
	//   of the triangle we're rendering
	//return float4(ambient, 1);
	//return float4(input.normal, 1); // Test Normals
	return float4(finalColor, 1); // Test light color
	//return float4(roughness.rrr, 1); //Test Roughness
	//return float4(input.uv, 0, 1); //Test UV coordinates
}