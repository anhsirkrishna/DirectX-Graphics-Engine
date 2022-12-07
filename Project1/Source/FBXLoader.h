#pragma once
#include <fbxsdk.h>
#include "FBXSkeleton.h"
#include "FBXAnimation.h"
class FBXMesh;

/*
* CREDITS:
* "Human mesh animated"
* https://www.turbosquid.com/3d-models/free-human-base-mesh-animations-3d-model/1049650
*/

class FBXLoader {
public:
	
	FBXLoader(const char* filename = "BaseMesh_Anim.fbx");
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