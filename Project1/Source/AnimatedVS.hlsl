cbuffer CBuf
{
	matrix modelView;
	matrix modelViewProj;
};

cbuffer cbBones
{
	matrix bone_transforms[96];
};

struct VSOut
{
	float4 pos : SV_Position;
	float3 normal : Normal;
	float2 tc : Texcoord;
	float4 weights : Weights;
};

VSOut main(float3 pos : Position, float3 normal : Normal, float2 tc : Texcoord, 
			float4 weights : Weights, uint4 weight_indx : Weightindex)
{
	VSOut vso;
	float3 pos_l;
	if (any(weights)) {
		matrix m = 0;

		for (uint i = 0; i < 4; i++) {
			m += bone_transforms[weight_indx[i]] * weights[i];
			//m += bone_transforms[weight_indx[i]];
		}

		pos = mul(m, float4(pos, 1.0f));
		normal = normalize(mul((float3x3)m, normal));
	}

	vso.pos = mul(float4(pos, 1.0f), modelViewProj);
	vso.normal = mul(normal, (float3x3)modelView);
	vso.tc = tc;
	vso.weights = weights;
	return vso;
}