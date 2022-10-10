#include <fbxsdk.h>
#include "FBXMatConverter.h"

FBXMatConverter::FBXMatConverter(FbxScene* pScene) {
	// Convert Axis System to what is used in this example, if needed
	FbxAxisSystem SceneAxisSystem = pScene->GetGlobalSettings().GetAxisSystem();

	FbxAxisSystem DxTransform(FbxAxisSystem::eDirectX);
	FbxAxisSystem MaxTransform(FbxAxisSystem::eMax);
	FbxAxisSystem MayaTransform(FbxAxisSystem::eMayaYUp);

	//construct the conversion matrix that takes a vector and converts 
	//it from one vector space to the other (invert the Z axis)
	ConversionMatrix.SetIdentity();
	NeedToChangedWinding = false;

	if (SceneAxisSystem == MaxTransform)
	{
		ConversionMatrix.SetRow(0, FbxVector4(1, 0, 0, 0));
		ConversionMatrix.SetRow(1, FbxVector4(0, 0, 1, 0));
		ConversionMatrix.SetRow(2, FbxVector4(0, 1, 0, 0));
		NeedToChangedWinding = true;
	}
	else if (SceneAxisSystem == MayaTransform)
	{
		ConversionMatrix.SetRow(0, FbxVector4(1, 0, 0, 0));
		ConversionMatrix.SetRow(1, FbxVector4(0, 1, 0, 0));
		ConversionMatrix.SetRow(2, FbxVector4(0, 0, -1, 0));
		NeedToChangedWinding = true;
	}
}

void FBXMatConverter::ConvertMatrix(FbxAMatrix& sourceMatX) {
	sourceMatX = ConversionMatrix * sourceMatX * ConversionMatrix;
}
