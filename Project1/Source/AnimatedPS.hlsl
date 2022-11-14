float4 main(float4 pos : SV_Position, float3 n : Normal, float2 tc : Texcoord, float4 weights : Weights) : SV_TARGET
{
	return float4(float3(0.3f, tc.x, tc.y), 1.0f);
}