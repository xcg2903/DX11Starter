#include "ShaderInclude.hlsli"

//Colortint cbuffer
cbuffer ExternalData : register(b0)
{
	float4 colorTint;
	float3 cameraPos;
	float roughness;
	float3 ambient;
	float2 uvScale;

	Light dirLight1;
	Light dirLight2;
	Light dirLight3;
	Light pointLight1;
	Light pointLight2;
	Light pointLight3;
}

//Textures
Texture2D Albedo : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
Texture2D ShadowMap	: register(t4);
//Samplers
SamplerState BasicSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

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
	//=INITALIZE VALUES====================================================================================================

	//Account for UV scaling
	input.uv *= uvScale;

	//Initial Variable Calculations
	float3 finalColor;
	float expWithRoughness = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;

	//SAMPLE ALBEDO
	float3 albedoColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f); //Gamma Corrected!

	//SAMPLE NORMAL AND CREATE TBN MATRIX
	float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1; //Sample normal map texture using RGB
	float3 N = normalize(input.normal); // Must be normalized here or before
	float3 T = normalize(input.tangent); // Must be normalized here or before
	T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
	float3 B = cross(T, N);
	float3x3 TBN = float3x3(T, B, N);
	//Apply TBN matrix to normal input
	input.normal = mul(unpackedNormal, TBN); // Note multiplication order!

	//ROUGHNESS
	float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
	//METALNESS
	float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

	// Specular color determination -----------------
	// Assume albedo texture is actually holding specular color where metalness == 1
	float3 specularColor = lerp(F0_NON_METAL.rrr, albedoColor.rgb, metalness);

	//View vector
	float3 dirToCamera = normalize((input.worldPosition - cameraPos) * -1);



	//=SHADOW MAPPING====================================================================================================

	// Calculate depth (distance) from light
// - Doing the perspective divide ourselves
	float lightDepth = input.shadowPos.z / input.shadowPos.w;
	// Adjust [-1 to 1] range to be [0 to 1] for UV�s
	float2 shadowUV = input.shadowPos.xy / input.shadowPos.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y; // Flip Y for sampling
	// Read shadow map for closest surface (red channel)
	float shadowDepth = ShadowMap.Sample(BasicSampler, shadowUV).r;

	//Sample using our Comparison Sampler
	float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, lightDepth);



	//=LIGHTS====================================================================================================

	//DIRECTIONAL LIGHTS
	
	//Light1 - SUN
	float3 dirToDirLight1 = normalize(dirLight1.direction * -1); 	//Negate light direction
	//Light Amounts
	float3 spec1 = MicrofacetBRDF(input.normal, dirToDirLight1, dirToCamera, roughness, specularColor);
	float3 diffuse1 = DiffusePBR(input.normal, dirToDirLight1);
	//Energy conservation
	float3 balancedDiff1 = DiffuseEnergyConserve(diffuse1, spec1, metalness);
	float3 light1 = (balancedDiff1 * albedoColor + spec1) * dirLight1.intensity * dirLight1.color;
	finalColor = light1 * shadowAmount; //This is the light casting shadow

	//Light2 - MOON
	float3 dirToDirLight2 = normalize(dirLight2.direction * -1); 	//Negate light direction
	//Light Amounts
	float3 spec2 = MicrofacetBRDF(input.normal, dirToDirLight2, dirToCamera, roughness, specularColor);
	float3 diffuse2 = DiffusePBR(input.normal, dirToDirLight2);
	//Energy conservation
	float3 balancedDiff2 = DiffuseEnergyConserve(diffuse2, spec2, metalness);
	float3 light2 = (balancedDiff2 * albedoColor + spec2) * dirLight2.intensity * dirLight2.color;
	finalColor += light2;

	//POINT LIGHTS
	// 
	//LIGHT 4 (POINT LIGHT 1)
	float3 dirToPointLight1 = normalize((input.worldPosition - pointLight1.position) * -1);
	//Light amounts
	float3 spec4 = MicrofacetBRDF(input.normal, dirToPointLight1, dirToCamera, roughness, specularColor);
	float3 diffuse4 = DiffusePBR(input.normal, dirToPointLight1);
	//Energy conservation
	float3 balancedDiff4 = DiffuseEnergyConserve(diffuse4, spec4, metalness);
	float3 light4 = (balancedDiff4 * albedoColor + spec4) * pointLight1.intensity * pointLight1.color;
	light4 *= attenuate(pointLight1, input.worldPosition);
	finalColor += light4;

	//LIGHT 5 (POINT LIGHT 2)
	float3 dirToPointLight2 = normalize((input.worldPosition - pointLight2.position) * -1);
	//Light amounts
	float3 spec5 = MicrofacetBRDF(input.normal, dirToPointLight2, dirToCamera, roughness, specularColor);
	float3 diffuse5 = DiffusePBR(input.normal, dirToPointLight2);
	//Energy conservation
	float3 balancedDiff5 = DiffuseEnergyConserve(diffuse5, spec5, metalness);
	float3 light5 = (balancedDiff5 * albedoColor + spec5) * pointLight2.intensity * pointLight2.color;
	light5 *= attenuate(pointLight2, input.worldPosition);
	finalColor += light5;

	//LIGHT 6 (POINT LIGHT 3)
	float3 dirToPointLight3 = normalize((input.worldPosition - pointLight3.position) * -1);
	//Light amounts
	float3 spec6 = MicrofacetBRDF(input.normal, dirToPointLight3, dirToCamera, roughness, specularColor);
	float3 diffuse6 = DiffusePBR(input.normal, dirToPointLight3);
	//Energy conservation
	float3 balancedDiff6 = DiffuseEnergyConserve(diffuse6, spec6, metalness);
	float3 light6 = (balancedDiff6 * albedoColor + spec6) * pointLight3.intensity * pointLight3.color;
	light6 *= attenuate(pointLight3, input.worldPosition);
	finalColor += light6;



	//=RETURN RESULT====================================================================================================

	return float4(pow(finalColor, 1.0f / 2.2f), 1); // Test light color WITH GAMMA
}