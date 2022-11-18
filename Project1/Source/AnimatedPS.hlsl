Texture2D tex;

SamplerState splr;

float4 main(float4 pos : SV_Position, float3 n : Normal, float2 tc : Texcoord, float4 weights : Weights) : SV_TARGET
{
	return tex.Sample(splr, tc);
}