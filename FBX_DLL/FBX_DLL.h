#pragma once

#include <fbxsdk.h>
#include <algorithm>
#include <set>

namespace FBXLibrary
{
	// This class is exported from the FBX_DLL.dll
	class FBX_Functions
	{
	private:
		Skeleton gSkeleton;
		Mesh gMesh;
		FbxScene *gScene;

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
		void DisplayAnimation(FbxScene* pScene);
		void DisplayAnimation(FbxAnimStack* pAnimStack, FbxNode* pNode, bool isSwitcher = false);
		void DisplayAnimation(FbxAnimLayer* pAnimLayer, FbxNode* pNode, bool isSwitcher = false);
		void DisplayChannels(FbxNode* pNode, FbxAnimLayer* pAnimLayer, bool isSwitcher);
		void DisplayCurveKeys(FbxAnimCurve* pCurve);
		void DisplayCurveKeys(FbxAnimCurve* pCurve, FbxNode* pNode, Vertex_Part pVertex_Part, Vertex_Part_Data pVertex_Part_Data, int pJointIndex);
		void DisplayListCurveKeys(FbxAnimCurve* pCurve, FbxProperty* pProperty);
		void ProcessJointsAndAnimations(FbxNode* inNode);
		unsigned int FindJointIndexUsingName(std::string currJointName);
		void ProcessControlPoints(FbxNode* inNode);

		// Personal functions for looping through skeleton hierarchy
		void ProcessSkeletonHierarchy(FbxNode* pRootNode);
		void ProcessSkeletonHierarchyRecursively(FbxNode* pInNode, int pInDepth, int pMyIndex, int pInParentIndex);
	
		// Personal function to get the matrices of nodes
		FbxAMatrix GetGeometryTransformation(FbxNode* pInNode);

		// Personal function to get the Skeleton
		Skeleton* getSkeleton();

		// Persoanl Function to construct vertices
		void ConstructKeyFrames();
		void ConstructUniqueVertices();
		void ConstructKeyTimes();
		int LargestKeyTime(std::vector<Keyframe_Vertex> pKeyframe_Vertices);
		int CompareKeyTime(Keyframe_Vertex pA, Keyframe_Vertex pB);
		Keyframe_Vertex GetDataAtKeyTime(std::vector<Keyframe_Vertex> pKeyframe_Infos, std::vector<int> pKeytimes, int pKeyTime);
		void FillOutJointKeyTimes(int pJointIndex);
	};
}
