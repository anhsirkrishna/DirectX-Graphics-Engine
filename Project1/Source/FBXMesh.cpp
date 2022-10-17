#include "FBXMesh.h"
#include "FBXUtil.h"

FBXMesh::FBXMesh(FbxNode* _mesh_node) : p_mesh_node(_mesh_node) {
}

void FBXMeshVertices::ConvertTriWinding() {
	for (unsigned int i = 0; i < processed_indices.size(); i += 3)
	{
		std::swap(processed_indices[i], processed_indices[i + 2]);
	}
}

void FBXMeshVertices::Triangulate() {
	std::vector<int> NewIndices;
	int c = 0;
	for (int i = 0; i < polygon_size_array.size(); ++i)
	{
		int size = polygon_size_array[i];
		//Simple Convex Polygon Triangulation: always n-2 tris
		const int NumberOfTris = size - 2;

		for (int p = 0; p < NumberOfTris; ++p)
		{
			NewIndices.push_back(processed_indices[c + 0]);
			NewIndices.push_back(processed_indices[c + 1 + p]);
			NewIndices.push_back(processed_indices[c + 2 + p]);
		}
		c += size;
	}

	//Swap the new triangulated indices with the old vertices
	processed_indices.swap(NewIndices);
}

int FBXMeshVertices::AddNewVertex(IndexedVertex v) {
	Vertex nv;
	nv.normal = normals[v.normalIndex];
	nv.point = points[v.posIndex];
	nv.texture = uvs[v.uvIndex];
	processed_vertices.push_back(nv);
	source_indices.push_back(v);
	return processed_vertices.size() - 1;
}

bool FBXMeshVertices::IsMatchingVertex(Vertex& vertex, IndexedVertex& cur, IndexedVertex& v) {
	//Need to check to see if the normals and uvs match if not generate a new vertex
	if (v.normalIndex != cur.normalIndex)
		return false;

	if (v.uvIndex != cur.uvIndex)
		return false;

	return true;
}

int FBXMeshVertices::FindMatchingVertex(IndexedVertex& v) {
	std::vector<int>& mappedVertices = control_map[v.posIndex];
	for (int i = 0; i < mappedVertices.size(); ++i)
	{
		if (IsMatchingVertex(processed_vertices[mappedVertices[i]], source_indices[mappedVertices[i]], v))
			return mappedVertices[i];
	}

	//No matching vertices mapped for this index, just add one
	int newIndex = AddNewVertex(v);
	mappedVertices.push_back(newIndex);
	return newIndex;
}

void FBXMeshVertices::GenerateVertices() {
	if (uv_indices.empty())
	{
		//Fill UvIndices with zeros
		uv_indices.resize(position_indices.size());
		//Put in a dummy UV
		uvs.push_back(FbxVector2(0, 0));
	}

	//Create a vertex and an index buffer
	control_map.resize(position_indices.size());
	for (int i = 0; i < position_indices.size(); ++i)
	{
		IndexedVertex v = { position_indices[i] , normal_indices[i] , uv_indices[i] };
		int index = FindMatchingVertex(v);
		processed_indices.push_back(index);
	}

	Triangulate();
	ConvertTriWinding();
}

