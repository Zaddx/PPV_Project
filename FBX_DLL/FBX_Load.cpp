#include "stdafx.h"
#include "FBX_Load.h"

void FBX_Load::LoadFBXFile(char** lFilename)
{
	FBX_Functions fbx_func;
	fbx_func.BeginLoading(lFilename);
}