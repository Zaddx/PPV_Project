#pragma once

#include <fbxsdk.h>

namespace FBXLibrary
{
	// This class is exported from the FBX_DLL.dll
	class FBX_Functions
	{
	private:
		Skeleton gSkeleton;
		Mesh gMesh;
	public:
		FBX_Functions(void);
		// TODO: add your methods here.

		// SDK Stuff
		void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene);
		void DestroySdkObjects(FbxManager* pManager, bool pExitStatus);

		// Loading Stuff
		void BeginLoading(char** lFilename);
		bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename);
		
		// Displays
		void DisplayMetaData(FbxScene* pScene);
		void DisplayMesh(FbxNode* pNode);
		void DisplayPose(FbxScene* pScene);
		void DisplayString(const char* pHeader, const char* pValue = "", const char* pSuffix = "");
		void DisplayBool(const char* pHeader, bool pValue, const char* pSuffix = "");
		void DisplayInt(const char* pHeader, int pValue, const char* pSuffix = "");
		void DisplayDouble(const char* pHeader, double pValue, const char* pSuffix = "");
		void Display2DVector(const char* pHeader, FbxVector2 pValue, const char* pSuffix = "");
		void Display3DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix = "");
		void DisplayColor(const char* pHeader, FbxColor pValue, const char* pSuffix = "");
		void Display4DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix = "");
		void DisplayContent(FbxScene* pScene);
		void DisplayContent(FbxNode* pNode);
		void DisplayUserProperties(FbxObject* pObject);
		void DisplayTarget(FbxNode* pNode);
		void DisplayPivotsAndLimits(FbxNode* pNode);
		void DisplayTransformPropagation(FbxNode* pNode);
		void DisplayGeometricTransform(FbxNode* pNode);
		void DisplayControlsPoints(FbxMesh* pMesh);
		void DisplayPolygons(FbxMesh* pMesh);
		void DisplayTextureNames(FbxProperty &pProperty, FbxString& pConnectionString);
		void DisplayMaterialTextureConnections(FbxSurfaceMaterial* pMaterial, char * header, int pMatId, int l);
		void DisplayMaterialConnections(FbxMesh* pMesh);
		void DisplayMaterialMapping(FbxMesh* pMesh);
		void DisplayMetaDataConnections(FbxObject* pObject);
		void DisplayMaterial(FbxGeometry* pGeometry);
		void DisplayTexture(FbxGeometry* pGeometry);
		void FindAndDisplayTextureInfoByProperty(FbxProperty pProperty, bool& pDisplayHeader, int pMaterialIndex);
		void DisplayTextureInfo(FbxTexture* pTexture, int pBlendMode);
		void DisplayLink(FbxGeometry* pGeometry);
		void DisplayShape(FbxGeometry* pGeometry);
		void DisplayCache(FbxGeometry* pGeometry);

		// Personal functions for looping through skeleton hierarchy
		void ProcessSkeletonHierarchy(FbxNode* pRootNode);
		void ProcessSkeletonHierarchyRecursively(FbxNode* pInNode, int pInDepth, int pMyIndex, int pInParentIndex);
	
		// Personal function to get the matrices of nodes
		FbxAMatrix GetGeometryTransformation(FbxNode* pInNode);

		// Personal function to get the Skeleton
		Skeleton* getSkeleton();
	};
}