void FBXMeshVertices::ExtractGeometry(FbxNode* p_node, FbxPose* p_pose, FBXMatConverter& converter) {
	std::string nodeName = p_node->GetName();

	if (p_node->GetNodeAttribute() != NULL)
	{
		FbxNodeAttribute::EType attribute_type = p_node->GetNodeAttribute()->GetAttributeType();

		if (attribute_type == FbxNodeAttribute::eMesh)
		{
			printf("Extracting Mesh '%s'\n", nodeName.c_str());

			//When want to move the mesh into model space and in the bind pose if one is provided
			FbxAMatrix  meshTransform;
			if (p_pose)
			{
				//For skinned meshes get the pose matrix
				meshTransform = GetPoseMatrix(p_node, p_pose);
			}
			else
			{
				//TODO: Adjust this code. What do you want it to do in the default case?

				//This will cause the mesh transform to be collapsed into the vertices.
				//Use this if you want exactly what is displayed in the model file 
				meshTransform = GetGlobalDefaultPosition(p_node);

				//This will make the mesh be exported at its local original ignore
				//its transformation in the scene.
				//meshTransform.SetIdentity();
			}

			//Meshes have a separate geometry transform that also needs to be applied
			FbxAMatrix geoTransform = GetGeometry(p_node);
			meshTransform = meshTransform * geoTransform;

			//Convert the matrix into the destination coordinate space
			converter.ConvertMeshMatrix(meshTransform);

			FbxMesh* mesh = p_node->GetMesh();
			int polyCount = mesh->GetPolygonCount();

			//Get the indices			
			int polyVertexCount = mesh->GetPolygonVertexCount();
			int* indices = mesh->GetPolygonVertices();

			position_indices.resize(polyVertexCount);
			memcpy(&position_indices[0], indices, sizeof(int) * polyVertexCount);

			for (int i = 0; i < polyCount; ++i)
				polygon_size_array.push_back(mesh->GetPolygonSize(i));

			//Get Points
			FbxVector4* v_points = mesh->GetControlPoints();
			int controlPointsCount = mesh->GetControlPointsCount();
			points.resize(controlPointsCount);
			for (int cp = 0; cp < controlPointsCount; ++cp)
				points[cp] = Transform(meshTransform, v_points[cp]);

			FbxLayer* layer = mesh->GetLayer(0);

			//Get the normals from the mesh
			if (FbxLayerElementNormal* normLayer = layer->GetNormals())
			{
				ConvetToStl(normals, &normLayer->GetDirectArray());
				//Remove translation from the matrix for the normals
				meshTransform.SetT(FbxVector4(0, 0, 0, 0));
				for (int n = 0; n < normals.size(); ++n)
				{
					normals[n] = Transform(meshTransform, normals[n]);
					normals[n].Normalize();
				}

				if (normLayer->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					ConvetToStl(normal_indices, &normLayer->GetIndexArray());
				else
				{
					FbxLayerElement::EMappingMode Mapping = normLayer->GetMappingMode();

					//Normals map directly to control points
					if (Mapping == FbxLayerElement::eByControlPoint)
						normal_indices = position_indices;

					if (Mapping == FbxLayerElement::eByPolygonVertex)
						FillStl(normal_indices, polyVertexCount);

				}
			}

			//If there is UV process them
			if (FbxLayerElementUV* uvLayer = layer->GetUVs())
			{
				ConvetToStl(uvs, &uvLayer->GetDirectArray());
				FbxLayerElement::EMappingMode Mapping = uvLayer->GetMappingMode();
				if (uvLayer->GetReferenceMode() == FbxLayerElement::eIndexToDirect)
					ConvetToStl(uv_indices, &uvLayer->GetIndexArray());
				else
					uv_indices = position_indices;
			}

			printf("Generating Vertices for  Mesh '%s'\n", nodeName.c_str());
			GenerateVertices();

			printf("Vertices Generated for Mesh '%s'\n", nodeName.c_str());
			printf("Triangles: %6d\n", processed_indices.size() / 3);
			printf("Points:    %6d\n", controlPointsCount);
			printf("Indices:   %6d\n", processed_indices.size());
			printf("Vertices:  %6d\n", processed_vertices.size());

		}
	}
}

bool SkinData::ExtractSkinWeights(FbxPose* p_pose, FbxNode* p_node,  FBXSkeleton& skel, FBXMatConverter& converter) {	
	FbxString nodeName = p_node->GetName();
	if( GetNodeAttributeType(p_node) == FbxNodeAttribute::eMesh )
	{
		printf("Extracting Skin Weights for '%s'\n" ,  nodeName.Buffer() );

		FbxMesh * p_mesh = p_node->GetMesh();
		int lVertexCount = p_mesh->GetControlPointsCount();

		// All the links must have the same link mode.
		FbxCluster::ELinkMode lClusterMode = ((FbxSkin*)p_mesh->GetDeformer(0, FbxDeformer::eSkin))->GetCluster(0)->GetLinkMode();

		point_weights.resize( lVertexCount );
		int lSkinCount=p_mesh->GetDeformerCount(FbxDeformer::eSkin);
		for(int i=0; i<lSkinCount; ++i)
		{
			FbxSkin * p_skin = (FbxSkin *)p_mesh->GetDeformer(i, FbxDeformer::eSkin);
			int lClusterCount = p_skin->GetClusterCount();

			for (int j=0; j<lClusterCount; ++j)
			{
				FbxCluster* p_cluster = p_skin->GetCluster(j);
				if (!p_cluster->GetLink())
					continue;

				FbxNode* p_link = p_cluster->GetLink();
				int bone_indx = skel.GetBoneIndex(p_node);

				//Bone does not have a bind pose so the Transform Link Matrix
				//as the bind pose
				int node_index = p_pose->Find( p_link );
				if( node_index == -1 )
				{
					FbxAMatrix linkBindMatrix;
					p_cluster->GetTransformLinkMatrix( linkBindMatrix );	
					p_pose->Add(p_link,FbxMatrix(linkBindMatrix));
				}

				int vertexIndexCount = p_cluster->GetControlPointIndicesCount();
				for (int k = 0; k < vertexIndexCount; ++k) 
				{            
					int lIndex = p_cluster->GetControlPointIndices()[k];
					double lWeight = p_cluster->GetControlPointWeights()[k];

					JointWeight j = {  lWeight , bone_indx };
					point_weights[ lIndex ].push_back( j );
				}
			}
		}
		const unsigned int MaxWeights = 4;
		//Normalize the skin weights for 4 weights for vertex that sum to one
		for( unsigned int i=0;i<point_weights.size();++i)
		{
			//Make sure there is MaxWeights by inserting dummies
			while( point_weights[i].size() < MaxWeights)
			{						
				JointWeight j = {  0 , 0 };
				point_weights[i].push_back( j );
			}

			//Normalize the weights
			float sum = 0.0f;
			for(int w=0;w<MaxWeights;++w)
				sum += point_weights[i][w].weight;
			for(int w=0;w<MaxWeights;++w)
				point_weights[i][w].weight /= sum;
		}

		return true;

	}
	else
	{
		printf("Could not get bone weights for mesh '%s'" , p_node->GetName() );
		return false;
	}
}
