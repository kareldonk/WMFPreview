// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "WMFPreview_i.h"
#include "dllmain.h"
#include "compreg.h"
#include "xdlldata.h"

CWMFPreviewModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
	hInstance;

	switch (dwReason)
	{
	case DLL_PROCESS_ATTACH:
		//
		break;
	case DLL_PROCESS_DETACH:
		//
		break;
	}

	return _AtlModule.DllMain(dwReason, lpReserved); 
}
