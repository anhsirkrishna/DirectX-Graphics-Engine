float4 main(float3 pos : Position, float3 n : Normal, float2 tc : Texcoord, float4 weights : Weights) : SV_TARGET
{
	return float4(weights.xyz, 1.0f);
}