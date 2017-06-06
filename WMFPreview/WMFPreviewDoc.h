// WMFPreviewDoc.h : Declaration of the WMFPreviewDoc class

#pragma once

#include <atlhandlerimpl.h>

using namespace ATL;
using namespace Gdiplus;

#ifdef _DEBUG
void DbgOut(CString txt);
#define DBGOUT DbgOut
#else
#define DBGOUT
#endif

// META_PLACEABLE record; see: https://msdn.microsoft.com/en-us/library/cc669452.aspx

typedef struct _WmfSpecialHeader
{
	DWORD Key;           /* Magic number (always 9AC6CDD7h) */
	WORD  Handle;        /* Metafile HANDLE number (always 0) */
	SHORT Left;          /* Left coordinate in metafile units */
	SHORT Top;           /* Top coordinate in metafile units */
	SHORT Right;         /* Right coordinate in metafile units */
	SHORT Bottom;        /* Bottom coordinate in metafile units */
	WORD  Inch;          /* Number of metafile units per inch */
	DWORD Reserved;      /* Reserved (always 0) */
	WORD  Checksum;      /* Checksum value for previous 10 WORDs */
} WMFSPECIALHEADER;

// See https://msdn.microsoft.com/en-us/library/cc250418.aspx

typedef enum
{
	MEMORYMETAFILE = 0x0001,
	DISKMETAFILE = 0x0002
} WmfType;

#define	WMFSPECIALHEADERKEY		0x9AC6CDD7
#define	WMFSPECIALHEADERSIZE	22

class WMFPreviewDoc : public CAtlDocumentImpl
{
protected:
	WMFSPECIALHEADER m_SpecialWMFHeader;
	BOOL m_HasSpecialWMFHeader;
	HENHMETAFILE m_hMetaFile;
	Metafile * m_MetaFile;

	ULONG_PTR gdiplusToken;
	GdiplusStartupInput gdiplusStartupInput;

public:
	WMFPreviewDoc(void)
	{
		m_HasSpecialWMFHeader = FALSE;
		m_hMetaFile = NULL;
		m_MetaFile = NULL;

		// Initialize GDI+
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	}

	virtual ~WMFPreviewDoc(void)
	{
		ReleaseWMF();

		// Shutdown GDI+
		GdiplusShutdown(gdiplusToken);
	}

	void ReleaseWMF();
	WORD CalcWMFHeaderChecksum(WMFSPECIALHEADER* wmfsh);
	BOOL GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha);
	BOOL DrawWMF(HDC hdc, LPRECT lprcBounds, COLORREF clrBack);

	virtual HRESULT LoadFromStream(IStream* pStream, DWORD grfMode);
	virtual void InitializeSearchContent();

protected:
	void SetSearchContent(CString& value);
	virtual void OnDrawThumbnail(HDC hDrawDC, LPRECT lprcBounds);
};
