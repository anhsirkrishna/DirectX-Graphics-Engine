#include "FBXSkeleton.h"
#include "Animation.h"

Skeleton::~Skeleton()
{
	for (auto bone : hierarchy) {
		delete bone;
	}
}

void Skeleton::ConvertFromFbx(FBXSkeleton* _skele) {
	dx::XMFLOAT3 _translation; 
	dx::XMFLOAT3 _rotation;
	dx::XMFLOAT3 _inv_translation;
	dx::XMFLOAT3 _inv_rotation;
	VQS _bind_transform, _inv_bind_transform;

	for (auto& bone : _skele->bones) {
		_translation.x = bone->BindPos[0];
		_translation.y = bone->BindPos[1];
		_translation.z = bone->BindPos[2];

		_rotation.x = bone->BindRot.GetAt(0);
		_rotation.y = bone->BindRot.GetAt(1);
		_rotation.z = bone->BindRot.GetAt(2);
		
		_bind_transform.SetV(_translation);
		_bind_transform.SetQ(_rotation);

		_inv_translation.x = bone->BoneSpacePos[0];
		_inv_translation.y = bone->BoneSpacePos[1];
		_inv_translation.z = bone->BoneSpacePos[2];

		_inv_rotation.x = bone->BoneSpaceRot.GetAt(0);
		_inv_rotation.y = bone->BoneSpaceRot.GetAt(1);
		_inv_rotation.z = bone->BoneSpaceRot.GetAt(2);

		_inv_bind_transform.SetV(_inv_translation);
		_inv_bind_transform.SetQ(_inv_rotation);

		hierarchy.push_back(new Bone(
			bone->BoneIndex,
			bone->ParentIndex,
			_bind_transform, 
			_inv_bind_transform)
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



Bone::Bone(int _indx, int _parent_indx, const VQS& _bind_transform, const VQS& _inv_bind_transform)
	: bone_indx(_indx), parent_indx(_parent_indx), bind_transform(_bind_transform), 
	  inv_bind_transform(_inv_bind_transform) {
}


/// <Animation Controller class>
/// Methods for  the animation controller

AnimationController::AnimationController() : 
	animation_time(0.0f), animation_speed(1.0f), skeleton(nullptr), active_animation(nullptr) {
}

AnimationController::~AnimationController() {
}

void AnimationController::Update(float dt)
{
	animation_time += dt * animation_speed;
	//Just loop forever
	if (animation_time > active_animation->duration)
	{
		animation_time = 0.0f;
		ClearTrackData();
	}
}

void AnimationController::ClearTrackData() {
	//Reset all the keys back to zero
	for (auto& animation_track : animation_track_data)
		animation_track.last_key = 0;

}