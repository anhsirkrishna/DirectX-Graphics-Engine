#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Line.h"
#include "VQS.h"
#include "FBXSkeleton.h"
#include "FBXAnimation.h"

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
	void CalculateTransform(float animTime, int trackIndex, VQS& animation_transform, TrackData& data);
	void ConvertFromFbx(FBXAnimation* fbx_animation);

	float duration;
	std::vector<Track> tracks;

};

class Skeleton {
public:
	~Skeleton();
	void ConvertFromFbx(FBXSkeleton* _skele);
	void Initialize();
	void ProcessAnimationGraph(float time, std::vector<dx::XMMATRIX>& matrix_buffer,
		Animation& anim, std::vector<TrackData>& track_buffer);
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
	Skeleton* skeleton;
	Animation* active_animation;
	std::vector<TrackData> animation_track_data;
	std::vector<dx::XMMATRIX> bone_matrix_buffer;
	std::vector<Animation*> animations;

	void ClearTrackData();
	void Process();
	void ProcessBindPose();
	void SetSkel(Skeleton* skel);
	void AddAnimation(Animation* anim);
};