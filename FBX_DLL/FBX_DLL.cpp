// FBX_DLL.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "FBX_DLL.h"

static bool gVerbose = true;

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

			//FBXSDK_printf("\n\n------------\nNode Content\n------------\n\n");

			//if( gVerbose ) DisplayContent(lScene);

			FBXSDK_printf("\n\n----\nPose\n----\n\n");

			if (gVerbose) DisplayPose(lScene);

			//FBXSDK_printf("\n\n---------\nAnimation\n---------\n\n");

			//if( gVerbose ) DisplayAnimation(lScene);

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

				//case FbxNodeAttribute::eMesh:      
				//    DisplayMesh(pNode);
				//    break;

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
}
