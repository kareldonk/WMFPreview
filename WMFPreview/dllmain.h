// dllmain.h : Declaration of module class.

#include "Gdiplus.h"

using namespace Gdiplus;

class CWMFPreviewModule : public ATL::CAtlDllModuleT< CWMFPreviewModule >
{
public :
	DECLARE_LIBID(LIBID_WMFPreviewLib)
	DECLARE_REGISTRY_APPID_RESOURCEID(IDR_WMFPREVIEW, "{ABC0839B-A36D-4D0A-ADBF-AE6983538F12}")
};

extern class CWMFPreviewModule _AtlModule;
