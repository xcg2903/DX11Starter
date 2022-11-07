cbuffer ExternalData : register(b0)
{
	matrix view;
	matrix projection;
}

struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float3 localPosition	: POSITION;     // XYZ position
	float3 normal			: NORMAL;        // RGBA color
	float2 uv				: TEXCOORD;        // RGBA color
	float3 tangent			: TANGENT;
};

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

VertexToPixel main(VertexShaderInput input)
{
	// Set up output struct
	VertexToPixel output;

	matrix viewNoTranslation = view; //Copy view matrix
	viewNoTranslation._14 = 0; //Remove translation from view matrix
	viewNoTranslation._24 = 0;
	viewNoTranslation._34 = 0;

	//Apply matrices to position
	output.position = mul(mul(projection, viewNoTranslation), float4(input.localPosition, 1.0f));
	//Update output depth
	output.position.z = output.position.w;

	//Equal since the box will always be at the origin
	output.sampleDir = input.localPosition;

	return output;
}