#include "stdafx.h"
#include "Info_Passer.h"
#include "FBX_Load.h"

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
	FBX_Load fbx_loader;
	fbx_loader.LoadFBXFile(pFileName);
}
