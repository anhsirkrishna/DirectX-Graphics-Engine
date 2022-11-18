#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Line.h"
#include "VQS.h"
#include "FBXSkeleton.h"
#include "FBXAnimation.h"
#include "Path.h"

class Bone {
public:
	~Bone() = default;
	Bone(int _indx, int _parent_indx, const VQS& _bind_transform, const VQS& _inv_bind_transform);

	int bone_indx;
	int parent_indx;

	VQS bind_transform;

	VQS inv_bind_transform;

	std::vector<Bone*> children;
};

class KeyFrame {
public:
	double time;
	VQS transform;
};

//A Track is a set of keyframes that are in temporal
//order from 0 to the animation duration
struct Track {
	std::vector<KeyFrame> key_frames;
};

//Track data is used to help process the animation in this
//case it is the last keyframe the track was on
struct TrackData
{
	unsigned int last_key;
};

class Animation {
public:
	Animation();
	~Animation();
	
	//Calculate the transform for the skeleton for a given animation_time and bone_index
	void CalculateTransform(float animTime, int trackIndex, VQS& animation_transform, TrackData& data);
	
	/*
	* Calculate the transform by blending between two different animations
	* Returns: bool - True if the blending has finished
	*/
	bool CalculateBlendTransform(float animTime, int trackIndex, Animation& next_animation,
								 VQS& animation_transform, TrackData& data, TrackData& next_data,
								 float normalized_velo);
	
	void ConvertFromFbx(FBXAnimation* fbx_animation);

	float duration;
	std::vector<Track> tracks;
	float pace;
};

class Skeleton {
public:
	~Skeleton();
	void ConvertFromFbx(FBXSkeleton* _skele);
	void Initialize();
	void ProcessAnimationGraph(float time, std::vector<dx::XMMATRIX>& matrix_buffer,
		Animation& anim, std::vector<TrackData>& track_buffer);
	bool ProcessBlendAnimationGraph(float time, std::vector<dx::XMMATRIX>& matrix_buffer,
			Animation& anim, Animation& anim_next, 
			std::vector<TrackData>& track_buffer, std::vector<TrackData>& next_track_buffer,
			float normalized_velo);
	void ProcessBindPose(std::vector<dx::XMMATRIX>& buffer);

	std::vector<Bone*> hierarchy;
};

//Controls the animation for a animated model by tracking time and
//using an animation and skeleton to generate a matrix buffer.
class AnimationController
{
public:
	AnimationController();
	~AnimationController();
	void Update(float dt);

	float animation_time;
	float animation_speed;

	//Velocity threshold to make the animation switch to running.
	float animation_run_threshold;

	//Check if we are blending between two animations currently
	bool animation_blending;
	
	//Flag to perform animation blending or not
	bool animation_blending_enabled = true;

	std::unique_ptr<Skeleton> skeleton;
	Animation* active_animation;
	Animation* next_animation;
	std::unique_ptr<Path> animation_path;
	std::vector<TrackData> animation_track_data;
	std::vector<TrackData> next_animation_track_data;
	std::vector<dx::XMMATRIX> bone_matrix_buffer;
	std::vector<Animation*> animations;

	void ClearTrackData();
	void Process();
	void ProcessBindPose();
	void SetSkel(Skeleton* skel);
	void AddAnimation(Animation* anim);
	void SetAnimationPath(Path* path);
	void SetActiveAnimation(unsigned int animation_index);
	void SwitchAnimation(unsigned int animation_index);

	void ShowPathControls();
	void ShowAnimationControls();
};