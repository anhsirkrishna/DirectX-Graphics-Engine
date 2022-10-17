#include <vector>
#include "FBXUtil.h"
#include "FBXMatConverter.h"
#include "FBXMesh.h"
#include "FBXLoader.h"
#include "FBXAnimation.h"

/* Tab character ("\t") counter */
int numTabs = 0;

/**
 * Print the required number of tabs.
 */
void PrintTabs() {
	for (int i = 0; i < numTabs; i++)
		printf("\t");
}

/**
	 * Return a string-based representation based on the attribute type.
	 */
FbxString GetAttributeTypeName(FbxNodeAttribute::EType type) {
	switch (type) {
	case FbxNodeAttribute::eUnknown: return "unidentified";
	case FbxNodeAttribute::eNull: return "null";
	case FbxNodeAttribute::eMarker: return "marker";
	case FbxNodeAttribute::eSkeleton: return "skeleton";
	case FbxNodeAttribute::eMesh: return "mesh";
	case FbxNodeAttribute::eNurbs: return "nurbs";
	case FbxNodeAttribute::ePatch: return "patch";
	case FbxNodeAttribute::eCamera: return "camera";
	case FbxNodeAttribute::eCameraStereo: return "stereo";
	case FbxNodeAttribute::eCameraSwitcher: return "camera switcher";
	case FbxNodeAttribute::eLight: return "light";
	case FbxNodeAttribute::eOpticalReference: return "optical reference";
	case FbxNodeAttribute::eOpticalMarker: return "marker";
	case FbxNodeAttribute::eNurbsCurve: return "nurbs curve";
	case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
	case FbxNodeAttribute::eBoundary: return "boundary";
	case FbxNodeAttribute::eNurbsSurface: return "nurbs surface";
	case FbxNodeAttribute::eShape: return "shape";
	case FbxNodeAttribute::eLODGroup: return "lodgroup";
	case FbxNodeAttribute::eSubDiv: return "subdiv";
	default: return "unknown";
	}
}


/**
  * Print an attribute.
  */
void PrintAttribute(FbxNodeAttribute* pAttribute) {
	if (!pAttribute) return;

	FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
	FbxString attrName = pAttribute->GetName();
	PrintTabs();
	// Note: to retrieve the character array of a FbxString, use its Buffer() method.
	printf("<attribute type='%s' name='%s'/>\n", typeName.Buffer(), attrName.Buffer());
}

/**
* Print a node, its attributes, and all its children recursively.
*/
void PrintNode(FbxNode* pNode) {
	PrintTabs();
	const char* nodeName = pNode->GetName();
	FbxDouble3 translation = pNode->LclTranslation.Get();
	FbxDouble3 rotation = pNode->LclRotation.Get();
	FbxDouble3 scaling = pNode->LclScaling.Get();

	// Print the contents of the node.
	printf("<node name='%s' translation='(%f, %f, %f)' rotation='(%f, %f, %f)' scaling='(%f, %f, %f)'>\n",
		nodeName,
		translation[0], translation[1], translation[2],
		rotation[0], rotation[1], rotation[2],
		scaling[0], scaling[1], scaling[2]
	);
	numTabs++;

	// Print the node's attributes.
	for (int i = 0; i < pNode->GetNodeAttributeCount(); i++)
		PrintAttribute(pNode->GetNodeAttributeByIndex(i));

	// Recursively print the children.
	for (int j = 0; j < pNode->GetChildCount(); j++)
		PrintNode(pNode->GetChild(j));

	numTabs--;
	PrintTabs();
	printf("</node>\n");
}

void PrintRootNode(FbxNode* pNode) {
	if (pNode) {
		for (int i = 0; i < pNode->GetChildCount(); i++)
			PrintNode(pNode->GetChild(i));
	}
}

