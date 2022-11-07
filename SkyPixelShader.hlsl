TextureCube SkyTexture : register(t0);
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
};

float4 main(VertexToPixel input) : SV_TARGET
{
	float3 outputColor = SkyTexture.Sample(BasicSampler, input.sampleDir).rgb;

	return float4(outputColor, 1.0f);
}