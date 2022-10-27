#include "Animation.h"

Skeleton::~Skeleton()
{
	for (auto bone : hierarchy) {
		delete bone;
	}
}

void Skeleton::ConvertFromFbx(FBXSkeleton* _skele) {
	dx::XMFLOAT3 _translation; 
	dx::XMFLOAT4 _rotation;
	dx::XMFLOAT3 _inv_translation;
	dx::XMFLOAT4 _inv_rotation;
	VQS _bind_transform, _inv_bind_transform;

	for (auto& bone : _skele->bones) {
		_translation.x = bone->BindPos[0];
		_translation.y = bone->BindPos[1];
		_translation.z = bone->BindPos[2];

		_rotation.x = bone->BindRot.GetAt(0);
		_rotation.y = bone->BindRot.GetAt(1);
		_rotation.z = bone->BindRot.GetAt(2);
		_rotation.w = bone->BindRot.GetAt(3);
		
		_bind_transform.SetV(_translation);
		_bind_transform.SetQ(_rotation);

		_inv_translation.x = bone->BoneSpacePos[0];
		_inv_translation.y = bone->BoneSpacePos[1];
		_inv_translation.z = bone->BoneSpacePos[2];

		_inv_rotation.x = bone->BoneSpaceRot.GetAt(0);
		_inv_rotation.y = bone->BoneSpaceRot.GetAt(1);
		_inv_rotation.z = bone->BoneSpaceRot.GetAt(2);
		_inv_rotation.w = bone->BoneSpaceRot.GetAt(3);

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

void Skeleton::ProcessAnimationGraph(float time, std::vector<dx::XMMATRIX>& matrix_buffer,
									 Animation& anim, std::vector<TrackData>& track_buffer) {
	for (unsigned int boneIndex = 0; boneIndex < hierarchy.size(); ++boneIndex)
	{
		Bone* bone = hierarchy[boneIndex];
		VQS animation_transform;
		anim.CalculateTransform(time, boneIndex, animation_transform, track_buffer[boneIndex]);
		dx::XMMATRIX parent_transform = bone->parent_indx != -1 ? matrix_buffer[bone->parent_indx] : dx::XMMatrixIdentity();
		dx::XMMATRIX local_transform = animation_transform.toMatrix();
		dx::XMMATRIX modelTransform = local_transform * parent_transform;
		matrix_buffer[boneIndex] = modelTransform;
	}
}

void Skeleton::ProcessBindPose(std::vector<dx::XMMATRIX>& buffer) {
	for (unsigned int i = 0; i < hierarchy.size(); ++i) {
		buffer[i] = hierarchy[i]->bind_transform.toMatrix();
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
	delete skeleton;
	
	for (auto& animation : animations)
		delete animation;
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

void AnimationController::Process() {
	skeleton->ProcessAnimationGraph(animation_time, bone_matrix_buffer, *active_animation, animation_track_data);
}

void AnimationController::ProcessBindPose() {
	skeleton->ProcessBindPose(bone_matrix_buffer);
}

void AnimationController::SetSkel(Skeleton* skel) {
	skeleton = skel;
	bone_matrix_buffer.resize(skeleton->hierarchy.size());
	animation_track_data.resize(skeleton->hierarchy.size());
	ClearTrackData();
}

void AnimationController::AddAnimation(Animation* anim) {
	active_animation = anim;
	animations.push_back(anim);
}

Animation::Animation() : duration(0.0f) {
}

Animation::~Animation() {
}

void Animation::CalculateTransform(float animTime, int trackIndex, VQS& animation_transform, TrackData& data) {
	int curr_key = data.last_key;
	Track& curr_path = tracks[trackIndex];

	//Search Forward in the keyframes for the interval
	while (curr_key != curr_path.key_frames.size() - 1 &&
		curr_path.key_frames[curr_key + 1].time < animTime)
		++curr_key;

	//Search Backward in the keyframes for the interval
	while (curr_key != 0 && curr_path.key_frames[curr_key].time > animTime)
		--curr_key;

	if (curr_key == curr_path.key_frames.size() - 1)
	{
		//Clamp animation to the last frame
		animation_transform = curr_path.key_frames[curr_key].transform;
	}
	else
	{
		//Interpolate between the two frames
		KeyFrame& key_0 = curr_path.key_frames[curr_key];
		KeyFrame& key_1 = curr_path.key_frames[curr_key + 1];

		float t1 = key_0.time;
		float t2 = key_1.time;

		//Normalize t to the range 0..1
		float normalized_t = (animTime - t1) / (t2 - t1);
		animation_transform = key_0.transform.InterpolateTo(key_1.transform, normalized_t);
		VQS temp_transform = key_0.transform.InterpolateTo(key_1.transform, normalized_t);
	}

	//Remember the last keyframe
	data.last_key = curr_key;
}

void Animation::ConvertFromFbx(FBXAnimation* fbx_animation) {
	duration = fbx_animation->duration.GetSecondDouble();
	KeyFrame new_key_frame;
	int frame_count = 0;
	for (auto& fbx_track : fbx_animation->tracks) {
		Track new_track;
		for (auto& fbx_key_frame : fbx_track.key_frames) {
			new_key_frame.time = fbx_key_frame.time.GetSecondDouble();
			if (frame_count == 209)
				double test_time = fbx_key_frame.time.GetSecondDouble();
			new_key_frame.transform = VQS(
				dx::XMFLOAT3(fbx_key_frame.translation[0], fbx_key_frame.translation[1], fbx_key_frame.translation[2]),
				Quaternion(fbx_key_frame.rotation[3], dx::XMFLOAT3(fbx_key_frame.rotation[0], fbx_key_frame.rotation[1], fbx_key_frame.rotation[2])),
				1.0f
			);
			new_track.key_frames.push_back(new_key_frame);
			frame_count++;
		}
		tracks.push_back(new_track);
	}
}
