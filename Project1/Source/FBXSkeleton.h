#pragma once
#include <fbxsdk.h>
#include <vector>
#include <string>
#include "FBXMatConverter.h"

struct FBXBone
{
	std::string Name;
	int ParentIndex;
	int BoneIndex;
	FbxNode* BoneNode;

	FbxVector4 BindPos;
	FbxQuaternion BindRot;

	FbxVector4 BoneSpacePos;
	FbxQuaternion BoneSpaceRot;
};

class FBXSkeleton
{
public:
	FBXSkeleton();
	~FBXSkeleton();
	void ExtractSkeletonFromScene(FbxNode* p_root_node);
	void ExtractBindPose(FBXMatConverter& converter);
	int GetBoneIndex(FbxNode* p_node);
	FbxPose* p_bind_pose;
	std::vector<FBXBone*> bones;
	std::vector<FbxNode*> bones_nodes;
};

