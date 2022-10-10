#pragma once
#include <vector>

class vqs {};

struct Skeleton {
	std::vector<Bone> hierarchy;
};

class Bone {
	int parent_indx;
	vqs bind_transform;
	vqs inverse_bind_transform;
};

class Animation {
	float duration;
	std::vector<Tracks> tracks;
};

class AnimatedModel {
	Skeleton skeleton;
	std::vector<Animation> animations;
};


struct Tracks {
	std::vector<KeyFrame> key_frames;
};

class KeyFrame {
	float elapsed_time;
	vqs transform;
};