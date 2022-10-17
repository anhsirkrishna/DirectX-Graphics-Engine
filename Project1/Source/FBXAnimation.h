#pragma once
#include <fbxsdk.h>
#include <vector>
#include "FBXMatConverter.h"

struct FBXKeyFrame
{
	FbxTime time;
	FbxVector4 translation;
	FbxQuaternion rotation;
};

struct Track
{
	std::vector< FBXKeyFrame > key_frames;
};

class FBXAnimation
{
public:
	std::string name;
	FbxTime duration;
	std::vector<Track> tracks;

	void ExtractAnimationTrack(FbxNode* p_node, int track_index, FbxAnimStack* animation_stack, FbxTime start, FbxTime stop, FBXMatConverter& converter);
	void ExtractAnimationTracksFromStack(FbxScene* p_scene, std::vector<FbxNode*> animation_nodes, FbxAnimStack* animation_stack, FBXMatConverter& converter);
};

