#include "FBXSkeleton.h"
#include "Animation.h"

Skeleton::~Skeleton()
{
	for (auto bone : hierarchy) {
		delete bone;
	}
}

void Skeleton::ConvertFromFbx(Graphics& gfx, FBXSkeleton* _skele) {
	dx::XMFLOAT3 _translation; 
	dx::XMVECTORF32 _rotation;
	dx::XMFLOAT3 _inv_translation;
	dx::XMVECTORF32 _inv_rotation;
	
	for (auto& bone : _skele->bones) {
		_translation.x = bone->BindPos[0];
		_translation.y = bone->BindPos[1];
		_translation.z = bone->BindPos[2];

		_rotation.f[0] = bone->BindRot.GetAt(0);
		_rotation.f[1] = bone->BindRot.GetAt(1);
		_rotation.f[2] = bone->BindRot.GetAt(2);

		_inv_translation.x = bone->BoneSpacePos[0];
		_inv_translation.y = bone->BoneSpacePos[1];
		_inv_translation.z = bone->BoneSpacePos[2];

		_inv_rotation.f[0] = bone->BoneSpaceRot.GetAt(0);
		_inv_rotation.f[1] = bone->BoneSpaceRot.GetAt(1);
		_inv_rotation.f[2] = bone->BoneSpaceRot.GetAt(2);

		hierarchy.push_back(new Bone(
			gfx,
			bone->BoneIndex,
			bone->ParentIndex,
			_translation, _rotation,
			_inv_translation, _inv_rotation)
		);
	}
}

void Skeleton::Initialize() {
	for (unsigned int i = 0; i < hierarchy.size(); ++i)
	{
		Bone* bone = hierarchy[i];

		if (bone->parent_indx != -1)
			((hierarchy[bone->parent_indx])->children).push_back(bone);
	}
}

void Skeleton::Draw(Graphics& gfx) {
	hierarchy[0]->Draw(gfx);
}



Bone::Bone(Graphics& _gfx, int _indx, int _parent_indx, dx::XMFLOAT3 _translation, dx::XMVECTORF32 _rotation, 
			dx::XMFLOAT3 _inv_translation, dx::XMVECTORF32 _inv_rotation)
	: Line(_gfx, { 1.0f, 0.0f, 0.0f, 0.0f }), 
	 bone_indx(_indx), parent_indx(_parent_indx), 
	 translation(_translation), rotation(_rotation),
	 inv_translation(_inv_translation), inv_rotation(_inv_rotation)
{
}

DirectX::XMMATRIX Bone::GetTransformXM() const noexcept {
	namespace dx = DirectX;
	//return dx::XMMatrixTranslation(translation.x, translation.y, translation.z) * dx::XMMatrixRotationQuaternion(rotation);
	return dx::XMMatrixTranslation(0.0f, 0.0f, 0.0f) * dx::XMMatrixRotationQuaternion(rotation);
}

void Bone::Update(float dt) noexcept
{
}
