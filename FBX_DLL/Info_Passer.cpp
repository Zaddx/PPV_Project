#include "stdafx.h"
#include "Info_Passer.h"
#include "FBX_Load.h"

// Global for the fbx loader
FBX_Load fbx_loader;

void INFO_PASSER::setFileName(char** lFilename)
{
	pFileName = lFilename;
}

char** INFO_PASSER::getFileName()
{
	return pFileName;
}

void INFO_PASSER::Load_FBX()
{
	fbx_loader.LoadFBXFile(pFileName);
}

Skeleton* INFO_PASSER::getSkeleton()
{
	return fbx_loader.getSkeleton();
}