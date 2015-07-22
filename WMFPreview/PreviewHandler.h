// PreviewHandler.h : Declaration of the preview handler

#pragma once
#include "resource.h"       // main symbols

#define AFX_PREVIEW_STANDALONE
#include <atlhandler.h>
#include <atlhandlerimpl.h>
#include "WMFPreviewDoc.h"
#include <atlpreviewctrlimpl.h>

#include "WMFPreview_i.h"

using namespace ATL;

// CPreviewCtrl implementation
class CPreviewCtrl : public CAtlPreviewCtrlImpl
{
protected:
	virtual void DoPaint(HDC hdc)
	{
		RECT crect;
		
		if (GetClientRect(&crect))
		{
			WMFPreviewDoc* pDoc = (WMFPreviewDoc*)m_pDocument;
			pDoc->DrawWMF(hdc, &crect);
		}
	}

public:
	BEGIN_MSG_MAP(CPreviewCtrl)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
	END_MSG_MAP()
	
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = TRUE;

		return TRUE;
	}

	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return CAtlPreviewCtrlImpl::OnPaint(uMsg, wParam, lParam, bHandled);
	}
};

// CPreviewHandler

class ATL_NO_VTABLE CPreviewHandler :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPreviewHandler, &CLSID_Preview>,
	public CPreviewHandlerImpl <CPreviewHandler>
{
public:
	CPreviewHandler()
	{
	}

DECLARE_REGISTRY_RESOURCEID(IDR_PREVIEW_HANDLER)
DECLARE_NOT_AGGREGATABLE(CPreviewHandler)

BEGIN_COM_MAP(CPreviewHandler)
	COM_INTERFACE_ENTRY(IObjectWithSite)
	COM_INTERFACE_ENTRY(IOleWindow)
	COM_INTERFACE_ENTRY(IInitializeWithStream)
	COM_INTERFACE_ENTRY(IPreviewHandler)
	COM_INTERFACE_ENTRY(IPreviewHandlerVisuals)
END_COM_MAP()

	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}

	void FinalRelease()
	{
		CPreviewHandlerImpl<CPreviewHandler>::FinalRelease();
	}

protected:
	virtual IPreviewCtrl* CreatePreviewControl()
	{
		// This class is defined at the beginning of this header
		CPreviewCtrl *pPreviewCtrl = NULL;
		ATLTRY(pPreviewCtrl = new CPreviewCtrl());
		return pPreviewCtrl;
	}

	virtual IDocument* CreateDocument()
	{
		WMFPreviewDoc *pDocument = NULL;
		ATLTRY(pDocument = new WMFPreviewDoc());
		return (IDocument*)pDocument;
	}

};

OBJECT_ENTRY_AUTO(__uuidof(Preview), CPreviewHandler)
