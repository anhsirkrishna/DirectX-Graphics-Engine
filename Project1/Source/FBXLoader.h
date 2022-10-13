#pragma once
#include <fbxsdk.h>

class FBXLoader {
public:
	FBXLoader();
	~FBXLoader();
	FbxManager* fbx_sdk_manager;
	FbxScene* p_Scene;
};