#pragma once
#include <fbxsdk.h>
#include "FBXSkeleton.h"
#include "FBXAnimation.h"
class FBXMesh;

class FBXLoader {
public:
	FBXLoader();
	~FBXLoader();

	void CollectMeshes(FbxNode* pRootNode);
	bool ExtractSceneData();

	FbxManager* fbx_sdk_manager;

	FbxScene* p_Scene;
	std::vector<FBXMesh*> meshes;
	FbxPose* bind_pose;
	FBXSkeleton skele;
	std::vector<FBXAnimation*> animations;
};