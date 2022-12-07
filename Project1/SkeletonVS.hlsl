cbuffer CBuf
{
	matrix modelView;
	matrix modelViewProj;
};

cbuffer cbBones
{
	matrix bone_transforms[96];
};

float4 main(uint2 bone_index : Bone_index) : SV_POSITION
{
	float4 pos = mul(bone_transforms[bone_index.x], float4(0.0f, 0.0f, 0.0f, 1.0f));
	return mul(pos, modelViewProj);
}