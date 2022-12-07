#include "Animation.h"
#include "imgui/imgui.h"
#include <string>


Skeleton::~Skeleton()
{
	for (auto bone : hierarchy) {
		delete bone;
	}
}

void Skeleton::ConvertFromFbx(const FBXSkeleton* _skele) {
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

	//Collect the end effectors
	for (unsigned int i = 0; i < hierarchy.size(); ++i)
	{
		Bone* bone = hierarchy[i];

		if (bone->children.size() == 0)
			end_effectors.push_back(bone);

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

bool Skeleton::ProcessBlendAnimationGraph(float time, std::vector<dx::XMMATRIX>& matrix_buffer,
	Animation& anim_prev, Animation& anim_next, 
	std::vector<TrackData>& track_buffer, std::vector<TrackData>& next_track_buffer,
	float normalized_velo) {
	bool blend_completed = true;
	for (unsigned int boneIndex = 0; boneIndex < hierarchy.size(); ++boneIndex)
	{
		Bone* bone = hierarchy[boneIndex];
		VQS animation_transform;
		blend_completed &= anim_prev.CalculateBlendTransform(time, boneIndex, anim_next,
			animation_transform, track_buffer[boneIndex], next_track_buffer[boneIndex], normalized_velo);
		dx::XMMATRIX parent_transform = bone->parent_indx != -1 ? matrix_buffer[bone->parent_indx] : dx::XMMatrixIdentity();
		dx::XMMATRIX local_transform = animation_transform.toMatrix();
		dx::XMMATRIX modelTransform = local_transform * parent_transform;
		matrix_buffer[boneIndex] = modelTransform;
	}
	return blend_completed;
}

void Skeleton::ProcessBindPose(std::vector<dx::XMMATRIX>& buffer) {
	for (unsigned int i = 0; i < hierarchy.size(); ++i) {
		buffer[i] = hierarchy[i]->bind_transform.toMatrix();
	}
}

void Skeleton::ProcessBaseAnimationGraph(std::vector<dx::XMMATRIX>& matrix_buffer, Animation& anim) {
	for (unsigned int boneIndex = 0; boneIndex < hierarchy.size(); ++boneIndex)
	{
		Bone* bone = hierarchy[boneIndex];
		const VQS& animation_transform = anim.GetBaseTransform(boneIndex);
		dx::XMMATRIX parent_transform = bone->parent_indx != -1 ? matrix_buffer[bone->parent_indx] : dx::XMMatrixIdentity();
		dx::XMMATRIX local_transform = animation_transform.toMatrix();
		dx::XMMATRIX modelTransform = local_transform * parent_transform;
		matrix_buffer[boneIndex] = modelTransform;
	}
}

Bone::Bone(int _indx, int _parent_indx, const VQS& _bind_transform, const VQS& _inv_bind_transform)
	: bone_indx(_indx), parent_indx(_parent_indx), bind_transform(_bind_transform), 
	  inv_bind_transform(_inv_bind_transform) {
}


/// <Animation Controller class>
/// Methods for  the animation controller

AnimationController::AnimationController() : 
	animation_time(0.0f), animation_speed(1.0f), skeleton(nullptr), 
	active_animation(nullptr), animation_blending(false), animation_run_threshold(0.0f),
	next_animation(nullptr) {
}

AnimationController::~AnimationController() {
	for (auto& animation : animations) {
		delete animation;
	}
}

void AnimationController::Update(float dt) {
	if (animation_path) {
		animation_path->Update(dt);
		float curr_velo = animation_path->GetCurrentVelocity();
		animation_speed = curr_velo / active_animation->pace;
		if (curr_velo < 50) {
			//Idle animation
			SwitchAnimation(0);
		}
		else if (curr_velo > animation_path->constant_velocity * (2.0f / 3)) {
			//Run animation
			SwitchAnimation(1);
		}
		else {
			//Walk animation
			SwitchAnimation(2);
		}
	}

	animation_time += dt * animation_speed;
	//Just loop forever
	if (animation_time > active_animation->duration)
	{
		animation_time = 0.0f;
		ClearTrackData();

		//End any blending that's happening
		if (animation_blending) {
			active_animation = next_animation;
			next_animation = nullptr;
			animation_blending = false;
		}
	}
	if (animation_path) {
		ShowPathControls();
		ShowAnimationControls();
	}
}

void AnimationController::ClearTrackData() {
	//Reset all the keys back to zero
	for (unsigned int i = 0; i < animation_track_data.size(); ++i) {
		animation_track_data[i].last_key = 0;
		next_animation_track_data[i].last_key = 0;
	}

}

void AnimationController::Process() {
	//Check if we are switching between two animations
	if (animation_blending && animation_blending_enabled) {
		float noramlized_velo = animation_path->GetCurrentVelocity() / animation_path->constant_velocity;
		//Blend between the two if we are
		bool blend_complete =
			skeleton->ProcessBlendAnimationGraph(animation_time, bone_matrix_buffer,
				*active_animation, *next_animation,
				animation_track_data, next_animation_track_data, noramlized_velo);
		//Check if blending is complete
		if (blend_complete) {
			animation_blending = false;
			active_animation = next_animation;
			next_animation = nullptr;
			ClearTrackData();
		}

	}
	else {
		skeleton->ProcessAnimationGraph(animation_time, bone_matrix_buffer,
			*active_animation, animation_track_data);
	}
}

void AnimationController::ProcessBindPose() {
	skeleton->ProcessBindPose(bone_matrix_buffer);
}

void AnimationController::SetSkel(const FBXSkeleton& fbx_skele) {
	skeleton = std::make_unique<Skeleton>();
	skeleton->ConvertFromFbx(&fbx_skele);
	skeleton->Initialize();
	bone_matrix_buffer.resize(skeleton->hierarchy.size());
	animation_track_data.resize(skeleton->hierarchy.size());
	next_animation_track_data.resize(skeleton->hierarchy.size());
	ClearTrackData();
}

const Skeleton& AnimationController::GetSkel() const {
	return *skeleton.get();
}

Skeleton* AnimationController::GetSkelP() const {
	return skeleton.get();
}

void AnimationController::AddAnimation(const FBXAnimation& anim) {
	Animation* new_anim = new Animation();
	new_anim->ConvertFromFbx(&anim);
	animations.push_back(new_anim);
	active_animation = new_anim;
}

void AnimationController::SetAnimationPath(Path* path) {
	animation_path.reset(path);
	//Idle animation pace
	animations[1]->pace = animation_path->constant_velocity;

	//Walk animation pace
	animations[2]->pace = 0.5f * animation_path->constant_velocity;

	//Run animation pace
	animations[1]->pace = animation_path->constant_velocity;

	//Walk animation pace
	animations[2]->pace = 0.5f * animation_path->constant_velocity;

	animation_run_threshold = animation_path->constant_velocity * (2.0f / 3);
}

void AnimationController::SetActiveAnimation(unsigned int animation_index) {
	active_animation = animations[animation_index];
	animation_time = 0.0f;
	ClearTrackData();
}

void AnimationController::SwitchAnimation(unsigned int animation_index) {
	if (active_animation != animations[animation_index]) {
		next_animation = animations[animation_index];
		animation_blending = true;
	}
}

void AnimationController::ShowPathControls() {
	if (ImGui::Begin("Animation Path"))
	{
		float curr_time = animation_path->GetCurrentPathTime();
		float norm_distance = animation_path->GetSinDistanceFromTime(curr_time);
		float curr_distance = norm_distance * animation_path->GetDistance(1);
		dx::XMVECTOR curr_pos = animation_path->GetCurrentPosition();
		ImGui::Text("Position : X-%f Y-%f Z%f", 
			dx::XMVectorGetX(curr_pos), 
			dx::XMVectorGetY(curr_pos), 
			dx::XMVectorGetZ(curr_pos));
		ImGui::Text("Distance : %f", curr_distance);
		float curr_u = animation_path->GetU(curr_distance);
		ImGui::Text("Inverse Distance : %f", curr_u);
		float u_distance = animation_path->GetDistance(curr_u);
		ImGui::Text("Distance from u : %f", u_distance);
		float velocity = animation_path->GetVelocity(curr_time);
		ImGui::Text("Velocity : %f", velocity);
		ImGui::Text("curr_time : %f", curr_time);
		ImGui::Text("Normalized T : %f", animation_path->GetNormalizedT(curr_time));

		ImGui::SliderFloat("Path velocity", &animation_path->constant_velocity, 500, 1200.0f, "%.4f");
		if (ImGui::Button("Recalculate velo functions") ) {
			animation_path->GenerateDefaultVelocityFunction();
		}

		ImGui::SliderFloat("Look ahead time", &animation_path->constant_look_ahead_time, 0.0f, 12.0f, "%.4f");

		ImGui::SliderFloat("Loop Time", &animation_path->loop_time,
			0.0f, 60, "%.4f");
	}
	ImGui::End();
}

void AnimationController::ShowAnimationControls() {
	if (ImGui::Begin("Animation Controller"))
	{
		ImGui::Checkbox("Enable animation blending", &animation_blending_enabled);
		if (ImGui::BeginMenu("Animation Pace")) {
			for (unsigned int i = 0; i < animations.size(); i++) {
				std::string str = "Animation " + std::to_string(i);
				ImGui::SliderFloat(str.c_str(), &animations[i]->pace, 0, 1500.0f, "%.4f");
			}
			ImGui::EndMenu();
		}
		ImGui::SliderFloat("Run Velocity threshold", &animation_run_threshold, 
						   0.0f, animation_path->constant_velocity, "%.4f");
	}
	ImGui::End();
}

Animation::Animation() : duration(0.0f), pace(0.0f) {
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
	}

	//Remember the last keyframe
	data.last_key = curr_key;
}

const VQS& Animation::GetBaseTransform(int bone_index) {
	Track& curr_path = tracks[bone_index];
	return curr_path.key_frames[0].transform;
}

bool Animation::CalculateBlendTransform(float animTime, int trackIndex, 
										Animation& next_animation, VQS& animation_transform, 
										TrackData& data, TrackData& next_data, float normalized_velo) {
	int curr_key = data.last_key;
	int next_curr_key = next_data.last_key;

	//Track for current animation
	Track& curr_path = tracks[trackIndex];

	//Track for the next animation to blend into
	Track& next_anim_curr_path = next_animation.tracks[trackIndex];

	//Search Forward in the keyframes for the interval
	while (curr_key != curr_path.key_frames.size() - 1 &&
		curr_path.key_frames[curr_key + 1].time < animTime)
		++curr_key;

	//Search Backward in the keyframes for the interval
	while (curr_key != 0 && curr_path.key_frames[curr_key].time > animTime)
		--curr_key;

	//Search Forward in the keyframes for the interval
	while (next_curr_key != next_anim_curr_path.key_frames.size() - 1 &&
		next_anim_curr_path.key_frames[next_curr_key + 1].time < animTime)
		++next_curr_key;

	//Search Backward in the keyframes for the interval
	while (next_curr_key != 0 && next_anim_curr_path.key_frames[next_curr_key].time > animTime)
		--next_curr_key;

	//Flag to check if the blending is done between two animations
	bool blend_completed = false;
	if (next_curr_key == next_anim_curr_path.key_frames.size() - 1)
	{
		//Clamp animation to the last frame of the next animation
		animation_transform = next_anim_curr_path.key_frames[next_curr_key].transform;
		blend_completed = true;
	}
	else 
	{
		//Blend between two different animation key frames
		//Interpolate between the two frames
		
		//Initialize key_prev_0 in case curr_key goes beyond that path
		KeyFrame& key_prev_0 = curr_path.key_frames[curr_key];
		KeyFrame& key_prev_1 = curr_path.key_frames[curr_key + 1];
		
		KeyFrame& key_next_0 = next_anim_curr_path.key_frames[next_curr_key];
		KeyFrame& key_next_1 = next_anim_curr_path.key_frames[next_curr_key + 1];

		float t1 = key_next_0.time;
		float t2 = key_next_1.time;
		//Normalize t to the range 0..1
		float normalized_t = (animTime - t1) / (t2 - t1);

		VQS prev_transform = key_prev_0.transform.InterpolateTo(key_prev_1.transform, normalized_t);

		VQS next_transform = key_next_0.transform.InterpolateTo(key_next_1.transform, normalized_t);

		animation_transform = prev_transform.InterpolateTo(next_transform, normalized_velo);
	}

	//Remember the last keyframe
	data.last_key = curr_key;
	next_data.last_key = next_curr_key;
	return blend_completed;
}

void Animation::ConvertFromFbx(const FBXAnimation* fbx_animation) {
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
