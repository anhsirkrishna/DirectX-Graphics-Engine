cbuffer CBuf
{
	matrix modelView;
	matrix modelViewProj;
};

struct VSOut
{
	float4 pos : SV_Position;
	float3 normal : Normal;
	float2 tc : Texcoord;
};

VSOut main(float3 pos : Position, float3 normal : Normal, float2 tc : Texcoord, 
			float4 weights : Weights, int4 weight_indx : Weightindex)
{
	VSOut vso;
	vso.pos = mul(float4(pos, 1.0f), modelViewProj);
	vso.normal = mul(normal, (float3x3)modelView);
	vso.tc = tc;
	return vso;
}