#include "stdafx.h"
#include "FBX_Load.h"

void FBX_Load::LoadFBXFile(char** lFilename)
{
	fbx_func.BeginLoading(lFilename);
}

Skeleton* FBX_Load::getSkeleton()
{
	return fbx_func.getSkeleton();
}