#pragma once

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the FBX_DLL_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// FBX_DLL_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef FBX_DLL_EXPORTS
#define FBX_DLL_API __declspec(dllexport)
#else
#define FBX_DLL_API __declspec(dllimport)
#endif

#include "FBX_Helper_Structures.h"
#include "Info_Passer.h"

class FBX_DLL_API FBX_EXECUTABLE
{
private:
	INFO_PASSER info_passer;
public:
	void Load_FBX(char** lFilename);
	void Notify_DLL();
	Skeleton* getSkeleton();
};