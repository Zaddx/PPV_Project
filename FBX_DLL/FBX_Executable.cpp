#include "stdafx.h"
#include "FBX_Executable.h"


void FBX_EXECUTABLE::Load_FBX(char** lFilename)
{
	info_passer.setFileName(lFilename);
	Notify_DLL();
}

void FBX_EXECUTABLE::Notify_DLL()
{
	info_passer.Load_FBX();
}