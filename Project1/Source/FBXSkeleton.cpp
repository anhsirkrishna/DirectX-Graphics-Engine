#include "FBXMatConverter.h"
#include "FBXSkeleton.h"
#include <queue>
#include "FBXUtil.h"

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
			bones_nodes.push_back(pNode);
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
			//Bone does not have a bind pose. 
			int parent_index = bones[i]->ParentIndex;
			int parent_node_index = p_bind_pose->Find(bones[parent_index]->BoneNode);
			localMatrix = GetPoseMatrix(p_bind_pose, parent_node_index);

			//localMatrix.SetIdentity();
			//localMatrix = GetGlobalDefaultPosition(bones[i]->BoneNode);
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

int FBXSkeleton::GetBoneIndex(FbxNode* p_node) {
	for (int i = 0; i < bones.size(); ++i)
	{
		if (p_node == bones[i]->BoneNode)
			return i;
	}
	return -1;
}
