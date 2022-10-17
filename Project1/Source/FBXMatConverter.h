#pragma once
#include <fbxsdk.h>
//Class to convert matrices from FBX format to DirectX supported format

class FBXMatConverter
{
public:
	FBXMatConverter(FbxScene* pScene);
	void ConvertMatrix(FbxAMatrix& sourceMatX);
	void ConvertMeshMatrix(FbxAMatrix& sourceMatX);
	FbxAMatrix ConversionMatrix;
	bool NeedToChangedWinding;
};

