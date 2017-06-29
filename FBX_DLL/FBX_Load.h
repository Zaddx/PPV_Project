#pragma once

#include "FBX_DLL.h"
#include "FBX_Helper_Structures.h"
#include "Info_Passer.h"
using namespace FBXLibrary;

class FBX_Load
{
private:
	FBX_Functions fbx_func;
public:
	void LoadFBXFile(char** lFilename);
	Skeleton* getSkeleton();
};