// FBX_DLL.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "FBX_DLL.h"

static bool gVerbose = true;

#define MAT_HEADER_LENGTH 200

namespace FBXLibrary
{
	FBX_Functions::FBX_Functions()
	{

	}

	void FBX_Functions::BeginLoading(char** lFilename)
	{
		FbxManager* lSdkManager = NULL;
		FbxScene* lScene = NULL;
		bool lResult;

		// Prepare the FBX SDK.
		InitializeSdkObjects(lSdkManager, lScene);
		// Load the scene
		gScene = lScene;

		// DLL can take a filename as an argument
		FbxString lFilePath("");
		int pathSize = strlen(*lFilename);
		char* filepath = *lFilename;
		for (int i = 0; i < pathSize; i++)
		{
			if (FbxString(filepath[i]) == "-test")
				gVerbose = false;
			else /*if (lFilePath.IsEmpty())*/
				lFilePath += filepath[i];
		}
		if (lFilePath.IsEmpty())
		{
			lResult = false;
			FBXSDK_printf("\n\nUsage: ImportScene <FBX file name>\n\n");
		}
		else
		{
			FBXSDK_printf("\n\nFile: %s\n\n", lFilePath.Buffer());
			lResult = LoadScene(lSdkManager, lScene, lFilePath.Buffer());
		}

		if (lResult == false)
		{
			FBXSDK_printf("\n\nAn error occurred while loading the scene...");
		}
		else
		{
			// Display the scene.
			DisplayMetaData(lScene);

			//FBXSDK_printf("\n\n---------------------\nGlobal Light Settings\n---------------------\n\n");

			//if( gVerbose ) DisplayGlobalLightSettings(&lScene->GetGlobalSettings());

			//FBXSDK_printf("\n\n----------------------\nGlobal Camera Settings\n----------------------\n\n");

			//if( gVerbose ) DisplayGlobalCameraSettings(&lScene->GetGlobalSettings());

			//FBXSDK_printf("\n\n--------------------\nGlobal Time Settings\n--------------------\n\n");

			//if( gVerbose ) DisplayGlobalTimeSettings(&lScene->GetGlobalSettings());

			//FBXSDK_printf("\n\n---------\nHierarchy\n---------\n\n");

			//if( gVerbose ) DisplayHierarchy(lScene);

			FBXSDK_printf("\n\n------------\nNode Content\n------------\n\n");

			if (gVerbose) DisplayContent(lScene);

			FBXSDK_printf("\n\n----\nPose\n----\n\n");

			if (gVerbose) DisplayPose(lScene);

			FBXSDK_printf("\n\n---------\nAnimation\n---------\n\n");

			if (gVerbose) DisplayAnimation(lScene);

			//now display generic information

			/* FBXSDK_printf("\n\n---------\nGeneric Information\n---------\n\n");
			if( gVerbose ) DisplayGenericInfo(lScene);*/
		}

		// Destroy all objects created by the FBX SDK.
		DestroySdkObjects(lSdkManager, lResult);

	}

	void FBX_Functions::InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
	{
		//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
		pManager = FbxManager::Create();
		if (!pManager)
		{
			FBXSDK_printf("Error: Unable to create FBX Manager!\n");
			exit(1);
		}
		else FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

		//Create an IOSettings object. This object holds all import/export settings.
		FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
		pManager->SetIOSettings(ios);

		//Load plugins from the executable directory (optional)
		FbxString lPath = FbxGetApplicationDirectory();
		pManager->LoadPluginsDirectory(lPath.Buffer());

		//Create an FBX scene. This object holds most objects imported/exported from/to files.
		pScene = FbxScene::Create(pManager, "My Scene");
		if (!pScene)
		{
			// FBXSDK_printf("Error: Unable to create FBX scene!\n");
			MessageBox(0, L"Unable to create FBX scene!", L"Error", MB_OK);

			exit(1);
		}
	}

	void FBX_Functions::DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
	{
		//Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
		if (pManager) pManager->Destroy();
		if (pExitStatus) FBXSDK_printf("Program Success!\n");
	}

	bool FBX_Functions::LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
	{
		int lFileMajor, lFileMinor, lFileRevision;
		int lSDKMajor, lSDKMinor, lSDKRevision;
		//int lFileFormat = -1;
		int i, lAnimStackCount;
		bool lStatus;
		char lPassword[1024];

		// Get the file version number generate by the FBX SDK.
		FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

		// Create an importer.
		FbxImporter* lImporter = FbxImporter::Create(pManager, "");

		// Initialize the importer by providing a filename.
		const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
		lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

		if (!lImportStatus)
		{
			FbxString error = lImporter->GetStatus().GetErrorString();
			FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
			FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

			if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
			{
				FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
				FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
			}

			return false;
		}

		FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

		if (lImporter->IsFBX())
		{
			FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

			// From this point, it is possible to access animation stack information without
			// the expense of loading the entire file.

			FBXSDK_printf("Animation Stack Information\n");

			lAnimStackCount = lImporter->GetAnimStackCount();

			FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
			FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
			FBXSDK_printf("\n");

			for (i = 0; i < lAnimStackCount; i++)
			{
				FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

				FBXSDK_printf("    Animation Stack %d\n", i);
				FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
				FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

				// Change the value of the import name if the animation stack should be imported 
				// under a different name.
				FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

				// Set the value of the import state to false if the animation stack should be not
				// be imported. 
				FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
				FBXSDK_printf("\n");
			}
		}

		// Import the scene.
		lStatus = lImporter->Import(pScene);

		if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
		{
			FBXSDK_printf("Please enter password: ");

			lPassword[0] = '\0';

			FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
				scanf("%s", lPassword);
			FBXSDK_CRT_SECURE_NO_WARNING_END

				FbxString lString(lPassword);

			lStatus = lImporter->Import(pScene);

			if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
			{
				FBXSDK_printf("\nPassword is wrong, import aborted.\n");
			}
		}

		// Destroy the importer.
		lImporter->Destroy();

		return lStatus;
	}

