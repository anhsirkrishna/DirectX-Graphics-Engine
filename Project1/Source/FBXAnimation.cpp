#include "FBXAnimation.h"
#include "FBXUtil.h"
#include <set>

void CollectKeyTimes(std::set<FbxTime>& keyTimes, FbxAnimCurve* anim_curve)
{
	if (anim_curve)
	{
		int keyCount = anim_curve->KeyGetCount();
		for (int i = 0; i < keyCount; ++i)
		{
			FbxAnimCurveKey curve_key = anim_curve->KeyGet(i);
			keyTimes.insert(curve_key.GetTime());
		}
		
	}
}

FbxAMatrix GetLocalMatrixFromTime(FbxNode* p_node,const FbxTime& current)
{
	//We should be extracting the local transform data directly from the node
	//buy this does not work due to other transforms like PreRotation
	//the simplest what to extract the data is to use GetGlobalFromCurrentTake
	//which always returns the correct matrix then use the parent's inverse to
	//get the local matrix (slower but effective)

	//This is what we want to do but it does not work in all cases
	//KFbxVector4 Rotate = pNode->GetLocalRFromCurrentTake( current );
	//KFbxVector4 Trans = pNode->GetLocalTFromCurrentTake( current );
	//KFbxVector4 Scale = pNode->GetLocalSFromCurrentTake( current );
	FbxAnimEvaluator* eval = p_node->GetScene()->GetAnimationEvaluator();

	FbxNode* p_parent = p_node->GetParent();
	FbxAMatrix local_transform;
	if (IsBoneNode(p_parent))
	{
		//compute the local transform by getting the global of this
		//node and the parent then multiplying the this nodes global
		//transform by the inverse of the parents global transform
		FbxAMatrix PX = eval->GetNodeGlobalTransform(p_parent, current);
		FbxAMatrix LX = eval->GetNodeGlobalTransform(p_node, current);
		PX = PX.Inverse();
		local_transform = PX * LX;
	}
	else
	{
		//We want root bones in global space
		local_transform = eval->GetNodeGlobalTransform(p_node, current);
	}

	return local_transform;
}

void FBXAnimation::ExtractAnimationTrack(
	FbxNode* p_node, int track_index, FbxAnimStack* animation_stack, FbxTime start, FbxTime stop, 
	FBXMatConverter& converter) {
	printf("Reading KeyFrames For Node '%s'\n", p_node->GetName());

	FBXTrack& track = tracks[track_index];

	FbxAnimLayer* animation_layer = animation_stack->GetMember<FbxAnimLayer>(0);

	//Collect all the times that a keyframe occurs. This implementation combines
	//all the curves (position,rotation,scale) together into one track for each node

	std::set< FbxTime > keyTimes;
	keyTimes.insert(start);//Make sure there at least one keyframe

	CollectKeyTimes(keyTimes, p_node->LclRotation.GetCurve(animation_layer, FBXSDK_CURVENODE_COMPONENT_X));
	CollectKeyTimes(keyTimes, p_node->LclRotation.GetCurve(animation_layer, FBXSDK_CURVENODE_COMPONENT_Y));
	CollectKeyTimes(keyTimes, p_node->LclRotation.GetCurve(animation_layer, FBXSDK_CURVENODE_COMPONENT_Z));

	CollectKeyTimes(keyTimes, p_node->LclTranslation.GetCurve(animation_layer, FBXSDK_CURVENODE_COMPONENT_X));
	CollectKeyTimes(keyTimes, p_node->LclTranslation.GetCurve(animation_layer, FBXSDK_CURVENODE_COMPONENT_Y));
	CollectKeyTimes(keyTimes, p_node->LclTranslation.GetCurve(animation_layer, FBXSDK_CURVENODE_COMPONENT_Z));

	//Make space for all the keyframes
	track.key_frames.resize(keyTimes.size());

	printf("Number of Keyframes:%d\n", keyTimes.size());

	//Iterate through the keyframe set storing the key frame data for each time
	int keyIndex = 0;
	std::set<FbxTime>::iterator it = keyTimes.begin();
	for (auto& current : keyTimes)
	{
		FbxAMatrix localXForm = GetLocalMatrixFromTime(p_node, current);

		//Convert the matrix into the destination coordinate space
		converter.ConvertMatrix(localXForm);

		FbxVector4 scale = localXForm.GetS();

		if (!CheckScaling(scale))
		{
			printf("Non-uniform scaling on node '%s' on layer '%s'.\n", p_node->GetName(), animation_layer->GetName());
			localXForm.SetS(FbxVector4(1, 1, 1));
		}

		track.key_frames[keyIndex].time = current;
		track.key_frames[keyIndex].translation = localXForm.GetT();//Store translation
		track.key_frames[keyIndex].rotation = localXForm.GetQ();//Store rotation
		keyIndex++;
	}

}

void FBXAnimation::ExtractAnimationTracksFromStack(
	FbxScene* p_scene, std::vector<FbxNode*> animation_nodes, 
	FbxAnimStack* animation_stack, FBXMatConverter& converter) {
	int animation_layer_count = animation_stack->GetMemberCount<FbxAnimLayer>();
	FbxAnimLayer* animation_layer = animation_stack->GetMember<FbxAnimLayer>(0);

	FbxString anim_name = animation_layer->GetName();
	printf("Extracting Animation for Layer '%s'\n", anim_name.Buffer());

	p_scene->SetCurrentAnimationStack(animation_stack);
	FbxTime gStart, gStop;

	FbxTimeSpan animation_span = animation_stack->GetLocalTimeSpan();
	duration = animation_span.GetDuration();
	gStart = animation_span.GetStart();
	gStop = animation_span.GetStop();
	
	tracks.resize(animation_nodes.size());
	for (int i = 0; i < animation_nodes.size(); ++i)
		ExtractAnimationTrack(animation_nodes[i], i, animation_stack, gStart, gStop, converter);

	printf("Extracted Animation '%s' Length %.2f seconds. Number of Tracks %d\n", anim_name.Buffer(), duration.GetSecondCount(), tracks.size());
}
