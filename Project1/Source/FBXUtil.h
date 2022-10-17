#pragma once
#include <fbxsdk.h>
#include <vector>

bool IsEqualEpsilon(float A, float B);

bool CheckPositive(FbxVector4 scale);

bool CheckScaling(FbxVector4 scale);

void SetAMatrix(FbxAMatrix& pXMatrix, const FbxMatrix& pMatrix);

FbxNodeAttribute::EType GetNodeAttributeType(FbxNode* pNode);

FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex);

FbxAMatrix GetPoseMatrixInverse(FbxPose* pPose, int pNodeIndex);

FbxAMatrix GetPoseMatrix(FbxNode* pNode, FbxPose* pPose);

FbxAMatrix GetGlobalDefaultPosition(FbxNode* node);

FbxAMatrix GetGeometry(FbxNode* pNode);

FbxVector4 Transform(const FbxAMatrix& pXMatrix, const FbxVector4& point);

bool IsBoneNode(FbxNode* pNode);

template< typename type >
void ConvetToStl(std::vector<type>& container, FbxLayerElementArrayTemplate<type>* FbxContainter)
{
	int numberofObjects = FbxContainter->GetCount();
	void* pData = FbxContainter->GetLocked();

	container.resize(numberofObjects);
	memcpy(&container[0], pData, sizeof(type) * numberofObjects);

	FbxContainter->Release(&pData);
}

template< typename type >
void FillStl(std::vector<type>& container, std::size_t size)
{
	container.resize(size);
	for (std::size_t i = 0; i < size; ++i)
		container[i] = i;
}