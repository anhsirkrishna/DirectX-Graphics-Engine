#include "FBXUtil.h"

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

FbxAMatrix GetPoseMatrix(FbxNode* pNode, FbxPose* pPose)
{
	int lNodeIndex = pPose->Find(pNode);
	return GetPoseMatrix(pPose, lNodeIndex);
}

// Recursive function to get a node's global default position 
// As a prerequisite, parent node's default local position must be already set 
FbxAMatrix GetGlobalDefaultPosition(FbxNode* node)
{
	// Stores the local position, global position, and parent's global position
	FbxAMatrix local_position;
	FbxAMatrix global_position;
	FbxAMatrix parent_global_position;

	// Get the translation, rotation, and scaling
	local_position.SetT(node->LclTranslation.Get());
	local_position.SetR(node->LclRotation.Get());
	local_position.SetS(node->LclScaling.Get());

	// If the node has a parent
	if (node->GetParent())
	{
		parent_global_position = GetGlobalDefaultPosition(node->GetParent());
		global_position = parent_global_position * local_position;
	}
	// Otherwise, we've reached the end of the recursion,
	// so the global position is the local position
	else
	{
		global_position = local_position;
	}

	// Return the global position
	return global_position;
}

// Get the geometry deformation local to a node. It is never inherited by the
// children.
FbxAMatrix GetGeometry(FbxNode* pNode) {
	FbxVector4 lT, lR, lS;
	FbxAMatrix lGeometry;

	lT = pNode->GetGeometricTranslation(FbxNode::EPivotSet::eSourcePivot);
	lR = pNode->GetGeometricRotation(FbxNode::EPivotSet::eSourcePivot);
	lS = pNode->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot);

	lGeometry.SetT(lT);
	lGeometry.SetR(lR);
	lGeometry.SetS(lS);

	return lGeometry;
}

//Does homogeneous transform
FbxVector4 Transform(const FbxAMatrix& pXMatrix, const FbxVector4& point)
{
	return pXMatrix.MultT(point);
}

bool IsBoneNode(FbxNode* pNode)
{
	if (pNode->GetNodeAttribute() != NULL)
	{
		FbxNodeAttribute::EType lAttributeType = pNode->GetNodeAttribute()->GetAttributeType();
		if (lAttributeType == FbxNodeAttribute::eSkeleton)
			return true;
	}
	return false;
}