	void FBX_Functions::DisplayMetaData(FbxScene* pScene)
	{
		FbxDocumentInfo* sceneInfo = pScene->GetSceneInfo();
		if (sceneInfo)
		{
			FBXSDK_printf("\n\n--------------------\nMeta-Data\n--------------------\n\n");
			FBXSDK_printf("    Title: %s\n", sceneInfo->mTitle.Buffer());
			FBXSDK_printf("    Subject: %s\n", sceneInfo->mSubject.Buffer());
			FBXSDK_printf("    Author: %s\n", sceneInfo->mAuthor.Buffer());
			FBXSDK_printf("    Keywords: %s\n", sceneInfo->mKeywords.Buffer());
			FBXSDK_printf("    Revision: %s\n", sceneInfo->mRevision.Buffer());
			FBXSDK_printf("    Comment: %s\n", sceneInfo->mComment.Buffer());

			FbxThumbnail* thumbnail = sceneInfo->GetSceneThumbnail();
			if (thumbnail)
			{
				FBXSDK_printf("    Thumbnail:\n");

				switch (thumbnail->GetDataFormat())
				{
				case FbxThumbnail::eRGB_24:
					FBXSDK_printf("        Format: RGB\n");
					break;
				case FbxThumbnail::eRGBA_32:
					FBXSDK_printf("        Format: RGBA\n");
					break;
				}

				switch (thumbnail->GetSize())
				{
				default:
					break;
				case FbxThumbnail::eNotSet:
					FBXSDK_printf("        Size: no dimensions specified (%ld bytes)\n", thumbnail->GetSizeInBytes());
					break;
				case FbxThumbnail::e64x64:
					FBXSDK_printf("        Size: 64 x 64 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
					break;
				case FbxThumbnail::e128x128:
					FBXSDK_printf("        Size: 128 x 128 pixels (%ld bytes)\n", thumbnail->GetSizeInBytes());
				}
			}
		}
	}

	void FBX_Functions::DisplayPose(FbxScene* pScene)
	{
		int      i, j, k, lPoseCount;
		FbxString  lName;

		lPoseCount = pScene->GetPoseCount();

		for (i = 0; i < lPoseCount; i++)
		{
			FbxPose* lPose = pScene->GetPose(i);

			lName = lPose->GetName();
			DisplayString("Pose Name: ", lName.Buffer());

			// Fill out the skeleton name
			gSkeleton.pName = lName;

			DisplayBool("    Is a bind pose: ", lPose->IsBindPose());

			DisplayInt("    Number of items in the pose: ", lPose->GetCount());

			DisplayString("", "");

			for (j = 0; j < lPose->GetCount(); j++)
			{
				lName = lPose->GetNodeName(j).GetCurrentName();
				DisplayString("    Item name: ", lName.Buffer());

				// Get the node and skeleton
				FbxNode *lNode = lPose->GetNode(j);
				FbxSkeleton *lSkeleton = lNode->GetSkeleton();

				// Get mesh
				FbxMesh* currMesh = lNode->GetMesh();
				unsigned int ctrlPointCount = currMesh->GetControlPointsCount();

				for (unsigned int i = 0; i < ctrlPointCount; i++)
				{
					CtrlPoint *tempCtrlPoint;
					gSkeleton.pControlPoints.push_back(tempCtrlPoint);
				}

				// Procces control points
				ProcessControlPoints(lNode);

				// Constrct Animation Data
				ProcessJointsAndAnimations(lNode);

				// Check to see if the skeleton eixsts
				// If it does then check to see if it's the root
				if (lSkeleton)
				{
					// Show if it's skeleton is root or not
					DisplayBool("    Is skeleton root: ", lSkeleton->IsSkeletonRoot());

					// Process the hierarchy
					if (lSkeleton->IsSkeletonRoot())
						ProcessSkeletonHierarchy(lNode);

					// Check to see if the Joint matches the current node
					// If so fill in the matrices of the joint
					std::string watchName = lNode->GetName();

					for (size_t m = 0; m < gSkeleton.pJoints.size(); m++)
					{
						if (gSkeleton.pJoints[m].pName == lNode->GetName())
						{
							FbxVector4 lXaxis = lNode->EvaluateGlobalTransform().GetRow(0);
							FbxVector4 lYaxis = lNode->EvaluateGlobalTransform().GetRow(1);
							FbxVector4 lZaxis = lNode->EvaluateGlobalTransform().GetRow(2);
							FbxVector4 lPosition = lNode->EvaluateGlobalTransform().GetRow(3);

							gSkeleton.pJoints[m].pXAxis = DirectX::XMFLOAT4(lXaxis[0], lXaxis[1], lXaxis[2], lXaxis[3]);
							gSkeleton.pJoints[m].pYAxis = DirectX::XMFLOAT4(lYaxis[0], lYaxis[1], lYaxis[2], lYaxis[3]);
							gSkeleton.pJoints[m].pZAxis = DirectX::XMFLOAT4(lZaxis[0], lZaxis[1], lZaxis[2], lZaxis[3]);
							gSkeleton.pJoints[m].pPosition = DirectX::XMFLOAT4(lPosition[0], lPosition[1], lPosition[2], lPosition[3]);

							gSkeleton.pJoints[m].pMatrix = DirectX::XMMATRIX(lXaxis[0], lXaxis[1], lXaxis[2], lXaxis[3],
								lYaxis[0], lYaxis[1], lYaxis[2], lYaxis[3],
								lZaxis[0], lZaxis[1], lZaxis[2], lZaxis[3],
								lPosition[0], lPosition[1], lPosition[2], lPosition[3]);
						}
					}

				}

				if (!lPose->IsBindPose())
				{
					// Rest pose can have local matrix
					DisplayBool("    Is local space matrix: ", lPose->IsLocalMatrix(j));
				}

				DisplayString("    Matrix value: ", "");

				FbxString lMatrixValue;

				for (k = 0; k < 4; k++)
				{
					FbxMatrix  lMatrix = lPose->GetMatrix(j);
					FbxVector4 lRow = lMatrix.GetRow(k);
					char        lRowValue[1024];

					FBXSDK_sprintf(lRowValue, 1024, "%9.4f %9.4f %9.4f %9.4f\n", lRow[0], lRow[1], lRow[2], lRow[3]);
					lMatrixValue += FbxString("		    ") + FbxString(lRowValue);
				}

				DisplayString("", lMatrixValue.Buffer());
			}
		}

		lPoseCount = pScene->GetCharacterPoseCount();

		for (i = 0; i < lPoseCount; i++)
		{
			FbxCharacterPose* lPose = pScene->GetCharacterPose(i);
			FbxCharacter*     lCharacter = lPose->GetCharacter();

			if (!lCharacter) break;

			DisplayString("Character Pose Name: ", lCharacter->GetName());

			FbxCharacterLink lCharacterLink;
			FbxCharacter::ENodeId  lNodeId = FbxCharacter::eHips;

			while (lCharacter->GetCharacterLink(lNodeId, &lCharacterLink))
			{
				FbxAMatrix& lGlobalPosition = lCharacterLink.mNode->EvaluateGlobalTransform(FBXSDK_TIME_ZERO);

				DisplayString("    Matrix value: ", "");

				FbxString lMatrixValue;

				for (k = 0; k < 4; k++)
				{
					FbxVector4 lRow = lGlobalPosition.GetRow(k);
					char        lRowValue[1024];

					FBXSDK_sprintf(lRowValue, 1024, "%9.4f %9.4f %9.4f %9.4f\n", lRow[0], lRow[1], lRow[2], lRow[3]);
					lMatrixValue += FbxString("        ") + FbxString(lRowValue);
				}

				DisplayString("", lMatrixValue.Buffer());

				lNodeId = FbxCharacter::ENodeId(int(lNodeId) + 1);
			}
		}
	}

	void FBX_Functions::DisplayString(const char* pHeader, const char* pValue /* = "" */, const char* pSuffix /* = "" */)
	{
		FbxString lString;

		lString = pHeader;
		lString += pValue;
		lString += pSuffix;
		lString += "\n";
		FBXSDK_printf(lString);
	}

	void FBX_Functions::DisplayBool(const char* pHeader, bool pValue, const char* pSuffix /* = "" */)
	{
		FbxString lString;

		lString = pHeader;
		lString += pValue ? "true" : "false";
		lString += pSuffix;
		lString += "\n";
		FBXSDK_printf(lString);
	}

	void FBX_Functions::DisplayInt(const char* pHeader, int pValue, const char* pSuffix /* = "" */)
	{
		FbxString lString;

		lString = pHeader;
		lString += pValue;
		lString += pSuffix;
		lString += "\n";
		FBXSDK_printf(lString);
	}

	void FBX_Functions::DisplayDouble(const char* pHeader, double pValue, const char* pSuffix /* = "" */)
	{
		FbxString lString;
		FbxString lFloatValue = (float)pValue;

		lFloatValue = pValue <= -HUGE_VAL ? "-INFINITY" : lFloatValue.Buffer();
		lFloatValue = pValue >= HUGE_VAL ? "INFINITY" : lFloatValue.Buffer();

		lString = pHeader;
		lString += lFloatValue;
		lString += pSuffix;
		lString += "\n";
		FBXSDK_printf(lString);
	}

	void FBX_Functions::Display2DVector(const char* pHeader, FbxVector2 pValue, const char* pSuffix  /* = "" */)
	{
		FbxString lString;
		FbxString lFloatValue1 = (float)pValue[0];
		FbxString lFloatValue2 = (float)pValue[1];

		lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
		lFloatValue1 = pValue[0] >= HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
		lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
		lFloatValue2 = pValue[1] >= HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();

		lString = pHeader;
		lString += lFloatValue1;
		lString += ", ";
		lString += lFloatValue2;
		lString += pSuffix;
		lString += "\n";
		FBXSDK_printf(lString);
	}

	void FBX_Functions::Display3DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix /* = "" */)
	{
		FbxString lString;
		FbxString lFloatValue1 = (float)pValue[0];
		FbxString lFloatValue2 = (float)pValue[1];
		FbxString lFloatValue3 = (float)pValue[2];

		lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
		lFloatValue1 = pValue[0] >= HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
		lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
		lFloatValue2 = pValue[1] >= HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
		lFloatValue3 = pValue[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
		lFloatValue3 = pValue[2] >= HUGE_VAL ? "INFINITY" : lFloatValue3.Buffer();

		lString = pHeader;
		lString += lFloatValue1;
		lString += ", ";
		lString += lFloatValue2;
		lString += ", ";
		lString += lFloatValue3;
		lString += pSuffix;
		lString += "\n";
		FBXSDK_printf(lString);
	}

	void FBX_Functions::Display4DVector(const char* pHeader, FbxVector4 pValue, const char* pSuffix /* = "" */)
	{
		FbxString lString;
		FbxString lFloatValue1 = (float)pValue[0];
		FbxString lFloatValue2 = (float)pValue[1];
		FbxString lFloatValue3 = (float)pValue[2];
		FbxString lFloatValue4 = (float)pValue[3];

		lFloatValue1 = pValue[0] <= -HUGE_VAL ? "-INFINITY" : lFloatValue1.Buffer();
		lFloatValue1 = pValue[0] >= HUGE_VAL ? "INFINITY" : lFloatValue1.Buffer();
		lFloatValue2 = pValue[1] <= -HUGE_VAL ? "-INFINITY" : lFloatValue2.Buffer();
		lFloatValue2 = pValue[1] >= HUGE_VAL ? "INFINITY" : lFloatValue2.Buffer();
		lFloatValue3 = pValue[2] <= -HUGE_VAL ? "-INFINITY" : lFloatValue3.Buffer();
		lFloatValue3 = pValue[2] >= HUGE_VAL ? "INFINITY" : lFloatValue3.Buffer();
		lFloatValue4 = pValue[3] <= -HUGE_VAL ? "-INFINITY" : lFloatValue4.Buffer();
		lFloatValue4 = pValue[3] >= HUGE_VAL ? "INFINITY" : lFloatValue4.Buffer();

		lString = pHeader;
		lString += lFloatValue1;
		lString += ", ";
		lString += lFloatValue2;
		lString += ", ";
		lString += lFloatValue3;
		lString += ", ";
		lString += lFloatValue4;
		lString += pSuffix;
		lString += "\n";
		FBXSDK_printf(lString);
	}

	void DisplayColor(const char* pHeader, FbxPropertyT<FbxDouble3> pValue, const char* pSuffix /* = "" */)
	{
		FbxString lString;

		lString = pHeader;
		//lString += (float) pValue.mRed;
		//lString += (double)pValue.GetArrayItem(0);
		lString += " (red), ";
		//lString += (float) pValue.mGreen;
		//lString += (double)pValue.GetArrayItem(1);
		lString += " (green), ";
		//lString += (float) pValue.mBlue;
		//lString += (double)pValue.GetArrayItem(2);
		lString += " (blue)";
		lString += pSuffix;
		lString += "\n";
		FBXSDK_printf(lString);
	}

	void FBX_Functions::DisplayColor(const char* pHeader, FbxColor pValue, const char* pSuffix /* = "" */)
	{
		FbxString lString;

		lString = pHeader;
		lString += (float)pValue.mRed;

		lString += " (red), ";
		lString += (float)pValue.mGreen;

		lString += " (green), ";
		lString += (float)pValue.mBlue;

		lString += " (blue)";
		lString += pSuffix;
		lString += "\n";
		FBXSDK_printf(lString);
	}

	void FBX_Functions::DisplayContent(FbxScene* pScene)
	{
		int i;
		FbxNode* lNode = pScene->GetRootNode();

		if (lNode)
		{
			for (i = 0; i < lNode->GetChildCount(); i++)
			{
				DisplayContent(lNode->GetChild(i));
			}
		}
	}

	void FBX_Functions::DisplayContent(FbxNode* pNode)
	{
		FbxNodeAttribute::EType lAttributeType;
		int i;

		if (pNode->GetNodeAttribute() == NULL)
		{
			FBXSDK_printf("NULL Node Attribute\n\n");
		}
		else
		{
			lAttributeType = (pNode->GetNodeAttribute()->GetAttributeType());

			switch (lAttributeType)
			{
			default:
				break;
				/*case FbxNodeAttribute::eMarker:
				DisplayMarker(pNode);
				break;*/

				//case FbxNodeAttribute::eSkeleton:  
				//    DisplaySkeleton(pNode);
				//    break;

			case FbxNodeAttribute::eMesh:
				DisplayMesh(pNode);
				break;

				//case FbxNodeAttribute::eNurbs:      
				//    DisplayNurb(pNode);
				//    break;

				//case FbxNodeAttribute::ePatch:     
				//    DisplayPatch(pNode);
				//    break;

				//case FbxNodeAttribute::eCamera:    
				//    DisplayCamera(pNode);
				//    break;

				//case FbxNodeAttribute::eLight:     
				//    DisplayLight(pNode);
				//    break;

				//case FbxNodeAttribute::eLODGroup:
				//    DisplayLodGroup(pNode);
				//    break;
			}
		}

		DisplayUserProperties(pNode);
		DisplayTarget(pNode);
		DisplayPivotsAndLimits(pNode);
		DisplayTransformPropagation(pNode);
		DisplayGeometricTransform(pNode);

		for (i = 0; i < pNode->GetChildCount(); i++)
		{
			DisplayContent(pNode->GetChild(i));
		}
	}

	void FBX_Functions::DisplayUserProperties(FbxObject* pObject)
	{
		int lCount = 0;
		FbxString lTitleStr = "    Property Count: ";

		FbxProperty lProperty = pObject->GetFirstProperty();
		while (lProperty.IsValid())
		{
			if (lProperty.GetFlag(FbxPropertyFlags::eUserDefined))
				lCount++;

			lProperty = pObject->GetNextProperty(lProperty);
		}

		if (lCount == 0)
			return; // there are no user properties to display

		DisplayInt(lTitleStr.Buffer(), lCount);

		lProperty = pObject->GetFirstProperty();
		int i = 0;
		while (lProperty.IsValid())
		{
			if (lProperty.GetFlag(FbxPropertyFlags::eUserDefined))
			{
				DisplayInt("        Property ", i);
				FbxString lString = lProperty.GetLabel();
				DisplayString("            Display Name: ", lString.Buffer());
				lString = lProperty.GetName();
				DisplayString("            Internal Name: ", lString.Buffer());
				DisplayString("            Type: ", lProperty.GetPropertyDataType().GetName());
				if (lProperty.HasMinLimit()) DisplayDouble("            Min Limit: ", lProperty.GetMinLimit());
				if (lProperty.HasMaxLimit()) DisplayDouble("            Max Limit: ", lProperty.GetMaxLimit());
				DisplayBool("            Is Animatable: ", lProperty.GetFlag(FbxPropertyFlags::eAnimatable));

				FbxDataType lPropertyDataType = lProperty.GetPropertyDataType();

				// BOOL
				if (lPropertyDataType.GetType() == eFbxBool)
				{
					DisplayBool("            Default Value: ", lProperty.Get<FbxBool>());
				}
				// REAL
				else if (lPropertyDataType.GetType() == eFbxDouble || lPropertyDataType.GetType() == eFbxFloat)
				{
					DisplayDouble("            Default Value: ", lProperty.Get<FbxDouble>());
				}
				// COLOR
				else if (lPropertyDataType.Is(FbxColor3DT) || lPropertyDataType.Is(FbxColor4DT))
				{
					FbxColor lDefault;
					char      lBuf[64];

					lDefault = lProperty.Get<FbxColor>();
					FBXSDK_sprintf(lBuf, 64, "R=%f, G=%f, B=%f, A=%f", lDefault.mRed, lDefault.mGreen, lDefault.mBlue, lDefault.mAlpha);
					DisplayString("            Default Value: ", lBuf);
				}
				// INTEGER
				else if (lPropertyDataType.GetType() == eFbxInt)
				{
					DisplayInt("            Default Value: ", lProperty.Get<FbxInt>());
				}
				// VECTOR
				else if (lPropertyDataType.GetType() == eFbxDouble3 || lPropertyDataType.GetType() == eFbxDouble4)
				{
					FbxDouble3 lDefault;
					char   lBuf[64];

					lDefault = lProperty.Get<FbxDouble3>();
					FBXSDK_sprintf(lBuf, 64, "X=%f, Y=%f, Z=%f", lDefault[0], lDefault[1], lDefault[2]);
					DisplayString("            Default Value: ", lBuf);
				}
				// LIST
				else if (lPropertyDataType.GetType() == eFbxEnum)
				{
					DisplayInt("            Default Value: ", lProperty.Get<FbxEnum>());
				}
				// UNIDENTIFIED
				else
				{
					DisplayString("            Default Value: UNIDENTIFIED");
				}
				i++;
			}

			lProperty = pObject->GetNextProperty(lProperty);
		}
	}

	void FBX_Functions::DisplayTarget(FbxNode* pNode)
	{
		if (pNode->GetTarget() != NULL)
		{
			DisplayString("    Target Name: ", (char *)pNode->GetTarget()->GetName());
		}
	}

	void FBX_Functions::DisplayPivotsAndLimits(FbxNode* pNode)
	{
		FbxVector4 lTmpVector;

		//
		// Pivots
		//
		FBXSDK_printf("    Pivot Information\n");

		FbxNode::EPivotState lPivotState;
		pNode->GetPivotState(FbxNode::eSourcePivot, lPivotState);
		FBXSDK_printf("        Pivot State: %s\n", lPivotState == FbxNode::ePivotActive ? "Active" : "Reference");

		lTmpVector = pNode->GetPreRotation(FbxNode::eSourcePivot);
		FBXSDK_printf("        Pre-Rotation: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

		lTmpVector = pNode->GetPostRotation(FbxNode::eSourcePivot);
		FBXSDK_printf("        Post-Rotation: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

		lTmpVector = pNode->GetRotationPivot(FbxNode::eSourcePivot);
		FBXSDK_printf("        Rotation Pivot: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

		lTmpVector = pNode->GetRotationOffset(FbxNode::eSourcePivot);
		FBXSDK_printf("        Rotation Offset: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

		lTmpVector = pNode->GetScalingPivot(FbxNode::eSourcePivot);
		FBXSDK_printf("        Scaling Pivot: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

		lTmpVector = pNode->GetScalingOffset(FbxNode::eSourcePivot);
		FBXSDK_printf("        Scaling Offset: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

		//
		// Limits
		//
		bool		lIsActive, lMinXActive, lMinYActive, lMinZActive;
		bool		lMaxXActive, lMaxYActive, lMaxZActive;
		FbxDouble3	lMinValues, lMaxValues;

		FBXSDK_printf("    Limits Information\n");

		lIsActive = pNode->TranslationActive;
		lMinXActive = pNode->TranslationMinX;
		lMinYActive = pNode->TranslationMinY;
		lMinZActive = pNode->TranslationMinZ;
		lMaxXActive = pNode->TranslationMaxX;
		lMaxYActive = pNode->TranslationMaxY;
		lMaxZActive = pNode->TranslationMaxZ;
		lMinValues = pNode->TranslationMin;
		lMaxValues = pNode->TranslationMax;

		FBXSDK_printf("        Translation limits: %s\n", lIsActive ? "Active" : "Inactive");
		FBXSDK_printf("            X\n");
		FBXSDK_printf("                Min Limit: %s\n", lMinXActive ? "Active" : "Inactive");
		FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[0]);
		FBXSDK_printf("                Max Limit: %s\n", lMaxXActive ? "Active" : "Inactive");
		FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[0]);
		FBXSDK_printf("            Y\n");
		FBXSDK_printf("                Min Limit: %s\n", lMinYActive ? "Active" : "Inactive");
		FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[1]);
		FBXSDK_printf("                Max Limit: %s\n", lMaxYActive ? "Active" : "Inactive");
		FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[1]);
		FBXSDK_printf("            Z\n");
		FBXSDK_printf("                Min Limit: %s\n", lMinZActive ? "Active" : "Inactive");
		FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[2]);
		FBXSDK_printf("                Max Limit: %s\n", lMaxZActive ? "Active" : "Inactive");
		FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[2]);

		lIsActive = pNode->RotationActive;
		lMinXActive = pNode->RotationMinX;
		lMinYActive = pNode->RotationMinY;
		lMinZActive = pNode->RotationMinZ;
		lMaxXActive = pNode->RotationMaxX;
		lMaxYActive = pNode->RotationMaxY;
		lMaxZActive = pNode->RotationMaxZ;
		lMinValues = pNode->RotationMin;
		lMaxValues = pNode->RotationMax;

		FBXSDK_printf("        Rotation limits: %s\n", lIsActive ? "Active" : "Inactive");
		FBXSDK_printf("            X\n");
		FBXSDK_printf("                Min Limit: %s\n", lMinXActive ? "Active" : "Inactive");
		FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[0]);
		FBXSDK_printf("                Max Limit: %s\n", lMaxXActive ? "Active" : "Inactive");
		FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[0]);
		FBXSDK_printf("            Y\n");
		FBXSDK_printf("                Min Limit: %s\n", lMinYActive ? "Active" : "Inactive");
		FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[1]);
		FBXSDK_printf("                Max Limit: %s\n", lMaxYActive ? "Active" : "Inactive");
		FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[1]);
		FBXSDK_printf("            Z\n");
		FBXSDK_printf("                Min Limit: %s\n", lMinZActive ? "Active" : "Inactive");
		FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[2]);
		FBXSDK_printf("                Max Limit: %s\n", lMaxZActive ? "Active" : "Inactive");
		FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[2]);

		lIsActive = pNode->ScalingActive;
		lMinXActive = pNode->ScalingMinX;
		lMinYActive = pNode->ScalingMinY;
		lMinZActive = pNode->ScalingMinZ;
		lMaxXActive = pNode->ScalingMaxX;
		lMaxYActive = pNode->ScalingMaxY;
		lMaxZActive = pNode->ScalingMaxZ;
		lMinValues = pNode->ScalingMin;
		lMaxValues = pNode->ScalingMax;

		FBXSDK_printf("        Scaling limits: %s\n", lIsActive ? "Active" : "Inactive");
		FBXSDK_printf("            X\n");
		FBXSDK_printf("                Min Limit: %s\n", lMinXActive ? "Active" : "Inactive");
		FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[0]);
		FBXSDK_printf("                Max Limit: %s\n", lMaxXActive ? "Active" : "Inactive");
		FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[0]);
		FBXSDK_printf("            Y\n");
		FBXSDK_printf("                Min Limit: %s\n", lMinYActive ? "Active" : "Inactive");
		FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[1]);
		FBXSDK_printf("                Max Limit: %s\n", lMaxYActive ? "Active" : "Inactive");
		FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[1]);
		FBXSDK_printf("            Z\n");
		FBXSDK_printf("                Min Limit: %s\n", lMinZActive ? "Active" : "Inactive");
		FBXSDK_printf("                Min Limit Value: %f\n", lMinValues[2]);
		FBXSDK_printf("                Max Limit: %s\n", lMaxZActive ? "Active" : "Inactive");
		FBXSDK_printf("                Max Limit Value: %f\n", lMaxValues[2]);
	}

	void FBX_Functions::DisplayTransformPropagation(FbxNode* pNode)
	{
		FBXSDK_printf("    Transformation Propagation\n");

		// 
		// Rotation Space
		//
		EFbxRotationOrder lRotationOrder;
		pNode->GetRotationOrder(FbxNode::eSourcePivot, lRotationOrder);

		FBXSDK_printf("        Rotation Space: ");

		switch (lRotationOrder)
		{
		case eEulerXYZ:
			FBXSDK_printf("Euler XYZ\n");
			break;
		case eEulerXZY:
			FBXSDK_printf("Euler XZY\n");
			break;
		case eEulerYZX:
			FBXSDK_printf("Euler YZX\n");
			break;
		case eEulerYXZ:
			FBXSDK_printf("Euler YXZ\n");
			break;
		case eEulerZXY:
			FBXSDK_printf("Euler ZXY\n");
			break;
		case eEulerZYX:
			FBXSDK_printf("Euler ZYX\n");
			break;
		case eSphericXYZ:
			FBXSDK_printf("Spheric XYZ\n");
			break;
		}

		//
		// Use the Rotation space only for the limits
		// (keep using eEulerXYZ for the rest)
		//
		FBXSDK_printf("        Use the Rotation Space for Limit specification only: %s\n",
			pNode->GetUseRotationSpaceForLimitOnly(FbxNode::eSourcePivot) ? "Yes" : "No");


		//
		// Inherit Type
		//
		FbxTransform::EInheritType lInheritType;
		pNode->GetTransformationInheritType(lInheritType);

		FBXSDK_printf("        Transformation Inheritance: ");

		switch (lInheritType)
		{
		case FbxTransform::eInheritRrSs:
			FBXSDK_printf("RrSs\n");
			break;
		case FbxTransform::eInheritRSrs:
			FBXSDK_printf("RSrs\n");
			break;
		case FbxTransform::eInheritRrs:
			FBXSDK_printf("Rrs\n");
			break;
		}
	}

	void FBX_Functions::DisplayGeometricTransform(FbxNode* pNode)
	{
		FbxVector4 lTmpVector;

		FBXSDK_printf("    Geometric Transformations\n");

		//
		// Translation
		//
		lTmpVector = pNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		FBXSDK_printf("        Translation: %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

		//
		// Rotation
		//
		lTmpVector = pNode->GetGeometricRotation(FbxNode::eSourcePivot);
		FBXSDK_printf("        Rotation:    %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);

		//
		// Scaling
		//
		lTmpVector = pNode->GetGeometricScaling(FbxNode::eSourcePivot);
		FBXSDK_printf("        Scaling:     %f %f %f\n", lTmpVector[0], lTmpVector[1], lTmpVector[2]);
	}

	void FBX_Functions::ProcessSkeletonHierarchy(FbxNode* pRootNode)
	{
		for (int childIndex = 0; childIndex < pRootNode->GetChildCount(); ++childIndex)
		{
			FbxNode* currNode = pRootNode->GetChild(childIndex);
			std::string nodeName = currNode->GetName();
			ProcessSkeletonHierarchyRecursively(currNode, 0, 0, -1);
		}
	}

	void FBX_Functions::ProcessSkeletonHierarchyRecursively(FbxNode* pInNode, int pInDepth, int pMyIndex, int pInParentIndex)
	{
		if (pInNode->GetNodeAttribute() && pInNode->GetNodeAttribute()->GetAttributeType() && pInNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
		{
			Joint currJoint;
			currJoint.pParentIndex = pInParentIndex;
			currJoint.pName = pInNode->GetName();
			gSkeleton.pJoints.push_back(currJoint);
		}
		for (int i = 0; i < pInNode->GetChildCount(); i++)
		{
			ProcessSkeletonHierarchyRecursively(pInNode->GetChild(i), pInDepth + 1, gSkeleton.pJoints.size(), pMyIndex);
		}
	}

	FbxAMatrix FBX_Functions::GetGeometryTransformation(FbxNode* pInNode)
	{
		if (!pInNode)
			throw std::exception("Null for mesh geometry");

		const FbxVector4 lT = pInNode->GetGeometricTranslation(FbxNode::eSourcePivot);
		const FbxVector4 lR = pInNode->GetGeometricRotation(FbxNode::eSourcePivot);
		const FbxVector4 lS = pInNode->GetGeometricScaling(FbxNode::eSourcePivot);

		return FbxAMatrix(lT, lR, lS);
	}

	Skeleton* FBX_Functions::getSkeleton()
	{
		// Construct Keyframes before giving skeleton
		ConstructKeyFrames();

		return &gSkeleton;
	}

	void FBX_Functions::DisplayControlsPoints(FbxMesh* pMesh)
	{
		int i, lControlPointsCount = pMesh->GetControlPointsCount();
		FbxVector4* lControlPoints = pMesh->GetControlPoints();

		DisplayString("    Control Points");

		for (i = 0; i < lControlPointsCount; i++)
		{
			DisplayInt("        Control Point ", i);
			Display3DVector("            Coordinates: ", lControlPoints[i]);

			for (int j = 0; j < pMesh->GetElementNormalCount(); j++)
			{
				FbxGeometryElementNormal* leNormals = pMesh->GetElementNormal(j);
				if (leNormals->GetMappingMode() == FbxGeometryElement::eByControlPoint)
				{
					char header[100];
					FBXSDK_sprintf(header, 100, "            Normal Vector: ");
					if (leNormals->GetReferenceMode() == FbxGeometryElement::eDirect)
						Display3DVector(header, leNormals->GetDirectArray().GetAt(i));
				}
			}
		}

		DisplayString("");
	}

	void FBX_Functions::DisplayPolygons(FbxMesh* pMesh)
	{
		int i, j, lPolygonCount = pMesh->GetPolygonCount();
		FbxVector4* lControlPoints = pMesh->GetControlPoints();
		char header[100];

		DisplayString("    Polygons");

		int vertexId = 0;
		for (i = 0; i < lPolygonCount; i++)
		{
			// A temporary mesh polygon that will be filled out
			// and later pushed into the mesh skeleton
			Mesh_Polygon lTempPolygon;

			DisplayInt("        Polygon ", i);
			int l;

			for (l = 0; l < pMesh->GetElementPolygonGroupCount(); l++)
			{
				FbxGeometryElementPolygonGroup* lePolgrp = pMesh->GetElementPolygonGroup(l);
				switch (lePolgrp->GetMappingMode())
				{
				case FbxGeometryElement::eByPolygon:
					if (lePolgrp->GetReferenceMode() == FbxGeometryElement::eIndex)
					{
						FBXSDK_sprintf(header, 100, "        Assigned to group: ");
						int polyGroupId = lePolgrp->GetIndexArray().GetAt(i);
						DisplayInt(header, polyGroupId);
						break;
					}
				default:
					// any other mapping modes don't make sense
					DisplayString("        \"unsupported group assignment\"");
					break;
				}
			}

			int lPolygonSize = pMesh->GetPolygonSize(i);

			for (j = 0; j < lPolygonSize; j++)
			{
				// A temporary mesh vertex that will be filled out
				// and later pushed into the mesh polygon
				Mesh_Vertex lTempVertex;

				int lControlPointIndex = pMesh->GetPolygonVertex(i, j);

				Display3DVector("            Coordinates: ", lControlPoints[lControlPointIndex]);
				lTempVertex.pPosition = DirectX::XMFLOAT4(lControlPoints[lControlPointIndex][0], lControlPoints[lControlPointIndex][1], lControlPoints[lControlPointIndex][2], lControlPoints[lControlPointIndex][3]);

				for (l = 0; l < pMesh->GetElementVertexColorCount(); l++)
				{
					FbxGeometryElementVertexColor* leVtxc = pMesh->GetElementVertexColor(l);
					FBXSDK_sprintf(header, 100, "            Color vertex: ");

					switch (leVtxc->GetMappingMode())
					{
					default:
						break;
					case FbxGeometryElement::eByControlPoint:
						switch (leVtxc->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							DisplayColor(header, leVtxc->GetDirectArray().GetAt(lControlPointIndex));
							break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leVtxc->GetIndexArray().GetAt(lControlPointIndex);
							DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));
						}
						break;
						default:
							break; // other reference modes not shown here!
						}
						break;

					case FbxGeometryElement::eByPolygonVertex:
					{
						switch (leVtxc->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							DisplayColor(header, leVtxc->GetDirectArray().GetAt(vertexId));
							break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leVtxc->GetIndexArray().GetAt(vertexId);
							DisplayColor(header, leVtxc->GetDirectArray().GetAt(id));
						}
						break;
						default:
							break; // other reference modes not shown here!
						}
					}
					break;

					case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
					case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
					case FbxGeometryElement::eNone:       // doesn't make much sense for UVs
						break;
					}
				}
				for (l = 0; l < pMesh->GetElementUVCount(); ++l)
				{
					FbxGeometryElementUV* leUV = pMesh->GetElementUV(l);
					FBXSDK_sprintf(header, 100, "            Texture UV: ");

					switch (leUV->GetMappingMode())
					{
					default:
						break;
					case FbxGeometryElement::eByControlPoint:
						switch (leUV->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
						{
							Display2DVector(header, leUV->GetDirectArray().GetAt(lControlPointIndex));
							lTempVertex.pUV = DirectX::XMFLOAT2(leUV->GetDirectArray().GetAt(lControlPointIndex)[0], leUV->GetDirectArray().GetAt(lControlPointIndex)[1]);
							break;
						}
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leUV->GetIndexArray().GetAt(lControlPointIndex);
							Display2DVector(header, leUV->GetDirectArray().GetAt(id));
							lTempVertex.pUV = DirectX::XMFLOAT2(leUV->GetDirectArray().GetAt(id)[0], leUV->GetDirectArray().GetAt(id)[1]);
							break;
						}
						default:
							break; // other reference modes not shown here!
						}
						break;

					case FbxGeometryElement::eByPolygonVertex:
					{
						int lTextureUVIndex = pMesh->GetTextureUVIndex(i, j);
						switch (leUV->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
						case FbxGeometryElement::eIndexToDirect:
						{
							Display2DVector(header, leUV->GetDirectArray().GetAt(lTextureUVIndex));
							lTempVertex.pUV = DirectX::XMFLOAT2(leUV->GetDirectArray().GetAt(lTextureUVIndex)[0], leUV->GetDirectArray().GetAt(lTextureUVIndex)[1]);
							break;
						}
						default:
							break; // other reference modes not shown here!
						}
					}
					break;

					case FbxGeometryElement::eByPolygon: // doesn't make much sense for UVs
					case FbxGeometryElement::eAllSame:   // doesn't make much sense for UVs
					case FbxGeometryElement::eNone:      // doesn't make much sense for UVs
						break;
					}
				}
				for (l = 0; l < pMesh->GetElementNormalCount(); ++l)
				{
					FbxGeometryElementNormal* leNormal = pMesh->GetElementNormal(l);
					FBXSDK_sprintf(header, 100, "            Normal: ");

					if (leNormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (leNormal->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
						{
							Display3DVector(header, leNormal->GetDirectArray().GetAt(vertexId));
							lTempVertex.pNormal = DirectX::XMFLOAT3(leNormal->GetDirectArray().GetAt(vertexId)[0], leNormal->GetDirectArray().GetAt(vertexId)[1], leNormal->GetDirectArray().GetAt(vertexId)[2]);
							break;
						}
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leNormal->GetIndexArray().GetAt(vertexId);
							Display3DVector(header, leNormal->GetDirectArray().GetAt(id));
							lTempVertex.pNormal = DirectX::XMFLOAT3(leNormal->GetDirectArray().GetAt(id)[0], leNormal->GetDirectArray().GetAt(id)[1], leNormal->GetDirectArray().GetAt(id)[2]);
							break;
						}
						default:
							break; // other reference modes not shown here!
						}
					}

				}
				for (l = 0; l < pMesh->GetElementTangentCount(); ++l)
				{
					FbxGeometryElementTangent* leTangent = pMesh->GetElementTangent(l);
					FBXSDK_sprintf(header, 100, "            Tangent: ");

					if (leTangent->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (leTangent->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							Display3DVector(header, leTangent->GetDirectArray().GetAt(vertexId));
							break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leTangent->GetIndexArray().GetAt(vertexId);
							Display3DVector(header, leTangent->GetDirectArray().GetAt(id));
						}
						break;
						default:
							break; // other reference modes not shown here!
						}
					}

				}
				for (l = 0; l < pMesh->GetElementBinormalCount(); ++l)
				{

					FbxGeometryElementBinormal* leBinormal = pMesh->GetElementBinormal(l);

					FBXSDK_sprintf(header, 100, "            Binormal: ");
					if (leBinormal->GetMappingMode() == FbxGeometryElement::eByPolygonVertex)
					{
						switch (leBinormal->GetReferenceMode())
						{
						case FbxGeometryElement::eDirect:
							Display3DVector(header, leBinormal->GetDirectArray().GetAt(vertexId));
							break;
						case FbxGeometryElement::eIndexToDirect:
						{
							int id = leBinormal->GetIndexArray().GetAt(vertexId);
							Display3DVector(header, leBinormal->GetDirectArray().GetAt(id));
						}
						break;
						default:
							break; // other reference modes not shown here!
						}
					}
				}
				vertexId++;

				// Pushback each vertex into the polygon
				lTempPolygon.pVertices.push_back(lTempVertex);

				// Pushback each vertex into the mesh
				gMesh.pVertices.push_back(lTempVertex);

			} // for polygonSize

			// Pushback each polygon mesh into the overall mesh
			gMesh.pPolygon.push_back(lTempPolygon);

		} // for polygonCount

		// Update the skeletons mesh to the gMesh
		gSkeleton.pMesh = gMesh;

		//check visibility for the edges of the mesh
		for (int l = 0; l < pMesh->GetElementVisibilityCount(); ++l)
		{
			FbxGeometryElementVisibility* leVisibility = pMesh->GetElementVisibility(l);
			FBXSDK_sprintf(header, 100, "    Edge Visibility : ");
			DisplayString(header);
			switch (leVisibility->GetMappingMode())
			{
			default:
				break;
				//should be eByEdge
			case FbxGeometryElement::eByEdge:
				//should be eDirect
				for (j = 0; j != pMesh->GetMeshEdgeCount(); ++j)
				{
					DisplayInt("        Edge ", j);
					DisplayBool("              Edge visibility: ", leVisibility->GetDirectArray().GetAt(j));
				}

				break;
			}
		}
		DisplayString("");
	}

	void FBX_Functions::DisplayTextureNames(FbxProperty &pProperty, FbxString& pConnectionString)
	{
		int lLayeredTextureCount = pProperty.GetSrcObjectCount<FbxLayeredTexture>();
		if (lLayeredTextureCount > 0)
		{
			for (int j = 0; j < lLayeredTextureCount; ++j)
			{
				FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
				int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();
				pConnectionString += " Texture ";

				for (int k = 0; k < lNbTextures; ++k)
				{
					//lConnectionString += k;
					pConnectionString += "\"";
					pConnectionString += (char*)lLayeredTexture->GetName();
					gMesh.pTextureName = lLayeredTexture->GetName();
					pConnectionString += "\"";
					pConnectionString += " ";
				}
				pConnectionString += "of ";
				pConnectionString += pProperty.GetName();
				pConnectionString += " on layer ";
				pConnectionString += j;
			}
			pConnectionString += " |";
		}
		else
		{
			//no layered texture simply get on the property
			int lNbTextures = pProperty.GetSrcObjectCount<FbxTexture>();

			if (lNbTextures > 0)
			{
				pConnectionString += " Texture ";
				pConnectionString += " ";

				for (int j = 0; j < lNbTextures; ++j)
				{
					FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
					if (lTexture)
					{
						pConnectionString += "\"";
						pConnectionString += (char*)lTexture->GetName();
						pConnectionString += "\"";
						pConnectionString += " ";
					}
				}
				pConnectionString += "of ";
				pConnectionString += pProperty.GetName();
				pConnectionString += " |";
			}
		}
	}

	void FBX_Functions::DisplayMaterialTextureConnections(FbxSurfaceMaterial* pMaterial, char * header, int pMatId, int l)
	{
		if (!pMaterial)
			return;

		FbxString lConnectionString = "            Material %d -- ";
		//Show all the textures

		FbxProperty lProperty;
		//Diffuse Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		DisplayTextureNames(lProperty, lConnectionString);

		//DiffuseFactor Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sDiffuseFactor);
		DisplayTextureNames(lProperty, lConnectionString);

		//Emissive Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sEmissive);
		DisplayTextureNames(lProperty, lConnectionString);

		//EmissiveFactor Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sEmissiveFactor);
		DisplayTextureNames(lProperty, lConnectionString);


		//Ambient Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sAmbient);
		DisplayTextureNames(lProperty, lConnectionString);

		//AmbientFactor Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sAmbientFactor);
		DisplayTextureNames(lProperty, lConnectionString);

		//Specular Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sSpecular);
		DisplayTextureNames(lProperty, lConnectionString);

		//SpecularFactor Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sSpecularFactor);
		DisplayTextureNames(lProperty, lConnectionString);

		//Shininess Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sShininess);
		DisplayTextureNames(lProperty, lConnectionString);

		//Bump Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sBump);
		DisplayTextureNames(lProperty, lConnectionString);

		//Normal Map Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
		DisplayTextureNames(lProperty, lConnectionString);

		//Transparent Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sTransparentColor);
		DisplayTextureNames(lProperty, lConnectionString);

		//TransparencyFactor Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sTransparencyFactor);
		DisplayTextureNames(lProperty, lConnectionString);

		//Reflection Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sReflection);
		DisplayTextureNames(lProperty, lConnectionString);

		//ReflectionFactor Textures
		lProperty = pMaterial->FindProperty(FbxSurfaceMaterial::sReflectionFactor);
		DisplayTextureNames(lProperty, lConnectionString);

		//Update header with material info
		bool lStringOverflow = (lConnectionString.GetLen() + 10 >= MAT_HEADER_LENGTH); // allow for string length and some padding for "%d"
		if (lStringOverflow)
		{
			// Truncate string!
			lConnectionString = lConnectionString.Left(MAT_HEADER_LENGTH - 10);
			lConnectionString = lConnectionString + "...";
		}
		FBXSDK_sprintf(header, MAT_HEADER_LENGTH, lConnectionString.Buffer(), pMatId, l);
		DisplayString(header);
	}

	void FBX_Functions::DisplayMaterialConnections(FbxMesh* pMesh)
	{
		int i, l, lPolygonCount = pMesh->GetPolygonCount();

		char header[MAT_HEADER_LENGTH];

		DisplayString("    Polygons Material Connections");

		//check whether the material maps with only one mesh
		bool lIsAllSame = true;
		for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
		{

			FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
			if (lMaterialElement->GetMappingMode() == FbxGeometryElement::eByPolygon)
			{
				lIsAllSame = false;
				break;
			}
		}

		//For eAllSame mapping type, just out the material and texture mapping info once
		if (lIsAllSame)
		{
			for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
			{

				FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
				if (lMaterialElement->GetMappingMode() == FbxGeometryElement::eAllSame)
				{
					FbxSurfaceMaterial* lMaterial = pMesh->GetNode()->GetMaterial(lMaterialElement->GetIndexArray().GetAt(0));
					int lMatId = lMaterialElement->GetIndexArray().GetAt(0);
					if (lMatId >= 0)
					{
						DisplayInt("        All polygons share the same material in mesh ", l);
						DisplayMaterialTextureConnections(lMaterial, header, lMatId, l);
					}
				}
			}

			//no material
			if (l == 0)
				DisplayString("        no material applied");
		}

		//For eByPolygon mapping type, just out the material and texture mapping info once
		else
		{
			for (i = 0; i < lPolygonCount; i++)
			{
				DisplayInt("        Polygon ", i);

				for (l = 0; l < pMesh->GetElementMaterialCount(); l++)
				{

					FbxGeometryElementMaterial* lMaterialElement = pMesh->GetElementMaterial(l);
					FbxSurfaceMaterial* lMaterial = NULL;
					int lMatId = -1;
					lMaterial = pMesh->GetNode()->GetMaterial(lMaterialElement->GetIndexArray().GetAt(i));
					lMatId = lMaterialElement->GetIndexArray().GetAt(i);

					if (lMatId >= 0)
					{
						DisplayMaterialTextureConnections(lMaterial, header, lMatId, l);
					}
				}
			}
		}
	}

	void FBX_Functions::DisplayMaterialMapping(FbxMesh* pMesh)
	{
		const char* lMappingTypes[] = { "None", "By Control Point", "By Polygon Vertex", "By Polygon", "By Edge", "All Same" };
		const char* lReferenceMode[] = { "Direct", "Index", "Index to Direct" };

		int lMtrlCount = 0;
		FbxNode* lNode = NULL;
		if (pMesh) {
			lNode = pMesh->GetNode();
			if (lNode)
				lMtrlCount = lNode->GetMaterialCount();
		}

		for (int l = 0; l < pMesh->GetElementMaterialCount(); l++)
		{
			FbxGeometryElementMaterial* leMat = pMesh->GetElementMaterial(l);
			if (leMat)
			{
				char header[100];
				FBXSDK_sprintf(header, 100, "    Material Element %d: ", l);
				DisplayString(header);


				DisplayString("           Mapping: ", lMappingTypes[leMat->GetMappingMode()]);
				DisplayString("           ReferenceMode: ", lReferenceMode[leMat->GetReferenceMode()]);

				int lMaterialCount = 0;
				FbxString lString;

				if (leMat->GetReferenceMode() == FbxGeometryElement::eDirect ||
					leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				{
					lMaterialCount = lMtrlCount;
				}

				if (leMat->GetReferenceMode() == FbxGeometryElement::eIndex ||
					leMat->GetReferenceMode() == FbxGeometryElement::eIndexToDirect)
				{
					int i;

					lString = "           Indices: ";

					int lIndexArrayCount = leMat->GetIndexArray().GetCount();
					for (i = 0; i < lIndexArrayCount; i++)
					{
						lString += leMat->GetIndexArray().GetAt(i);

						if (i < lIndexArrayCount - 1)
						{
							lString += ", ";
						}
					}

					lString += "\n";

					FBXSDK_printf(lString);
				}
			}
		}

		DisplayString("");
	}

	void FBX_Functions::DisplayMesh(FbxNode* pNode)
	{
		FbxMesh* lMesh = (FbxMesh*)pNode->GetNodeAttribute();

		// SEt the name of the mesh
		gMesh.pName = pNode->GetName();

		DisplayString("Mesh Name: ", (char *)pNode->GetName());
		DisplayMetaDataConnections(lMesh);
		DisplayControlsPoints(lMesh);
		DisplayPolygons(lMesh);
		DisplayMaterialMapping(lMesh);
		DisplayMaterial(lMesh);
		DisplayTexture(lMesh);
		DisplayMaterialConnections(lMesh);
		DisplayLink(lMesh);
		DisplayShape(lMesh);

		DisplayCache(lMesh);
	}

	void FBX_Functions::DisplayMetaDataConnections(FbxObject* pObject)
	{
		int nbMetaData = pObject->GetSrcObjectCount<FbxObjectMetaData>();
		if (nbMetaData > 0)
			DisplayString("    MetaData connections ");

		for (int i = 0; i < nbMetaData; i++)
		{
			FbxObjectMetaData* metaData = pObject->GetSrcObject<FbxObjectMetaData>(i);
			DisplayString("        Name: ", (char*)metaData->GetName());
		}
	}

	void FBX_Functions::DisplayMaterial(FbxGeometry* pGeometry)
	{
		int lMaterialCount = 0;
		FbxNode* lNode = NULL;
		if (pGeometry) {
			lNode = pGeometry->GetNode();
			if (lNode)
				lMaterialCount = lNode->GetMaterialCount();
		}

		if (lMaterialCount > 0)
		{
			FbxPropertyT<FbxDouble3> lKFbxDouble3;
			FbxPropertyT<FbxDouble> lKFbxDouble1;
			FbxColor theColor;

			for (int lCount = 0; lCount < lMaterialCount; lCount++)
			{
				DisplayInt("        Material ", lCount);

				FbxSurfaceMaterial *lMaterial = lNode->GetMaterial(lCount);

				DisplayString("            Name: \"", (char *)lMaterial->GetName(), "\"");

				//Get the implementation to see if it's a hardware shader.
				const FbxImplementation* lImplementation = GetImplementation(lMaterial, FBXSDK_IMPLEMENTATION_HLSL);
				FbxString lImplemenationType = "HLSL";
				if (!lImplementation)
				{
					lImplementation = GetImplementation(lMaterial, FBXSDK_IMPLEMENTATION_CGFX);
					lImplemenationType = "CGFX";
				}
				if (lImplementation)
				{
					//Now we have a hardware shader, let's read it
					FBXSDK_printf("            Hardware Shader Type: %s\n", lImplemenationType.Buffer());
					const FbxBindingTable* lRootTable = lImplementation->GetRootTable();
					FbxString lFileName = lRootTable->DescAbsoluteURL.Get();
					FbxString lTechniqueName = lRootTable->DescTAG.Get();


					const FbxBindingTable* lTable = lImplementation->GetRootTable();
					size_t lEntryNum = lTable->GetEntryCount();

					for (int i = 0; i < (int)lEntryNum; ++i)
					{
						const FbxBindingTableEntry& lEntry = lTable->GetEntry(i);
						const char* lEntrySrcType = lEntry.GetEntryType(true);
						FbxProperty lFbxProp;


						FbxString lTest = lEntry.GetSource();
						FBXSDK_printf("            Entry: %s\n", lTest.Buffer());


						if (strcmp(FbxPropertyEntryView::sEntryType, lEntrySrcType) == 0)
						{
							lFbxProp = lMaterial->FindPropertyHierarchical(lEntry.GetSource());
							if (!lFbxProp.IsValid())
							{
								lFbxProp = lMaterial->RootProperty.FindHierarchical(lEntry.GetSource());
							}


						}
						else if (strcmp(FbxConstantEntryView::sEntryType, lEntrySrcType) == 0)
						{
							lFbxProp = lImplementation->GetConstants().FindHierarchical(lEntry.GetSource());
						}
						if (lFbxProp.IsValid())
						{
							if (lFbxProp.GetSrcObjectCount<FbxTexture>() > 0)
							{
								//do what you want with the textures
								for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxFileTexture>(); ++j)
								{
									FbxFileTexture *lTex = lFbxProp.GetSrcObject<FbxFileTexture>(j);
									FBXSDK_printf("           File Texture: %s\n", lTex->GetFileName());
								}
								for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxLayeredTexture>(); ++j)
								{
									FbxLayeredTexture *lTex = lFbxProp.GetSrcObject<FbxLayeredTexture>(j);
									FBXSDK_printf("        Layered Texture: %s\n", lTex->GetName());
								}
								for (int j = 0; j < lFbxProp.GetSrcObjectCount<FbxProceduralTexture>(); ++j)
								{
									FbxProceduralTexture *lTex = lFbxProp.GetSrcObject<FbxProceduralTexture>(j);
									FBXSDK_printf("     Procedural Texture: %s\n", lTex->GetName());
								}
							}
							else
							{
								FbxDataType lFbxType = lFbxProp.GetPropertyDataType();
								FbxString blah = lFbxType.GetName();
								if (FbxBoolDT == lFbxType)
								{
									DisplayBool("                Bool: ", lFbxProp.Get<FbxBool>());
								}
								else if (FbxIntDT == lFbxType || FbxEnumDT == lFbxType)
								{
									DisplayInt("                Int: ", lFbxProp.Get<FbxInt>());
								}
								else if (FbxFloatDT == lFbxType)
								{
									DisplayDouble("                Float: ", lFbxProp.Get<FbxFloat>());

								}
								else if (FbxDoubleDT == lFbxType)
								{
									DisplayDouble("                Double: ", lFbxProp.Get<FbxDouble>());
								}
								else if (FbxStringDT == lFbxType
									|| FbxUrlDT == lFbxType
									|| FbxXRefUrlDT == lFbxType)
								{
									DisplayString("                String: ", lFbxProp.Get<FbxString>().Buffer());
								}
								else if (FbxDouble2DT == lFbxType)
								{
									FbxDouble2 lDouble2 = lFbxProp.Get<FbxDouble2>();
									FbxVector2 lVect;
									lVect[0] = lDouble2[0];
									lVect[1] = lDouble2[1];

									Display2DVector("                2D vector: ", lVect);
								}
								else if (FbxDouble3DT == lFbxType || FbxColor3DT == lFbxType)
								{
									FbxDouble3 lDouble3 = lFbxProp.Get<FbxDouble3>();


									FbxVector4 lVect;
									lVect[0] = lDouble3[0];
									lVect[1] = lDouble3[1];
									lVect[2] = lDouble3[2];
									Display3DVector("                3D vector: ", lVect);
								}

								else if (FbxDouble4DT == lFbxType || FbxColor4DT == lFbxType)
								{
									FbxDouble4 lDouble4 = lFbxProp.Get<FbxDouble4>();
									FbxVector4 lVect;
									lVect[0] = lDouble4[0];
									lVect[1] = lDouble4[1];
									lVect[2] = lDouble4[2];
									lVect[3] = lDouble4[3];
									Display4DVector("                4D vector: ", lVect);
								}
								else if (FbxDouble4x4DT == lFbxType)
								{
									FbxDouble4x4 lDouble44 = lFbxProp.Get<FbxDouble4x4>();
									for (int j = 0; j < 4; ++j)
									{

										FbxVector4 lVect;
										lVect[0] = lDouble44[j][0];
										lVect[1] = lDouble44[j][1];
										lVect[2] = lDouble44[j][2];
										lVect[3] = lDouble44[j][3];
										Display4DVector("                4x4D vector: ", lVect);
									}

								}
							}

						}
					}
				}
				else if (lMaterial->GetClassId().Is(FbxSurfacePhong::ClassId))
				{
					// We found a Phong material.  Display its properties.

					// Display the Ambient Color
					lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Ambient;
					theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
					DisplayColor("            Ambient: ", theColor);

					// Display the Diffuse Color
					lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Diffuse;
					theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
					DisplayColor("            Diffuse: ", theColor);

					// Display the Specular Color (unique to Phong materials)
					lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Specular;
					theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
					DisplayColor("            Specular: ", theColor);

					// Display the Emissive Color
					lKFbxDouble3 = ((FbxSurfacePhong *)lMaterial)->Emissive;
					theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
					DisplayColor("            Emissive: ", theColor);

					//Opacity is Transparency factor now
					lKFbxDouble1 = ((FbxSurfacePhong *)lMaterial)->TransparencyFactor;
					DisplayDouble("            Opacity: ", 1.0 - lKFbxDouble1.Get());

					// Display the Shininess
					lKFbxDouble1 = ((FbxSurfacePhong *)lMaterial)->Shininess;
					DisplayDouble("            Shininess: ", lKFbxDouble1.Get());

					// Display the Reflectivity
					lKFbxDouble1 = ((FbxSurfacePhong *)lMaterial)->ReflectionFactor;
					DisplayDouble("            Reflectivity: ", lKFbxDouble1.Get());
				}
				else if (lMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId))
				{
					// We found a Lambert material. Display its properties.
					// Display the Ambient Color
					lKFbxDouble3 = ((FbxSurfaceLambert *)lMaterial)->Ambient;
					theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
					DisplayColor("            Ambient: ", theColor);

					// Display the Diffuse Color
					lKFbxDouble3 = ((FbxSurfaceLambert *)lMaterial)->Diffuse;
					theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
					DisplayColor("            Diffuse: ", theColor);

					// Display the Emissive
					lKFbxDouble3 = ((FbxSurfaceLambert *)lMaterial)->Emissive;
					theColor.Set(lKFbxDouble3.Get()[0], lKFbxDouble3.Get()[1], lKFbxDouble3.Get()[2]);
					DisplayColor("            Emissive: ", theColor);

					// Display the Opacity
					lKFbxDouble1 = ((FbxSurfaceLambert *)lMaterial)->TransparencyFactor;
					DisplayDouble("            Opacity: ", 1.0 - lKFbxDouble1.Get());
				}
				else
					DisplayString("Unknown type of Material");

				FbxPropertyT<FbxString> lString;
				lString = lMaterial->ShadingModel;
				DisplayString("            Shading Model: ", lString.Get().Buffer());
				DisplayString("");
			}
		}
	}

	void FBX_Functions::DisplayTexture(FbxGeometry* pGeometry)
	{
		int lMaterialIndex;
		FbxProperty lProperty;
		if (pGeometry->GetNode() == NULL)
			return;
		int lNbMat = pGeometry->GetNode()->GetSrcObjectCount<FbxSurfaceMaterial>();
		for (lMaterialIndex = 0; lMaterialIndex < lNbMat; lMaterialIndex++) {
			FbxSurfaceMaterial *lMaterial = pGeometry->GetNode()->GetSrcObject<FbxSurfaceMaterial>(lMaterialIndex);
			bool lDisplayHeader = true;

			//go through all the possible textures
			if (lMaterial) {

				int lTextureIndex;
				FBXSDK_FOR_EACH_TEXTURE(lTextureIndex)
				{
					lProperty = lMaterial->FindProperty(FbxLayerElement::sTextureChannelNames[lTextureIndex]);
					FindAndDisplayTextureInfoByProperty(lProperty, lDisplayHeader, lMaterialIndex);
				}

			}//end if(lMaterial)

		}// end for lMaterialIndex     
	}

	void FBX_Functions::FindAndDisplayTextureInfoByProperty(FbxProperty pProperty, bool& pDisplayHeader, int pMaterialIndex) {

		if (pProperty.IsValid())
		{
			int lTextureCount = pProperty.GetSrcObjectCount<FbxTexture>();

			for (int j = 0; j < lTextureCount; ++j)
			{
				//Here we have to check if it's layeredtextures, or just textures:
				FbxLayeredTexture *lLayeredTexture = pProperty.GetSrcObject<FbxLayeredTexture>(j);
				if (lLayeredTexture)
				{
					DisplayInt("    Layered Texture: ", j);
					int lNbTextures = lLayeredTexture->GetSrcObjectCount<FbxTexture>();
					for (int k = 0; k < lNbTextures; ++k)
					{
						FbxTexture* lTexture = lLayeredTexture->GetSrcObject<FbxTexture>(k);
						if (lTexture)
						{

							if (pDisplayHeader) {
								DisplayInt("    Textures connected to Material ", pMaterialIndex);
								pDisplayHeader = false;
							}

							//NOTE the blend mode is ALWAYS on the LayeredTexture and NOT the one on the texture.
							//Why is that?  because one texture can be shared on different layered textures and might
							//have different blend modes.

							FbxLayeredTexture::EBlendMode lBlendMode;
							lLayeredTexture->GetTextureBlendMode(k, lBlendMode);
							DisplayString("    Textures for ", pProperty.GetName());
							DisplayInt("        Texture ", k);
							DisplayTextureInfo(lTexture, (int)lBlendMode);
						}

					}
				}
				else
				{
					//no layered texture simply get on the property
					FbxTexture* lTexture = pProperty.GetSrcObject<FbxTexture>(j);
					if (lTexture)
					{
						//display connected Material header only at the first time
						if (pDisplayHeader) {
							DisplayInt("    Textures connected to Material ", pMaterialIndex);
							pDisplayHeader = false;
						}

						DisplayString("    Textures for ", pProperty.GetName());
						DisplayInt("        Texture ", j);
						DisplayTextureInfo(lTexture, -1);
					}
				}
			}
		}//end if pProperty

	}

	void FBX_Functions::DisplayTextureInfo(FbxTexture* pTexture, int pBlendMode)
	{
		FbxFileTexture *lFileTexture = FbxCast<FbxFileTexture>(pTexture);
		FbxProceduralTexture *lProceduralTexture = FbxCast<FbxProceduralTexture>(pTexture);

		DisplayString("            Name: \"", (char *)pTexture->GetName(), "\"");
		if (lFileTexture)
		{
			DisplayString("            Type: File Texture");
			DisplayString("            File Name: \"", (char *)lFileTexture->GetFileName(), "\"");
		}
		else if (lProceduralTexture)
		{
			DisplayString("            Type: Procedural Texture");
		}
		DisplayDouble("            Scale U: ", pTexture->GetScaleU());
		DisplayDouble("            Scale V: ", pTexture->GetScaleV());
		DisplayDouble("            Translation U: ", pTexture->GetTranslationU());
		DisplayDouble("            Translation V: ", pTexture->GetTranslationV());
		DisplayBool("            Swap UV: ", pTexture->GetSwapUV());
		DisplayDouble("            Rotation U: ", pTexture->GetRotationU());
		DisplayDouble("            Rotation V: ", pTexture->GetRotationV());
		DisplayDouble("            Rotation W: ", pTexture->GetRotationW());

		const char* lAlphaSources[] = { "None", "RGB Intensity", "Black" };

		DisplayString("            Alpha Source: ", lAlphaSources[pTexture->GetAlphaSource()]);
		DisplayDouble("            Cropping Left: ", pTexture->GetCroppingLeft());
		DisplayDouble("            Cropping Top: ", pTexture->GetCroppingTop());
		DisplayDouble("            Cropping Right: ", pTexture->GetCroppingRight());
		DisplayDouble("            Cropping Bottom: ", pTexture->GetCroppingBottom());

		const char* lMappingTypes[] = { "Null", "Planar", "Spherical", "Cylindrical",
			"Box", "Face", "UV", "Environment" };

		DisplayString("            Mapping Type: ", lMappingTypes[pTexture->GetMappingType()]);

		if (pTexture->GetMappingType() == FbxTexture::ePlanar)
		{
			const char* lPlanarMappingNormals[] = { "X", "Y", "Z" };

			DisplayString("            Planar Mapping Normal: ", lPlanarMappingNormals[pTexture->GetPlanarMappingNormal()]);
		}

		const char* lBlendModes[] = { "Translucent", "Add", "Modulate", "Modulate2" };
		if (pBlendMode >= 0)
			DisplayString("            Blend Mode: ", lBlendModes[pBlendMode]);
		DisplayDouble("            Alpha: ", pTexture->GetDefaultAlpha());

		if (lFileTexture)
		{
			const char* lMaterialUses[] = { "Model Material", "Default Material" };
			DisplayString("            Material Use: ", lMaterialUses[lFileTexture->GetMaterialUse()]);
		}

		const char* pTextureUses[] = { "Standard", "Shadow Map", "Light Map",
			"Spherical Reflexion Map", "Sphere Reflexion Map", "Bump Normal Map" };

		DisplayString("            Texture Use: ", pTextureUses[pTexture->GetTextureUse()]);
		DisplayString("");

	}

	void FBX_Functions::DisplayLink(FbxGeometry* pGeometry)
	{
		//Display cluster now

		//int i, lLinkCount;
		//FbxCluster* lLink;

		int i, j;
		int lSkinCount = 0;
		int lClusterCount = 0;
		FbxCluster* lCluster;

		lSkinCount = pGeometry->GetDeformerCount(FbxDeformer::eSkin);



		//lLinkCount = pGeometry->GetLinkCount();
		for (i = 0; i != lSkinCount; ++i)
		{
			lClusterCount = ((FbxSkin *)pGeometry->GetDeformer(i, FbxDeformer::eSkin))->GetClusterCount();
			for (j = 0; j != lClusterCount; ++j)
			{
				DisplayInt("    Cluster ", i);

				lCluster = ((FbxSkin *)pGeometry->GetDeformer(i, FbxDeformer::eSkin))->GetCluster(j);
				//lLink = pGeometry->GetLink(i);    

				const char* lClusterModes[] = { "Normalize", "Additive", "Total1" };

				DisplayString("    Mode: ", lClusterModes[lCluster->GetLinkMode()]);

				if (lCluster->GetLink() != NULL)
				{
					DisplayString("        Name: ", (char *)lCluster->GetLink()->GetName());
				}

				FbxString lString1 = "        Link Indices: ";
				FbxString lString2 = "        Weight Values: ";

				int k, lIndexCount = lCluster->GetControlPointIndicesCount();
				int* lIndices = lCluster->GetControlPointIndices();
				double* lWeights = lCluster->GetControlPointWeights();

				for (k = 0; k < lIndexCount; k++)
				{
					lString1 += lIndices[k];
					lString2 += (float)lWeights[k];

					if (k < lIndexCount - 1)
					{
						lString1 += ", ";
						lString2 += ", ";
					}
				}

				lString1 += "\n";
				lString2 += "\n";

				FBXSDK_printf(lString1);
				FBXSDK_printf(lString2);

				DisplayString("");

				FbxAMatrix lMatrix;

				lMatrix = lCluster->GetTransformMatrix(lMatrix);
				Display3DVector("        Transform Translation: ", lMatrix.GetT());
				Display3DVector("        Transform Rotation: ", lMatrix.GetR());
				Display3DVector("        Transform Scaling: ", lMatrix.GetS());

				lMatrix = lCluster->GetTransformLinkMatrix(lMatrix);
				Display3DVector("        Transform Link Translation: ", lMatrix.GetT());
				Display3DVector("        Transform Link Rotation: ", lMatrix.GetR());
				Display3DVector("        Transform Link Scaling: ", lMatrix.GetS());

				if (lCluster->GetAssociateModel() != NULL)
				{
					lMatrix = lCluster->GetTransformAssociateModelMatrix(lMatrix);
					DisplayString("        Associate Model: ", (char *)lCluster->GetAssociateModel()->GetName());
					Display3DVector("        Associate Model Translation: ", lMatrix.GetT());
					Display3DVector("        Associate Model Rotation: ", lMatrix.GetR());
					Display3DVector("        Associate Model Scaling: ", lMatrix.GetS());
				}

				DisplayString("");
			}
		}
	}

	void FBX_Functions::DisplayShape(FbxGeometry* pGeometry)
	{
		int lBlendShapeCount, lBlendShapeChannelCount, lTargetShapeCount;
		FbxBlendShape* lBlendShape;
		FbxBlendShapeChannel* lBlendShapeChannel;
		FbxShape* lShape;

		lBlendShapeCount = pGeometry->GetDeformerCount(FbxDeformer::eBlendShape);

		for (int lBlendShapeIndex = 0; lBlendShapeIndex < lBlendShapeCount; ++lBlendShapeIndex)
		{
			lBlendShape = (FbxBlendShape*)pGeometry->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);
			DisplayString("    BlendShape ", (char *)lBlendShape->GetName());

			lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
			for (int lBlendShapeChannelIndex = 0; lBlendShapeChannelIndex < lBlendShapeChannelCount; ++lBlendShapeChannelIndex)
			{
				lBlendShapeChannel = lBlendShape->GetBlendShapeChannel(lBlendShapeChannelIndex);
				DisplayString("    BlendShapeChannel ", (char *)lBlendShapeChannel->GetName());
				DisplayDouble("    Default Deform Value: ", lBlendShapeChannel->DeformPercent.Get());

				lTargetShapeCount = lBlendShapeChannel->GetTargetShapeCount();
				for (int lTargetShapeIndex = 0; lTargetShapeIndex < lTargetShapeCount; ++lTargetShapeIndex)
				{
					lShape = lBlendShapeChannel->GetTargetShape(lTargetShapeIndex);
					DisplayString("    TargetShape ", (char *)lShape->GetName());

					int j, lControlPointsCount = lShape->GetControlPointsCount();
					FbxVector4* lControlPoints = lShape->GetControlPoints();
					FbxLayerElementArrayTemplate<FbxVector4>* lNormals = NULL;
					bool lStatus = lShape->GetNormals(&lNormals);

					for (j = 0; j < lControlPointsCount; j++)
					{
						DisplayInt("        Control Point ", j);
						Display3DVector("            Coordinates: ", lControlPoints[j]);

						if (lStatus && lNormals && lNormals->GetCount() == lControlPointsCount)
						{
							Display3DVector("            Normal Vector: ", lNormals->GetAt(j));
						}
					}

					DisplayString("");
				}
			}
		}
	}

	void FBX_Functions::DisplayCache(FbxGeometry* pGeometry)
	{
		int lVertexCacheDeformerCount = pGeometry->GetDeformerCount(FbxDeformer::eVertexCache);

		for (int i = 0; i < lVertexCacheDeformerCount; ++i)
		{
			FbxVertexCacheDeformer* lDeformer = static_cast<FbxVertexCacheDeformer*>(pGeometry->GetDeformer(i, FbxDeformer::eVertexCache));
			if (!lDeformer) continue;

			FbxCache* lCache = lDeformer->GetCache();
			if (!lCache) continue;

			if (lCache->OpenFileForRead())
			{
				DisplayString("    Vertex Cache");
				int lChannelIndex = lCache->GetChannelIndex(lDeformer->Channel.Get());
				// skip normal channel
				if (lChannelIndex < 0)
					continue;

				FbxString lChnlName, lChnlInterp;

				FbxCache::EMCDataType lChnlType;
				FbxTime start, stop, rate;
				FbxCache::EMCSamplingType lChnlSampling;
				unsigned int lChnlSampleCount, lDataCount;

				lCache->GetChannelName(lChannelIndex, lChnlName);
				DisplayString("        Channel Name: ", lChnlName.Buffer());
				lCache->GetChannelDataType(lChannelIndex, lChnlType);
				switch (lChnlType)
				{
				case FbxCache::eUnknownData:
					DisplayString("        Channel Type: Unknown Data"); break;
				case FbxCache::eDouble:
					DisplayString("        Channel Type: Double"); break;
				case FbxCache::eDoubleArray:
					DisplayString("        Channel Type: Double Array"); break;
				case FbxCache::eDoubleVectorArray:
					DisplayString("        Channel Type: Double Vector Array"); break;
				case FbxCache::eInt32Array:
					DisplayString("        Channel Type: Int32 Array"); break;
				case FbxCache::eFloatArray:
					DisplayString("        Channel Type: Float Array"); break;
				case FbxCache::eFloatVectorArray:
					DisplayString("        Channel Type: Float Vector Array"); break;
				}
				lCache->GetChannelInterpretation(lChannelIndex, lChnlInterp);
				DisplayString("        Channel Interpretation: ", lChnlInterp.Buffer());
				lCache->GetChannelSamplingType(lChannelIndex, lChnlSampling);
				DisplayInt("        Channel Sampling Type: ", lChnlSampling);
				lCache->GetAnimationRange(lChannelIndex, start, stop);
				lCache->GetChannelSamplingRate(lChannelIndex, rate);
				lCache->GetChannelSampleCount(lChannelIndex, lChnlSampleCount);
				DisplayInt("        Channel Sample Count: ", lChnlSampleCount);

				// Only display cache data if the data type is float vector array
				if (lChnlType != FbxCache::eFloatVectorArray)
					continue;

				if (lChnlInterp == "normals")
					DisplayString("        Normal Cache Data");
				else
					DisplayString("        Points Cache Data");
				float* lBuffer = NULL;
				unsigned int lBufferSize = 0;
				int lFrame = 0;
				for (FbxTime t = start; t <= stop; t += rate)
				{
					DisplayInt("            Frame ", lFrame);
					lCache->GetChannelPointCount(lChannelIndex, t, lDataCount);
					if (lBuffer == NULL)
					{
						lBuffer = new float[lDataCount * 3];
						lBufferSize = lDataCount * 3;
					}
					else if (lBufferSize < lDataCount * 3)
					{
						delete[] lBuffer;
						lBuffer = new float[lDataCount * 3];
						lBufferSize = lDataCount * 3;
					}
					else
						memset(lBuffer, 0, lBufferSize * sizeof(float));

					lCache->Read(lChannelIndex, t, lBuffer, lDataCount);
					if (lChnlInterp == "normals")
					{
						// display normals cache data
						// the normal data is per-polygon per-vertex. we can get the polygon vertex index
						// from the index array of polygon vertex
						FbxMesh* lMesh = (FbxMesh*)pGeometry;

						if (lMesh == NULL)
						{
							// Only Mesh can have normal cache data
							continue;
						}

						DisplayInt("                Normal Count ", lDataCount);
						int pi, j, lPolygonCount = lMesh->GetPolygonCount();
						unsigned lNormalIndex = 0;
						for (pi = 0; pi < lPolygonCount && lNormalIndex + 2 < lDataCount * 3; pi++)
						{
							DisplayInt("                    Polygon ", pi);
							DisplayString("                    Normals for Each Polygon Vertex: ");
							int lPolygonSize = lMesh->GetPolygonSize(pi);
							for (j = 0; j < lPolygonSize && lNormalIndex + 2 < lDataCount * 3; j++)
							{
								FbxVector4 normal(lBuffer[lNormalIndex], lBuffer[lNormalIndex + 1], lBuffer[lNormalIndex + 2]);
								Display3DVector("                       Normal Cache Data  ", normal);
								lNormalIndex += 3;
							}
						}
					}
					else
					{
						DisplayInt("               Points Count: ", lDataCount);
						for (unsigned int j = 0; j < lDataCount * 3; j = j + 3)
						{
							FbxVector4 points(lBuffer[j], lBuffer[j + 1], lBuffer[j + 2]);
							Display3DVector("                   Points Cache Data: ", points);
						}
					}

					lFrame++;
				}

				if (lBuffer != NULL)
				{
					delete[] lBuffer;
					lBuffer = NULL;
				}

				lCache->CloseFile();
			}
		}
	}

	void FBX_Functions::DisplayAnimation(FbxScene* pScene)
	{
		int i;
		for (i = 0; i < pScene->GetSrcObjectCount<FbxAnimStack>(); i++)
		{
			FbxAnimStack* lAnimStack = pScene->GetSrcObject<FbxAnimStack>(i);

			FbxString lOutputString = "Animation Stack Name: ";
			lOutputString += lAnimStack->GetName();
			lOutputString += "\n\n";
			FBXSDK_printf(lOutputString);

			DisplayAnimation(lAnimStack, pScene->GetRootNode(), true);
			DisplayAnimation(lAnimStack, pScene->GetRootNode());
		}
	}

	void FBX_Functions::DisplayAnimation(FbxAnimStack* pAnimStack, FbxNode* pNode, bool isSwitcher)
	{
		int l;
		int nbAnimLayers = pAnimStack->GetMemberCount<FbxAnimLayer>();
		FbxString lOutputString;

		lOutputString = "Animation stack contains ";
		lOutputString += nbAnimLayers;
		lOutputString += " Animation Layer(s)\n";
		FBXSDK_printf(lOutputString);

		for (l = 0; l < nbAnimLayers; l++)
		{
			FbxAnimLayer* lAnimLayer = pAnimStack->GetMember<FbxAnimLayer>(l);

			lOutputString = "AnimLayer ";
			lOutputString += l;
			lOutputString += "\n";
			FBXSDK_printf(lOutputString);

			DisplayAnimation(lAnimLayer, pNode, isSwitcher);
		}
	}

	void FBX_Functions::DisplayAnimation(FbxAnimLayer* pAnimLayer, FbxNode* pNode, bool isSwitcher)
	{
		int lModelCount;
		FbxString lOutputString;

		lOutputString = "     Node Name: ";
		lOutputString += pNode->GetName();
		lOutputString += "\n\n";
		FBXSDK_printf(lOutputString);

		DisplayChannels(pNode, pAnimLayer, isSwitcher);
		FBXSDK_printf("\n");

		for (lModelCount = 0; lModelCount < pNode->GetChildCount(); lModelCount++)
		{
			DisplayAnimation(pAnimLayer, pNode->GetChild(lModelCount), isSwitcher);
		}
	}

	void FBX_Functions::DisplayChannels(FbxNode* pNode, FbxAnimLayer* pAnimLayer, bool isSwitcher)
	{
		FbxAnimCurve* lAnimCurve = NULL;
		std::string lNodeName = pNode->GetName();

		int jointIndex = 0;
		for (unsigned int i = 0; i < gSkeleton.pJoints.size(); i++)
		{
			if (gSkeleton.pJoints[i].pName == lNodeName)
			{
				jointIndex = i;
				break;
			}

			// If it's not in the list make the index -1
			// Make check where i create nodes
			else
			{
				jointIndex = -1;
			}
		}
		// Display general curves.
		if (!isSwitcher)
		{
			lAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
			if (lAnimCurve)
			{
				FBXSDK_printf("        TX\n");
				//DisplayCurveKeys(lAnimCurve);
				DisplayCurveKeys(lAnimCurve, pNode, Vertex_Part::TRANSLATION, Vertex_Part_Data::X, jointIndex);
			}
			lAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
			if (lAnimCurve)
			{
				FBXSDK_printf("        TY\n");
				DisplayCurveKeys(lAnimCurve, pNode, Vertex_Part::TRANSLATION, Vertex_Part_Data::Y, jointIndex);
			}
			lAnimCurve = pNode->LclTranslation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
			if (lAnimCurve)
			{
				FBXSDK_printf("        TZ\n");
				DisplayCurveKeys(lAnimCurve, pNode, Vertex_Part::TRANSLATION, Vertex_Part_Data::Z, jointIndex);
			}

			lAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
			if (lAnimCurve)
			{
				FBXSDK_printf("        RX\n");
				DisplayCurveKeys(lAnimCurve, pNode, Vertex_Part::ROTATION, Vertex_Part_Data::X, jointIndex);
			}
			lAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
			if (lAnimCurve)
			{
				FBXSDK_printf("        RY\n");
				DisplayCurveKeys(lAnimCurve, pNode, Vertex_Part::ROTATION, Vertex_Part_Data::Y, jointIndex);
			}
			lAnimCurve = pNode->LclRotation.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
			if (lAnimCurve)
			{
				FBXSDK_printf("        RZ\n");
				DisplayCurveKeys(lAnimCurve, pNode, Vertex_Part::ROTATION, Vertex_Part_Data::Z, jointIndex);
			}

			lAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_X);
			if (lAnimCurve)
			{
				FBXSDK_printf("        SX\n");
				DisplayCurveKeys(lAnimCurve, pNode, Vertex_Part::SCALE, Vertex_Part_Data::X, jointIndex);
			}
			lAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Y);
			if (lAnimCurve)
			{
				FBXSDK_printf("        SY\n");
				DisplayCurveKeys(lAnimCurve, pNode, Vertex_Part::SCALE, Vertex_Part_Data::Y, jointIndex);
			}
			lAnimCurve = pNode->LclScaling.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COMPONENT_Z);
			if (lAnimCurve)
			{
				FBXSDK_printf("        SZ\n");
				DisplayCurveKeys(lAnimCurve, pNode, Vertex_Part::SCALE, Vertex_Part_Data::Z, jointIndex);
			}
		}

		// Display curves specific to a light or marker.
		FbxNodeAttribute* lNodeAttribute = pNode->GetNodeAttribute();

		if (lNodeAttribute)
		{
			lAnimCurve = lNodeAttribute->Color.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COLOR_RED);
			if (lAnimCurve)
			{
				FBXSDK_printf("        Red\n");
				DisplayCurveKeys(lAnimCurve);
			}
			lAnimCurve = lNodeAttribute->Color.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COLOR_GREEN);
			if (lAnimCurve)
			{
				FBXSDK_printf("        Green\n");
				DisplayCurveKeys(lAnimCurve);
			}
			lAnimCurve = lNodeAttribute->Color.GetCurve(pAnimLayer, FBXSDK_CURVENODE_COLOR_BLUE);
			if (lAnimCurve)
			{
				FBXSDK_printf("        Blue\n");
				DisplayCurveKeys(lAnimCurve);
			}

			// Display curves specific to a light.
			FbxLight* light = pNode->GetLight();
			if (light)
			{
				lAnimCurve = light->Intensity.GetCurve(pAnimLayer);
				if (lAnimCurve)
				{
					FBXSDK_printf("        Intensity\n");
					DisplayCurveKeys(lAnimCurve);
				}

				lAnimCurve = light->OuterAngle.GetCurve(pAnimLayer);
				if (lAnimCurve)
				{
					FBXSDK_printf("        Outer Angle\n");
					DisplayCurveKeys(lAnimCurve);
				}

				lAnimCurve = light->Fog.GetCurve(pAnimLayer);
				if (lAnimCurve)
				{
					FBXSDK_printf("        Fog\n");
					DisplayCurveKeys(lAnimCurve);
				}
			}

			// Display curves specific to a camera.
			FbxCamera* camera = pNode->GetCamera();
			if (camera)
			{
				lAnimCurve = camera->FieldOfView.GetCurve(pAnimLayer);
				if (lAnimCurve)
				{
					FBXSDK_printf("        Field of View\n");
					DisplayCurveKeys(lAnimCurve);
				}

				lAnimCurve = camera->FieldOfViewX.GetCurve(pAnimLayer);
				if (lAnimCurve)
				{
					FBXSDK_printf("        Field of View X\n");
					DisplayCurveKeys(lAnimCurve);
				}

				lAnimCurve = camera->FieldOfViewY.GetCurve(pAnimLayer);
				if (lAnimCurve)
				{
					FBXSDK_printf("        Field of View Y\n");
					DisplayCurveKeys(lAnimCurve);
				}

				lAnimCurve = camera->OpticalCenterX.GetCurve(pAnimLayer);
				if (lAnimCurve)
				{
					FBXSDK_printf("        Optical Center X\n");
					DisplayCurveKeys(lAnimCurve);
				}

				lAnimCurve = camera->OpticalCenterY.GetCurve(pAnimLayer);
				if (lAnimCurve)
				{
					FBXSDK_printf("        Optical Center Y\n");
					DisplayCurveKeys(lAnimCurve);
				}

				lAnimCurve = camera->Roll.GetCurve(pAnimLayer);
				if (lAnimCurve)
				{
					FBXSDK_printf("        Roll\n");
					DisplayCurveKeys(lAnimCurve);
				}
			}

			// Display curves specific to a geometry.
			if (lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eMesh ||
				lNodeAttribute->GetAttributeType() == FbxNodeAttribute::eNurbs ||
				lNodeAttribute->GetAttributeType() == FbxNodeAttribute::ePatch)
			{
				FbxGeometry* lGeometry = (FbxGeometry*)lNodeAttribute;

				int lBlendShapeDeformerCount = lGeometry->GetDeformerCount(FbxDeformer::eBlendShape);
				for (int lBlendShapeIndex = 0; lBlendShapeIndex < lBlendShapeDeformerCount; ++lBlendShapeIndex)
				{
					FbxBlendShape* lBlendShape = (FbxBlendShape*)lGeometry->GetDeformer(lBlendShapeIndex, FbxDeformer::eBlendShape);

					int lBlendShapeChannelCount = lBlendShape->GetBlendShapeChannelCount();
					for (int lChannelIndex = 0; lChannelIndex < lBlendShapeChannelCount; ++lChannelIndex)
					{
						FbxBlendShapeChannel* lChannel = lBlendShape->GetBlendShapeChannel(lChannelIndex);
						const char* lChannelName = lChannel->GetName();

						lAnimCurve = lGeometry->GetShapeChannel(lBlendShapeIndex, lChannelIndex, pAnimLayer, true);
						if (lAnimCurve)
						{
							FBXSDK_printf("        Shape %s\n", lChannelName);
							DisplayCurveKeys(lAnimCurve);
						}
					}
				}
			}
		}

		// Display curves specific to properties
		FbxProperty lProperty = pNode->GetFirstProperty();
		while (lProperty.IsValid())
		{
			if (lProperty.GetFlag(FbxPropertyFlags::eUserDefined))
			{
				FbxString lFbxFCurveNodeName = lProperty.GetName();
				FbxAnimCurveNode* lCurveNode = lProperty.GetCurveNode(pAnimLayer);

				if (!lCurveNode) {
					lProperty = pNode->GetNextProperty(lProperty);
					continue;
				}

				FbxDataType lDataType = lProperty.GetPropertyDataType();
				if (lDataType.GetType() == eFbxBool || lDataType.GetType() == eFbxDouble || lDataType.GetType() == eFbxFloat || lDataType.GetType() == eFbxInt)
				{
					FbxString lMessage;

					lMessage = "        Property ";
					lMessage += lProperty.GetName();
					if (lProperty.GetLabel().GetLen() > 0)
					{
						lMessage += " (Label: ";
						lMessage += lProperty.GetLabel();
						lMessage += ")";
					};

					DisplayString(lMessage.Buffer());

					for (int c = 0; c < lCurveNode->GetCurveCount(0U); c++)
					{
						lAnimCurve = lCurveNode->GetCurve(0U, c);
						if (lAnimCurve)
							DisplayCurveKeys(lAnimCurve);
					}
				}
				else if (lDataType.GetType() == eFbxDouble3 || lDataType.GetType() == eFbxDouble4 || lDataType.Is(FbxColor3DT) || lDataType.Is(FbxColor4DT))
				{
					char* lComponentName1 = (lDataType.Is(FbxColor3DT) || lDataType.Is(FbxColor4DT)) ? (char*)FBXSDK_CURVENODE_COLOR_RED : (char*)"X";
					char* lComponentName2 = (lDataType.Is(FbxColor3DT) || lDataType.Is(FbxColor4DT)) ? (char*)FBXSDK_CURVENODE_COLOR_GREEN : (char*)"Y";
					char* lComponentName3 = (lDataType.Is(FbxColor3DT) || lDataType.Is(FbxColor4DT)) ? (char*)FBXSDK_CURVENODE_COLOR_BLUE : (char*)"Z";
					FbxString      lMessage;

					lMessage = "        Property ";
					lMessage += lProperty.GetName();
					if (lProperty.GetLabel().GetLen() > 0)
					{
						lMessage += " (Label: ";
						lMessage += lProperty.GetLabel();
						lMessage += ")";
					}
					DisplayString(lMessage.Buffer());

					for (int c = 0; c < lCurveNode->GetCurveCount(0U); c++)
					{
						lAnimCurve = lCurveNode->GetCurve(0U, c);
						if (lAnimCurve)
						{
							DisplayString("        Component ", lComponentName1);
							DisplayCurveKeys(lAnimCurve);
						}
					}

					for (int c = 0; c < lCurveNode->GetCurveCount(1U); c++)
					{
						lAnimCurve = lCurveNode->GetCurve(1U, c);
						if (lAnimCurve)
						{
							DisplayString("        Component ", lComponentName2);
							DisplayCurveKeys(lAnimCurve);
						}
					}

					for (int c = 0; c < lCurveNode->GetCurveCount(2U); c++)
					{
						lAnimCurve = lCurveNode->GetCurve(2U, c);
						if (lAnimCurve)
						{
							DisplayString("        Component ", lComponentName3);
							DisplayCurveKeys(lAnimCurve);
						}
					}
				}
				else if (lDataType.GetType() == eFbxEnum)
				{
					FbxString lMessage;

					lMessage = "        Property ";
					lMessage += lProperty.GetName();
					if (lProperty.GetLabel().GetLen() > 0)
					{
						lMessage += " (Label: ";
						lMessage += lProperty.GetLabel();
						lMessage += ")";
					};
					DisplayString(lMessage.Buffer());

					for (int c = 0; c < lCurveNode->GetCurveCount(0U); c++)
					{
						lAnimCurve = lCurveNode->GetCurve(0U, c);
						if (lAnimCurve)
							DisplayListCurveKeys(lAnimCurve, &lProperty);
					}
				}
			}

			lProperty = pNode->GetNextProperty(lProperty);
		} // while


	}

	static int InterpolationFlagToIndex(int flags)
	{
		if ((flags & FbxAnimCurveDef::eInterpolationConstant) == FbxAnimCurveDef::eInterpolationConstant) return 1;
		if ((flags & FbxAnimCurveDef::eInterpolationLinear) == FbxAnimCurveDef::eInterpolationLinear) return 2;
		if ((flags & FbxAnimCurveDef::eInterpolationCubic) == FbxAnimCurveDef::eInterpolationCubic) return 3;
		return 0;
	}

	static int ConstantmodeFlagToIndex(int flags)
	{
		if ((flags & FbxAnimCurveDef::eConstantStandard) == FbxAnimCurveDef::eConstantStandard) return 1;
		if ((flags & FbxAnimCurveDef::eConstantNext) == FbxAnimCurveDef::eConstantNext) return 2;
		return 0;
	}

	static int TangentmodeFlagToIndex(int flags)
	{
		if ((flags & FbxAnimCurveDef::eTangentAuto) == FbxAnimCurveDef::eTangentAuto) return 1;
		if ((flags & FbxAnimCurveDef::eTangentAutoBreak) == FbxAnimCurveDef::eTangentAutoBreak) return 2;
		if ((flags & FbxAnimCurveDef::eTangentTCB) == FbxAnimCurveDef::eTangentTCB) return 3;
		if ((flags & FbxAnimCurveDef::eTangentUser) == FbxAnimCurveDef::eTangentUser) return 4;
		if ((flags & FbxAnimCurveDef::eTangentGenericBreak) == FbxAnimCurveDef::eTangentGenericBreak) return 5;
		if ((flags & FbxAnimCurveDef::eTangentBreak) == FbxAnimCurveDef::eTangentBreak) return 6;
		return 0;
	}

	static int TangentweightFlagToIndex(int flags)
	{
		if ((flags & FbxAnimCurveDef::eWeightedNone) == FbxAnimCurveDef::eWeightedNone) return 1;
		if ((flags & FbxAnimCurveDef::eWeightedRight) == FbxAnimCurveDef::eWeightedRight) return 2;
		if ((flags & FbxAnimCurveDef::eWeightedNextLeft) == FbxAnimCurveDef::eWeightedNextLeft) return 3;
		return 0;
	}

	static int TangentVelocityFlagToIndex(int flags)
	{
		if ((flags & FbxAnimCurveDef::eVelocityNone) == FbxAnimCurveDef::eVelocityNone) return 1;
		if ((flags & FbxAnimCurveDef::eVelocityRight) == FbxAnimCurveDef::eVelocityRight) return 2;
		if ((flags & FbxAnimCurveDef::eVelocityNextLeft) == FbxAnimCurveDef::eVelocityNextLeft) return 3;
		return 0;
	}

	void FBX_Functions::DisplayCurveKeys(FbxAnimCurve* pCurve)
	{
		static const char* interpolation[] = { "?", "constant", "linear", "cubic" };
		static const char* constantMode[] = { "?", "Standard", "Next" };
		static const char* cubicMode[] = { "?", "Auto", "Auto break", "Tcb", "User", "Break", "User break" };
		static const char* tangentWVMode[] = { "?", "None", "Right", "Next left" };

		FbxTime   lKeyTime;
		float   lKeyValue;
		char    lTimeString[256];
		FbxString lOutputString;
		int     lCount;

		int lKeyCount = pCurve->KeyGetCount();

		// Keyframe that will be filled out and passed into the joint
		Keyframe_Info lKeyframe_Info;

		for (lCount = 0; lCount < lKeyCount; lCount++)
		{
			lKeyValue = static_cast<float>(pCurve->KeyGetValue(lCount));
			lKeyTime = pCurve->KeyGetTime(lCount);

			lOutputString = "            Key Time: ";
			lOutputString += lKeyTime.GetTimeString(lTimeString, FbxUShort(256));
			lOutputString += ".... Key Value: ";
			lOutputString += lKeyValue;
			lOutputString += " [ ";
			lOutputString += interpolation[InterpolationFlagToIndex(pCurve->KeyGetInterpolation(lCount))];
			if ((pCurve->KeyGetInterpolation(lCount)&FbxAnimCurveDef::eInterpolationConstant) == FbxAnimCurveDef::eInterpolationConstant)
			{
				lOutputString += " | ";
				lOutputString += constantMode[ConstantmodeFlagToIndex(pCurve->KeyGetConstantMode(lCount))];
			}
			else if ((pCurve->KeyGetInterpolation(lCount)&FbxAnimCurveDef::eInterpolationCubic) == FbxAnimCurveDef::eInterpolationCubic)
			{
				lOutputString += " | ";
				lOutputString += cubicMode[TangentmodeFlagToIndex(pCurve->KeyGetTangentMode(lCount))];
				lOutputString += " | ";
				lOutputString += tangentWVMode[TangentweightFlagToIndex(pCurve->KeyGet(lCount).GetTangentWeightMode())];
				lOutputString += " | ";
				lOutputString += tangentWVMode[TangentVelocityFlagToIndex(pCurve->KeyGet(lCount).GetTangentVelocityMode())];
			}
			lOutputString += " ]";
			lOutputString += "\n";
			FBXSDK_printf(lOutputString);
		}
	}

	// Vertex Section [ 0 = Translation, 1 = Rotation, 2 = 
	void FBX_Functions::DisplayCurveKeys(FbxAnimCurve* pCurve, FbxNode* pNode, Vertex_Part pVertex_Part, Vertex_Part_Data pVertex_Part_Data, int pJointIndex)
	{
		static const char* interpolation[] = { "?", "constant", "linear", "cubic" };
		static const char* constantMode[] = { "?", "Standard", "Next" };
		static const char* cubicMode[] = { "?", "Auto", "Auto break", "Tcb", "User", "Break", "User break" };
		static const char* tangentWVMode[] = { "?", "None", "Right", "Next left" };

		FbxTime   lKeyTime;
		float   lKeyValue;
		char    lTimeString[256];
		FbxString lOutputString;
		int     lCount;

		int lKeyCount = pCurve->KeyGetCount();

		for (lCount = 0; lCount < lKeyCount; lCount++)
		{
			// Keyframe that will be filled out and passed into the joint
			Keyframe_Vertex_Info lKeyframe_Vertex_Info;

			lKeyValue = static_cast<float>(pCurve->KeyGetValue(lCount));
			lKeyTime = pCurve->KeyGetTime(lCount);

			int checkingVariable = atoi(lKeyTime.GetTimeString(lTimeString, FbxUShort(256)));

			lOutputString = "            Key Time: ";
			lOutputString += lKeyTime.GetTimeString(lTimeString, FbxUShort(256));
			lOutputString += ".... Key Value: ";
			lOutputString += lKeyValue;
			lOutputString += " [ ";
			lOutputString += interpolation[InterpolationFlagToIndex(pCurve->KeyGetInterpolation(lCount))];

			// If index is -1 it is a root node or it's not in the list
			if (pJointIndex != -1)
			{
				// Check to see what we should be putting the info into
				lKeyframe_Vertex_Info.pKeytime = atoi(lKeyTime.GetTimeString(lTimeString, FbxUShort(256)));
				lKeyframe_Vertex_Info.pVal = lKeyValue;
				//lKeyframe_Vertex_Info.pVal = *pNode->EvaluateGlobalTransform().Buffer()->mData;
				// Fill out the valuetype and valueindex of the info
				lKeyframe_Vertex_Info.pValueType = pVertex_Part;
				lKeyframe_Vertex_Info.pValueIndex = pVertex_Part_Data;

				// Push back the keyframe info
				gSkeleton.pJoints[pJointIndex].pVertex_Info_Vector.pVertex_Infos.push_back(lKeyframe_Vertex_Info);
			}

			if ((pCurve->KeyGetInterpolation(lCount)&FbxAnimCurveDef::eInterpolationConstant) == FbxAnimCurveDef::eInterpolationConstant)
			{
				lOutputString += " | ";
				lOutputString += constantMode[ConstantmodeFlagToIndex(pCurve->KeyGetConstantMode(lCount))];
			}
			else if ((pCurve->KeyGetInterpolation(lCount)&FbxAnimCurveDef::eInterpolationCubic) == FbxAnimCurveDef::eInterpolationCubic)
			{
				lOutputString += " | ";
				lOutputString += cubicMode[TangentmodeFlagToIndex(pCurve->KeyGetTangentMode(lCount))];
				lOutputString += " | ";
				lOutputString += tangentWVMode[TangentweightFlagToIndex(pCurve->KeyGet(lCount).GetTangentWeightMode())];
				lOutputString += " | ";
				lOutputString += tangentWVMode[TangentVelocityFlagToIndex(pCurve->KeyGet(lCount).GetTangentVelocityMode())];
			}
			lOutputString += " ]";
			lOutputString += "\n";
			FBXSDK_printf(lOutputString);
		}
	}

	void FBX_Functions::DisplayListCurveKeys(FbxAnimCurve* pCurve, FbxProperty* pProperty)
	{
		FbxTime   lKeyTime;
		int     lKeyValue;
		char    lTimeString[256];
		FbxString lListValue;
		FbxString lOutputString;
		int     lCount;

		int lKeyCount = pCurve->KeyGetCount();

		for (lCount = 0; lCount < lKeyCount; lCount++)
		{
			lKeyValue = static_cast<int>(pCurve->KeyGetValue(lCount));
			lKeyTime = pCurve->KeyGetTime(lCount);

			lOutputString = "            Key Time: ";
			lOutputString += lKeyTime.GetTimeString(lTimeString, FbxUShort(256));
			lOutputString += ".... Key Value: ";
			lOutputString += lKeyValue;
			lOutputString += " (";
			lOutputString += pProperty->GetEnumValue(lKeyValue);
			lOutputString += ")";

			lOutputString += "\n";
			FBXSDK_printf(lOutputString);
		}
	}

	void FBX_Functions::ConstructKeyFrames()
	{
		// Loop through each joint
		for (unsigned int i = 0; i < gSkeleton.pJoints.size(); i++)
		{
			// Get current joint
			Joint lCurrJoint = gSkeleton.pJoints[i];

			// Get current vertex infos
			std::vector<Keyframe_Vertex_Info> lCurrVertex_Infos = lCurrJoint.pVertex_Info_Vector.pVertex_Infos;

			// Create Info Vectors for Trans, Rot, Scale
			std::vector<Keyframe_Vertex_Info> lTranslation_Infos;
			std::vector<Keyframe_Vertex_Info> lRotation_Infos;
			std::vector<Keyframe_Vertex_Info> lScale_Infos;

			// Create a keyframe vertex
			Keyframe_Vertex lKeyframeVertex;

			// Loop through joints vertex info vector
			// And construct the types (trans, rot, scale)
			for (unsigned j = 0; j < lCurrVertex_Infos.size(); j++)
			{
				// Current Vertex Info
				Keyframe_Vertex_Info lCurrVertex_Info = lCurrVertex_Infos[j];

				switch (lCurrVertex_Info.pValueType)
				{
				case TRANSLATION:
					lTranslation_Infos.push_back(lCurrVertex_Info);
					break;
				case ROTATION:
					lRotation_Infos.push_back(lCurrVertex_Info);
					break;
				case SCALE:
					lScale_Infos.push_back(lCurrVertex_Info);
					break;
				}
			}

			// Create variables to hold largest keytime
			// Find the largest keytime for translation, rotation, scale
			unsigned int lTranslation_Size = LargestKeyTime(lTranslation_Infos);
			unsigned int lRotation_Size = LargestKeyTime(lRotation_Infos);
			unsigned int lScale_size = LargestKeyTime(lScale_Infos);

			// Check to see if all sizes match if so
			// Loop through each type and create a keyframe
			if (lTranslation_Size == lRotation_Size &&
				lTranslation_Size == lScale_size &&
				lRotation_Size == lScale_size)
			{
				unsigned int lInfo_Size = lTranslation_Size;

				// Create a keyframe for the info size
				// Not filling it out just yet
				for (unsigned int k = 0; k < lInfo_Size; k++)
				{
					// Create keyframe info
					Keyframe_Info lKeyframe;

					// Push back the keyframe to the skeleton
					gSkeleton.pJoints[i].pKeyframes.push_back(lKeyframe);
				}
			}

			// Set joint infos
			gSkeleton.pJoints[i].pTranslation_Infos = lTranslation_Infos;
			gSkeleton.pJoints[i].pRotation_Infos = lRotation_Infos;
			gSkeleton.pJoints[i].pScale_Infos = lScale_Infos;

			// Get all keytimes for later use
			FillOutJointKeyTimes(i);

			// Update the current joint to get updated info
			lCurrJoint = gSkeleton.pJoints[i];

			// Loop through the vector of empty keyframes
			for (unsigned int l = 0; l < lCurrJoint.pKeyframes.size(); l++)
			{
				// Create the 3 vertices. (Trans, Rot, Scale)
				Keyframe_Vertex lTranslation_Vertex;
				Keyframe_Vertex lRotation_Vertex;
				Keyframe_Vertex lScale_Vertex;

				// Get Translation Vertex At Current Keytime
				lTranslation_Vertex = GetDataAtKeyTime(lTranslation_Infos, gSkeleton.pJoints[i].pTranslation_KeyTimes, l);

				// Get Rotation Vertex At Current Keytime
				lRotation_Vertex = GetDataAtKeyTime(lRotation_Infos, gSkeleton.pJoints[i].pRotation_KeyTimes, l);

				// Get Scale Vertex At Current Keytime
				lScale_Vertex = GetDataAtKeyTime(lScale_Infos, gSkeleton.pJoints[i].pScale_KeyTimes, l);

				// Copy the vertices into the current keyframe
				gSkeleton.pJoints[i].pKeyframes[l].pTranslation = lTranslation_Vertex;
				gSkeleton.pJoints[i].pKeyframes[l].pRotation = lRotation_Vertex;
				gSkeleton.pJoints[i].pKeyframes[l].pScale = lScale_Vertex;
			}
		}
	}

	int FBX_Functions::LargestKeyTime(std::vector<Keyframe_Vertex_Info> pKeyframe_Infos)
	{
		int lMaxKeyTime = 0;

		for (unsigned int i = 0; i < pKeyframe_Infos.size() - 1; i++)
			lMaxKeyTime = CompareKeyTime(pKeyframe_Infos[i], pKeyframe_Infos[i + 1]);

		return lMaxKeyTime;
	}

	int FBX_Functions::CompareKeyTime(Keyframe_Vertex_Info pA, Keyframe_Vertex_Info pB)
	{
		int lBiggestKeyTime = 0;

		if (pA.pKeytime == pB.pKeytime)
			lBiggestKeyTime = pA.pKeytime;
		else if (pA.pKeytime > pB.pKeytime)
			lBiggestKeyTime = pA.pKeytime;
		else if (pB.pKeytime > pA.pKeytime)
			lBiggestKeyTime = pB.pKeytime;

		return lBiggestKeyTime;
	}

	Keyframe_Vertex VertexInterpolation(Keyframe_Vertex pA, Keyframe_Vertex pB, float pRatio)
	{
		// Create the keyframe to fill out and return
		Keyframe_Vertex lTemp;

		// Fill out the keytime of the vertex
		lTemp.pX.pKeytime = (pA.pX.pKeytime + pB.pX.pKeytime) / 2;
		lTemp.pY.pKeytime = (pA.pY.pKeytime + pB.pY.pKeytime) / 2;
		lTemp.pZ.pKeytime = (pA.pZ.pKeytime + pB.pZ.pKeytime) / 2;

		// Fill out the value of the vertex
		lTemp.pX.pVal = (pB.pX.pVal - pA.pX.pVal) * pRatio;
		lTemp.pY.pVal = (pB.pY.pVal - pA.pY.pVal) * pRatio;
		lTemp.pZ.pVal = (pB.pZ.pVal - pA.pZ.pVal) * pRatio;

		// Fill out the vertex part
		lTemp.pX.pValueType = pA.pX.pValueType;
		lTemp.pY.pValueType = pA.pY.pValueType;
		lTemp.pZ.pValueType = pA.pZ.pValueType;

		// FIll out the vertex part data
		lTemp.pX.pValueIndex = X;
		lTemp.pY.pValueIndex = Y;
		lTemp.pZ.pValueIndex = Z;

		// Return the newly interpolate vertex
		return lTemp;
	}

	Keyframe_Vertex FBX_Functions::GetDataAtKeyTime(std::vector<Keyframe_Vertex_Info> pKeyframe_Infos, std::vector<int> pKeytimes, int pKeyTime)
	{
		// Create a Vertex to hold data
		Keyframe_Vertex lTemp_Vertex;

		// Create variable to hold vector size
		unsigned int lSize = pKeyframe_Infos.size();
		unsigned int lK_Size = pKeytimes.size();

		// Create bool to see if the keytime exists
		bool lKeytime_Exist = true;

		// To hold prev and next keytimes
		int lPrev_Keytime;
		int lNext_Keytime;

		// 
		bool lPrevFound = false;

		for (unsigned int i = 0; i < lK_Size; i++)
		{
			if (pKeyTime != pKeytimes[i])
			{
				if ((i + 1) >= lK_Size)
				{
					// Means animation should restart
					lNext_Keytime = pKeytimes[0];
				}
				if (i == lK_Size - 1)
				{
					lKeytime_Exist = false;
					lNext_Keytime = pKeytimes[i];
					break;
				}
				else if (lPrevFound)
				{
					lKeytime_Exist = false;
					lNext_Keytime = pKeytimes[i];
					break;
				}
			}
			else
			{
				// Then it does exist
				// break because it's found
				lKeytime_Exist = true;
				break;
			}

			// Update previous key time
			if (pKeytimes[i] < pKeyTime)
				lPrev_Keytime = pKeytimes[i];

			// Check to see if lPrev_Keytime is done updatign
			if (pKeytimes[i] > pKeyTime)
			{
				lKeytime_Exist = false;
				lNext_Keytime = pKeytimes[i];
				break;
			}
		}

		if (lKeytime_Exist)
		{
			// Loop through for each X
			for (unsigned int i = 0; i < lSize; i++)
				if (pKeyframe_Infos[i].pKeytime == pKeyTime && pKeyframe_Infos[i].pValueIndex == X)
					lTemp_Vertex.pX = pKeyframe_Infos[i];

			// Loop through for each Y
			for (unsigned int i = 0; i < lSize; i++)
				if (pKeyframe_Infos[i].pKeytime == pKeyTime && pKeyframe_Infos[i].pValueIndex == Y)
					lTemp_Vertex.pY = pKeyframe_Infos[i];

			// Loop through for each Z
			for (unsigned int i = 0; i < lSize; i++)
				if (pKeyframe_Infos[i].pKeytime == pKeyTime && pKeyframe_Infos[i].pValueIndex == Z)
					lTemp_Vertex.pZ = pKeyframe_Infos[i];
		}
		else
		{
			// interpolate between prev and next keytimes vertices
			Keyframe_Vertex lNext_Vertex;
			Keyframe_Vertex lPrev_Vertex;
			Keyframe_Vertex lLerp_Vertex;

			// Fill out the prev vertex
			// Loop through for each X
			for (unsigned int i = 0; i < lSize; i++)
				if (pKeyframe_Infos[i].pKeytime == lPrev_Keytime && pKeyframe_Infos[i].pValueIndex == X)
					lPrev_Vertex.pX = pKeyframe_Infos[i];

			// Loop through for each Y
			for (unsigned int i = 0; i < lSize; i++)
				if (pKeyframe_Infos[i].pKeytime == lPrev_Keytime && pKeyframe_Infos[i].pValueIndex == Y)
					lPrev_Vertex.pY = pKeyframe_Infos[i];

			// Loop through for each Z
			for (unsigned int i = 0; i < lSize; i++)
				if (pKeyframe_Infos[i].pKeytime == lPrev_Keytime && pKeyframe_Infos[i].pValueIndex == Z)
					lPrev_Vertex.pZ = pKeyframe_Infos[i];

			// Fill out the next vertex
			// Loop through for each X
			for (unsigned int i = 0; i < lSize; i++)
				if (pKeyframe_Infos[i].pKeytime == lNext_Keytime && pKeyframe_Infos[i].pValueIndex == X)
					lNext_Vertex.pX = pKeyframe_Infos[i];

			// Loop through for each Y
			for (unsigned int i = 0; i < lSize; i++)
				if (pKeyframe_Infos[i].pKeytime == lNext_Keytime && pKeyframe_Infos[i].pValueIndex == Y)
					lNext_Vertex.pY = pKeyframe_Infos[i];

			// Loop through for each Z
			for (unsigned int i = 0; i < lSize; i++)
				if (pKeyframe_Infos[i].pKeytime == lNext_Keytime && pKeyframe_Infos[i].pValueIndex == Z)
					lNext_Vertex.pZ = pKeyframe_Infos[i];

			// Interpolate between the prev -> next
			lLerp_Vertex = VertexInterpolation(lPrev_Vertex, lNext_Vertex, 0.5f);

			// return the interpolated vertex
			return lLerp_Vertex;
		}


		return lTemp_Vertex;
	}

	void FBX_Functions::FillOutJointKeyTimes(int pJointIndex)
	{
		// Create a set to hold my keytimes
		std::set<int> lT_Set;
		std::set<int> lR_Set;
		std::set<int> lS_Set;
		std::set<int> lAll_Set;

		// Get the sizes of the key time vectors
		unsigned int lT_Size = gSkeleton.pJoints[pJointIndex].pTranslation_Infos.size();
		unsigned int lR_Size = gSkeleton.pJoints[pJointIndex].pRotation_Infos.size();
		unsigned int lS_Size = gSkeleton.pJoints[pJointIndex].pScale_Infos.size();

		// Fill out the set for translation
		for (unsigned i = 0; i < lT_Size; ++i)
		{
			lT_Set.insert(gSkeleton.pJoints[pJointIndex].pTranslation_Infos[i].pKeytime);
			lAll_Set.insert(gSkeleton.pJoints[pJointIndex].pTranslation_Infos[i].pKeytime);
		}
		// Fill out the set for rotation
		for (unsigned i = 0; i < lR_Size; ++i)
		{
			lR_Set.insert(gSkeleton.pJoints[pJointIndex].pRotation_Infos[i].pKeytime);
			lAll_Set.insert(gSkeleton.pJoints[pJointIndex].pRotation_Infos[i].pKeytime);
		}
		// Fill out the set for scale
		for (unsigned i = 0; i < lS_Size; ++i)
		{
			lS_Set.insert(gSkeleton.pJoints[pJointIndex].pScale_Infos[i].pKeytime);
			lAll_Set.insert(gSkeleton.pJoints[pJointIndex].pScale_Infos[i].pKeytime);
		}
		

		// Assign the sets to the keytime vectors of the joint
		gSkeleton.pJoints[pJointIndex].pTranslation_KeyTimes.assign(lT_Set.begin(), lT_Set.end());
		gSkeleton.pJoints[pJointIndex].pRotation_KeyTimes.assign(lR_Set.begin(), lR_Set.end());
		gSkeleton.pJoints[pJointIndex].pScale_KeyTimes.assign(lS_Set.begin(), lS_Set.end());
		gSkeleton.pJoints[pJointIndex].pAll_KeyTimes.assign(lAll_Set.begin(), lAll_Set.end());
	}

	unsigned int FBX_Functions::FindJointIndexUsingName(std::string currJointName)
	{
		unsigned int index = -1;

		for (unsigned int i = 0; i < gSkeleton.pJoints.size(); i++)
		{
			if (gSkeleton.pJoints[i].pName == currJointName)
				index = i;
				break;
		}

		return index;
	}

	void FBX_Functions::ProcessControlPoints(FbxNode* inNode)
	{
		FbxMesh* currMesh = inNode->GetMesh();
		unsigned int ctrlPointCount = currMesh->GetControlPointsCount();
		for (unsigned int i = 0; i < ctrlPointCount; ++i)
		{
			CtrlPoint* currCtrlPoint = new CtrlPoint();
			DirectX::XMFLOAT3 currPosition;
			currPosition.x = static_cast<float>(currMesh->GetControlPointAt(i).mData[0]);
			currPosition.y = static_cast<float>(currMesh->GetControlPointAt(i).mData[1]);
			currPosition.z = static_cast<float>(currMesh->GetControlPointAt(i).mData[2]);
			currCtrlPoint->pPosition = currPosition;
			gSkeleton.pControlPoints[i] = currCtrlPoint;
		}
	}

	void FBX_Functions::ProcessJointsAndAnimations(FbxNode* inNode)
	{
		FbxMesh* currMesh = inNode->GetMesh();
		unsigned int numOfDeformers = currMesh->GetDeformerCount();
		// This geometry transform is something I cannot understand
		// I think it is from MotionBuilder
		// If you are using Maya for your models, 99% this is just an
		// identity matrix
		// But I am taking it into account anyways......
		FbxAMatrix geometryTransform =	GetGeometryTransformation(inNode);

		// A deformer is a FBX thing, which contains some clusters
		// A cluster contains a link, which is basically a joint
		// Normally, there is only one deformer in a mesh
		for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
		{
			// There are many types of deformers in Maya,
			// We are using only skins, so we see if this is a skin
			FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
			if (!currSkin)
			{
				continue;
			}

			unsigned int numOfClusters = currSkin->GetClusterCount();
			for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
			{
				FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
				std::string currJointName = currCluster->GetLink()->GetName();
				unsigned int currJointIndex = FindJointIndexUsingName(currJointName);
				FbxAMatrix transformMatrix;
				FbxAMatrix transformLinkMatrix;
				FbxAMatrix globalBindposeInverseMatrix;

				currCluster->GetTransformMatrix(transformMatrix);	// The transformation of the mesh at binding time
				currCluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(joint) at binding time from joint space to world space
				globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;

				// Update the information in mSkeleton 
				//gSkeleton.pJoints[currJointIndex].mGlobalBindposeInverse = globalBindposeInverseMatrix;
				// gSkeleton.pJoints[currJointIndex].mNode = currCluster->GetLink();

				// Associate each joint with the control points it affects
				unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();
				for (unsigned int i = 0; i < numOfIndices; ++i)
				{
					BlendingIndexWeightPair currBlendingIndexWeightPair;
					currBlendingIndexWeightPair.pBlendingIndex = currJointIndex;
					currBlendingIndexWeightPair.pBlendingWeight = currCluster->GetControlPointWeights()[i];
					gSkeleton.pControlPoints[currCluster->GetControlPointIndices()[i]]->pBlendingInfo.push_back(currBlendingIndexWeightPair);
				}

				// Get animation information
				// Now only supports one take
				FbxAnimStack* currAnimStack = gScene->GetSrcObject<FbxAnimStack>(0);
				FbxString animStackName = currAnimStack->GetName();
				gSkeleton.pAnimationName = animStackName.Buffer();
				FbxTakeInfo* takeInfo = gScene->GetTakeInfo(animStackName);
				FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
				FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
				gSkeleton.pAnimationLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
				//Keyframe** currAnim = &gSkeleton.mJoints[currJointIndex].mAnimation;

				/*for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i)
				{
					FbxTime currTime;
					currTime.SetFrame(i, FbxTime::eFrames24);
					*currAnim = new Keyframe();
					(*currAnim)->mFrameNum = i;
					FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geometryTransform;
					(*currAnim)->mGlobalTransform = currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime);
					currAnim = &((*currAnim)->mNext);
				}*/
			}
		}

		// Some of the control points only have less than 4 joints
		// affecting them.
		// For a normal renderer, there are usually 4 joints
		// I am adding more dummy joints if there isn't enough
		BlendingIndexWeightPair currBlendingIndexWeightPair;
		currBlendingIndexWeightPair.pBlendingIndex = 0;
		currBlendingIndexWeightPair.pBlendingWeight = 0;
		/*for (auto itr = gSkeleton.pControlPoints.begin(); itr != gSkeleton.pControlPoints.end(); ++itr)
		{
			for (unsigned int i = itr->pBlendingInfo.size(); i <= 4; ++i)
			{
				itr->pBlendingInfo.push_back(currBlendingIndexWeightPair);
			}
		}*/
	}
}
