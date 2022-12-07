cbuffer CBuf
{
	matrix modelView;
	matrix modelViewProj;
};

struct VSOut
{
	float4 pos : SV_Position;
	float2 tc : Texcoord;
};

VSOut main(float3 pos : Position, float2 tc : Texcoord)
{
	VSOut vso;
	vso.pos = mul(float4(pos, 1.0f), modelViewProj);
	vso.tc = tc;
	return vso;
}