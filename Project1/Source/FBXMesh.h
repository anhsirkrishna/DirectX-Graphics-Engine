#pragma once
#include "FBXLoader.h"
#include "FBXMatConverter.h"
#include <algorithm>

enum VertexType
{
	VertexTypeDefault,
	VertexTypeSkin
};

struct Vertex
{
	FbxVector4 point;
	FbxVector4 normal;
	FbxVector2 texture;
};

struct IndexedVertex
{
	int posIndex;
	int normalIndex;
	int uvIndex;
};

class FBXMeshVertices
{
public:
	void ConvertTriWinding();
	void Triangulate();
	int AddNewVertex(IndexedVertex v);
	bool IsMatchingVertex(Vertex& vertex, IndexedVertex& cur, IndexedVertex& v);
	int FindMatchingVertex(IndexedVertex& v);
	void GenerateVertices();
	void ExtractGeometry(FbxNode* p_node, FbxPose* p_pose, FBXMatConverter& converter);

	//Input Data
	std::vector<int> position_indices;
	std::vector<int> uv_indices;
	std::vector<int> normal_indices;
	std::vector<FbxVector4> points;
	std::vector<FbxVector4> normals;
	std::vector<FbxVector2> uvs;
	std::vector<int> polygon_size_array;

	//Intermediate
	std::vector< std::vector<int> > control_map;
	std::vector< IndexedVertex > source_indices;

	//Resulting Data
	std::vector<Vertex> processed_vertices;
	std::vector<int> processed_indices;
};

//Skinning Data
struct JointWeight
{
	float weight;
	unsigned int index;
};
typedef std::vector< JointWeight > WeightVector;

class SkinData {
public:
	//Index in PointWeight vector correspond to mesh control points which
	//are mapped by the Position Indices
	std::vector< WeightVector > point_weights;

	bool ExtractSkinWeights(FbxPose* pPose, FbxNode* pNode, FBXSkeleton& skel, FBXMatConverter& converter);

};

class FBXMesh {
public:
	FBXMesh(FbxNode* _mesh_node);
	FbxNode* p_mesh_node;
	FBXMeshVertices mesh;
	SkinData skin;
	VertexType mesh_vertex_type;
	void CombineInto(FBXMesh& mesh);
};

