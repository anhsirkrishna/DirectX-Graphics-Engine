#pragma once
#include <vector>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Line.h"

namespace dx = DirectX;

class vqs {};

class Bone : public Line {
public:
	~Bone() = default;
	Bone(Graphics& _gfx, int _indx, int _parent_indx, dx::XMFLOAT3 _translation, dx::XMVECTORF32 _rotation,
		dx::XMFLOAT3 _inv_translation, dx::XMVECTORF32 _inv_rotation);

	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void Update(float dt) noexcept override;

	int bone_indx;
	int parent_indx;
	dx::XMFLOAT3 translation;
	dx::XMVECTORF32 rotation;


	dx::XMFLOAT3 inv_translation;
	dx::XMVECTORF32 inv_rotation;

	std::vector<Bone*> children;
};

class Skeleton {
public:
	~Skeleton();
	void ConvertFromFbx(Graphics& gfx, FBXSkeleton* _skele);
	void Initialize();
	void Draw(Graphics& gfx);
	std::vector<Bone*> hierarchy;
};

class KeyFrame {
	float elapsed_time;
	vqs transform;
};

struct Tracks {
	std::vector<KeyFrame> key_frames;
};

class Animation {
	float duration;
	std::vector<Tracks> tracks;
};

class AnimatedModel {
	Skeleton skeleton;
	std::vector<Animation> animations;
};
