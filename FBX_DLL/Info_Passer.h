#pragma once

#include "FBX_Helper_Structures.h"

class INFO_PASSER
{
	char** pFileName;
public:
	
	// Pass filename from Exec to DLL
	void setFileName(char** lFilename);
	char** getFileName();

	// Load the model
	void Load_FBX();
	Skeleton* getSkeleton();
};