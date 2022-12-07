Texture2D tex;

SamplerState splr;

float4 main(float4 pos : SV_Position, float2 tc : Texcoord) : SV_TARGET
{
	return tex.Sample(splr, tc);
}