FBXLoader::FBXLoader() : bind_pose(nullptr) {
	/*
	* "Facial & Body Animated Toon Goon - ActorCore" (https://skfb.ly/oyvBs) by Reallusion is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
	*/
	
	/*
	* "Dwarf Warrior" (https://skfb.ly/6VZNo) by Northcliffe is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
	*/

	const char* filename = "Tad.fbx";

	// Initialize the SDK manager. This object handles memory management.
	fbx_sdk_manager = FbxManager::Create();

	// Create the IO settings object.
	FbxIOSettings* ios = FbxIOSettings::Create(fbx_sdk_manager, IOSROOT);
	fbx_sdk_manager->SetIOSettings(ios);

	// Create an importer using the SDK manager.
	FbxImporter* lImporter = FbxImporter::Create(fbx_sdk_manager, "");

	// Use the first argument as the filename for the importer.
	if (!lImporter->Initialize(filename, -1, fbx_sdk_manager->GetIOSettings())) {
		printf("Call to FbxImporter::Initialize() failed.\n");
		printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
		exit(-1);
	}

	// Create a new scene so that it can be populated by the imported file.
	p_Scene = FbxScene::Create(fbx_sdk_manager, "myScene");

	// Import the contents of the file into the scene.
	lImporter->Import(p_Scene);

	// The file is imported, so get rid of the importer.
	lImporter->Destroy();
	
	bind_pose = p_Scene->GetPose(0);
	skele.p_bind_pose = bind_pose;
}

FBXLoader::~FBXLoader() {
	for (auto& mesh : meshes)
		delete mesh;

	for (auto& animation : animations)
		delete animation;

	// Destroy the SDK manager and all the other objects it was handling.
	fbx_sdk_manager->Destroy();
}

void FBXLoader::CollectMeshes(FbxNode* pRootNode)
{
	for (int i = 0; i < pRootNode->GetChildCount(); i++)
	{
		FbxNode* pNode = pRootNode->GetChild(i);

		//If the node invisible it is usually a construction or dummy mesh
		//either way ignore it
		if (pNode->GetVisibility())
		{
			//The node must be a mesh
			if (GetNodeAttributeType(pNode) == FbxNodeAttribute::eMesh)
			{
				FbxMesh* pMesh = pNode->GetMesh();
				meshes.push_back(new FBXMesh(pRootNode->GetChild(i)));
			}

			if (GetNodeAttributeType(pNode) == FbxNodeAttribute::eNull)
			{
				CollectMeshes(pNode);
			}
		}
	}
}

bool FBXLoader::ExtractSceneData()
{
	//Determine conversion
	FBXMatConverter converter(p_Scene);

	FbxNode* root_node = p_Scene->GetRootNode();

	int pose_count = p_Scene->GetPoseCount();//Need a Bind Pose

	printf("Exporting As Skinned Model\n");
	printf("Using Bind Pose: %s\n", bind_pose->GetName());

	CollectMeshes(p_Scene->GetRootNode());

	if (meshes.size() == 0)
	{
		printf("No valid meshes found in scene");
		return false;
	}

	printf("Exporting Geometry...\n");
	for (auto p_mesh : meshes)
		p_mesh->mesh.ExtractGeometry(p_mesh->p_mesh_node, bind_pose, converter);

	skele.ExtractSkeletonFromScene(root_node);

	for (auto p_mesh : meshes) {
		if (p_mesh->skin.ExtractSkinWeights(bind_pose, p_mesh->p_mesh_node, skele, converter))
			p_mesh->mesh_vertex_type = VertexTypeSkin;
	}

	skele.ExtractBindPose(converter);

	//Extracting animations
	int animation_stack_count = p_Scene->GetSrcObjectCount<FbxAnimStack>();
	
	printf("Extracting animations \n");
	for (int i = 0; i < animation_stack_count; i++)
	{
		FbxAnimStack* anim_stack = p_Scene->GetSrcObject<FbxAnimStack>(i);
		FbxString stack_name = anim_stack->GetName();
		if (stack_name.Compare(FBXSDK_TAKENODE_DEFAULT_NAME) == 0)
			continue;

		FBXAnimation* animation = new FBXAnimation();
		animation->ExtractAnimationTracksFromStack(p_Scene, skele.bones_nodes, anim_stack, converter);
		animations.push_back(animation);
	}

	return true;
}