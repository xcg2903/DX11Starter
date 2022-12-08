TextureCube SkyTexture : register(t0);
TextureCube SkyTextureNight : register(t1);
SamplerState BasicSampler : register(s0);

struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position			: SV_POSITION;
	float3 sampleDir		: DIRECTION;
	float totalTime			: TIME;
};

float4 main(VertexToPixel input) : SV_TARGET
{
	//Lerp between the two textures based on the totalTime
	float3 outputColor = lerp(SkyTextureNight.Sample(BasicSampler, input.sampleDir).rgb,
	SkyTexture.Sample(BasicSampler, input.sampleDir).rgb,
	(clamp(sin(input.totalTime) * 1.5 + 0.5, 0, 1)));

	return float4(outputColor, 1.0f);
}