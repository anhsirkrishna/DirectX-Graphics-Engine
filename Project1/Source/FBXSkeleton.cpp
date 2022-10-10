#include "FBXMatConverter.h"
#include "FBXSkeleton.h"
#include <queue>

inline bool IsEqualEpsilon(float A, float B)
{
	return fabs(A - B) <= 1e-5f;
}

bool CheckPositive(FbxVector4 scale)
{
	if (scale[0] < 0.0f ||
		scale[1] < 0.0f ||
		scale[2] < 0.0f)
	{
		return false;
	}

	return true;
}

bool CheckScaling(FbxVector4 scale)
{
	if (!IsEqualEpsilon(scale[0], scale[1]) ||
		!IsEqualEpsilon(scale[1], scale[2]) ||
		!IsEqualEpsilon(scale[0], scale[2]))
	{
		return false;
	}

	return true;
}

//FBX SDK has two different types of matrices XMatrix and Matrix
//XMatrix is affine and has functions for extracting pos,trans and rot
//Matrix has all the matrix operations and is returned by several SDK functions
//because of this the matrices need to be converted between each other
void SetAMatrix(FbxAMatrix& pXMatrix, const FbxMatrix& pMatrix)
{
	memcpy((double*)pXMatrix.mData, &pMatrix.mData, sizeof(pMatrix.mData));
}

FbxNodeAttribute::EType GetNodeAttributeType(FbxNode* pNode)
{
	FbxNodeAttribute* nodeAtt = pNode->GetNodeAttribute();
	if (nodeAtt != NULL)
		return nodeAtt->GetAttributeType();
	return FbxNodeAttribute::eNull;
}

// Get the matrix of the node in the given pose
FbxAMatrix GetPoseMatrix(FbxPose* pPose, int pNodeIndex)
{
	FbxAMatrix lPoseMatrix;
	FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);
	SetAMatrix(lPoseMatrix, lMatrix);
	return lPoseMatrix;
}

// Get the matrix of the node in the given pose
FbxAMatrix GetPoseMatrixInverse(FbxPose* pPose, int pNodeIndex)
{
	FbxAMatrix lPoseMatrix;
	FbxMatrix lMatrix = pPose->GetMatrix(pNodeIndex);
	lMatrix = lMatrix.Inverse();
	SetAMatrix(lPoseMatrix, lMatrix);
	return lPoseMatrix;
}

struct NodeAndParent
{
	NodeAndParent(FbxNode* n, int i)
		: node(n), ParentId(i) {}
	FbxNode* node;
	int ParentId;
};

FBXSkeleton::FBXSkeleton() {

}

FBXSkeleton::~FBXSkeleton() {

}

void FBXSkeleton::ExtractSkeletonFromScene(FbxNode* p_root_node) {
	std::queue<NodeAndParent> nodeStack;
	nodeStack.push(NodeAndParent(p_root_node, -1));

	while (nodeStack.size() != 0)
	{
		NodeAndParent curNode = nodeStack.front();
		nodeStack.pop();

		FbxNode* pNode = curNode.node;
		int parentIndex = curNode.ParentId;
		int nodeIndex = -1;

		if (GetNodeAttributeType(pNode) == FbxNodeAttribute::eSkeleton)
		{
			FBXBone* bone = new FBXBone;
			nodeIndex = bones.size();
			bone->ParentIndex = parentIndex;
			bone->BoneIndex = nodeIndex;
			bone->Name = pNode->GetName();
			bone->BoneNode = pNode;

			bones.push_back(bone);
		}

		for (int i = 0; i < pNode->GetChildCount(); i++)
		{
			nodeStack.push(NodeAndParent(pNode->GetChild(i), nodeIndex));
		}
	}
}

void FBXSkeleton::ExtractBindPose(FBXMatConverter& converter) {
	for (int i = 0; i < bones.size(); ++i) {
		int lNodeIndex = p_bind_pose->Find(bones[i]->BoneNode);
		FbxAMatrix localMatrix;
		if (lNodeIndex == -1)
		{
			//Bone does not have a bind pose export identity
			localMatrix.SetIdentity();
		}
		else
		{
			localMatrix = GetPoseMatrix(p_bind_pose, lNodeIndex);
		}

		converter.ConvertMatrix(localMatrix);

		FbxVector4 scale = localMatrix.GetS();
		if (!CheckScaling(scale) || !CheckPositive(scale))
		{
			//If there is negative scaling the matrix inverse might have problems
			localMatrix.SetS(FbxVector4(1, 1, 1));
		}

		FbxAMatrix Inverse = localMatrix.Inverse();

		//TODO: You could also store scaling if you want to have scaling on your mesh but
		//it should always be uniform scaling

		bones[i]->BindPos = localMatrix.GetT();
		bones[i]->BindRot = localMatrix.GetQ();

		bones[i]->BoneSpacePos = Inverse.GetT();
		bones[i]->BoneSpaceRot = Inverse.GetQ();
	}
}
