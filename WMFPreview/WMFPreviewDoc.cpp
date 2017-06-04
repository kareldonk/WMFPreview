// WMFPreviewDoc.cpp : Implementation of the WMFPreviewDoc class

#include "stdafx.h"
#include <propkey.h>
#include "WMFPreviewDoc.h"


HRESULT WMFPreviewDoc::LoadFromStream(IStream* pStream, DWORD grfMode)
{
	// If we already have a metafile release it 
	if (m_hMetaFile != NULL) ReleaseWMF();

	BOOL bShdr = FALSE;

	ULONG len = 0;
	STATSTG stat;
	ZeroMemory(&stat, sizeof(STATSTG));

	if (pStream->Stat(&stat, STATFLAG_DEFAULT) != S_OK) return S_FALSE;

	if (stat.cbSize.QuadPart < WMFSPECIALHEADERSIZE) return S_FALSE;

	ZeroMemory(&m_SpecialWMFHeader, WMFSPECIALHEADERSIZE);

	// First check if there's a special WMF header
	if (pStream->Read(&m_SpecialWMFHeader, WMFSPECIALHEADERSIZE, &len) == S_OK)
	{
		if (len == WMFSPECIALHEADERSIZE)
		{
			if (m_SpecialWMFHeader.Key == WMFSPECIALHEADERKEY)
			{
				// Has special WMF header
				bShdr = TRUE;

				// Calculate header cheksum and verify
				if (CalcWMFHeaderChecksum(&m_SpecialWMFHeader) != m_SpecialWMFHeader.Checksum) return S_FALSE;
			}

			LARGE_INTEGER pos;
			ZeroMemory(&pos, sizeof(LARGE_INTEGER));

			if (bShdr) pos.QuadPart = WMFSPECIALHEADERSIZE;

			if (pStream->Seek(pos, STREAM_SEEK_SET, NULL) == S_OK)
			{
				METAHEADER hdr;

				ZeroMemory(&hdr, sizeof(METAHEADER));

				// Read WMF header
				if (pStream->Read(&hdr, sizeof(hdr), &len) == S_OK)
				{
					if (len == sizeof(hdr))
					{
						// Should be valid WMF file
						if ((hdr.mtType == WmfType::DISKMETAFILE) || (hdr.mtType == WmfType::MEMORYMETAFILE))
						{
							pos.QuadPart = 0;

							// Go back to beginning and read entire file
							if (pStream->Seek(pos, STREAM_SEEK_SET, NULL) == S_OK)
							{
								BYTE * data = new BYTE[(ULONG) stat.cbSize.QuadPart];

								if (pStream->Read(data, (ULONG) stat.cbSize.QuadPart, &len) == S_OK)
								{
									if (len == stat.cbSize.QuadPart)
									{
										HENHMETAFILE mfile = NULL;

										if (bShdr)
										{
											// Old style windows meta file
											mfile = SetWinMetaFileBits((ULONG) stat.cbSize.QuadPart - WMFSPECIALHEADERSIZE, data + WMFSPECIALHEADERSIZE, NULL, NULL);
										}
										else
										{
											// Enhanced windows meta file
											mfile = SetEnhMetaFileBits((ULONG) stat.cbSize.QuadPart, data);
										}

										if (mfile != NULL)
										{
											HDC hdc = ::GetDC(NULL);
											
											// GDI+ should be initialized
											Graphics graphics(hdc);

											// Turn on AntiAliasing for best quality
											graphics.SetSmoothingMode(SmoothingModeAntiAlias);

											m_MetaFile = new Metafile(mfile);

											// Convert the EMF to EMF+ to enable antialiased drawing
											m_MetaFile->ConvertToEmfPlus(&graphics);

											m_hMetaFile = mfile;
											m_HasSpecialWMFHeader = bShdr;

											ReleaseDC(NULL, hdc);

											delete data;

											return S_OK;
										}
									}
								}

								delete data;
							}
						}
					}
				}
			}
		}
	}

	return S_FALSE;
}

void WMFPreviewDoc::InitializeSearchContent()
{
}

void WMFPreviewDoc::SetSearchContent(CString& value)
{
}

void WMFPreviewDoc::ReleaseWMF()
{
	if (m_MetaFile != NULL)
	{
		delete m_MetaFile;
		m_MetaFile = NULL;
	}

	if (m_hMetaFile != NULL)
	{
		DeleteEnhMetaFile(m_hMetaFile);
		m_hMetaFile = NULL;
	}

	m_HasSpecialWMFHeader = FALSE;
}

WORD WMFPreviewDoc::CalcWMFHeaderChecksum(WMFSPECIALHEADER* wmfsh)
{
	WORD chksum = 0;

	WORD * words = (WORD*)wmfsh;

	// Calculate checksum for first 10 words
	for (int x = 0; x < 10; x++)
	{
		chksum ^= words[x];
	}

	return chksum;
}

BOOL WMFPreviewDoc::GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha)
{
	// In case no valid WMF was loaded return
	if (m_hMetaFile == NULL) return FALSE;

	HDC hdc = ::GetDC(NULL);
	HDC memDC = CreateCompatibleDC(hdc);

	if (memDC != NULL)
	{
		HBITMAP memBM = CreateBitmap(cx, cx, 1, 32, NULL);

		if (memBM != NULL)
		{
			HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

			HBRUSH hDrawBrush = CreateSolidBrush(RGB(255, 255, 255));

			RECT rcBounds;
			SetRect(&rcBounds, 0, 0, cx, cx);

			// Fill background
			FillRect(memDC, &rcBounds, hDrawBrush);

			int margin = 4;

			rcBounds.left += margin;
			rcBounds.top += margin;
			rcBounds.right -= margin;
			rcBounds.bottom -= margin;

			// Draw WMF file
			OnDrawThumbnail(memDC, &rcBounds);

			SelectObject(memDC, oldBM);

			DeleteObject(hDrawBrush);
			DeleteDC(memDC);
			ReleaseDC(NULL, hdc);

			*phbmp = memBM;
			*pdwAlpha = WTS_ALPHATYPE::WTSAT_UNKNOWN;

			return TRUE;
		}

		DeleteDC(memDC);
	}

	ReleaseDC(NULL, hdc);

	return FALSE;
}

BOOL WMFPreviewDoc::DrawWMF(HDC hdc, LPRECT lprcBounds, COLORREF clrBack)
{
	HDC memDC = CreateCompatibleDC(hdc);

	if (memDC != NULL)
	{
		HBITMAP memBM = CreateBitmap(lprcBounds->right - lprcBounds->left, lprcBounds->bottom - lprcBounds->top, 1, 32, NULL);

		if (memBM != NULL)
		{
			HBITMAP oldBM = (HBITMAP)SelectObject(memDC, memBM);

			HBRUSH hDrawBrush = CreateSolidBrush(clrBack);

			// Fill background
			FillRect(memDC, lprcBounds, hDrawBrush);

			RECT crect;
			int margin = 10;

			crect.left = lprcBounds->left + margin;
			crect.top = lprcBounds->top + margin;
			crect.right = lprcBounds->right - margin;
			crect.bottom = lprcBounds->bottom - margin;

			// Draw WMF file
			OnDrawThumbnail(memDC, &crect);

			// Copy to main paint DC
			if (BitBlt(hdc, lprcBounds->left, lprcBounds->top, lprcBounds->right - lprcBounds->left, lprcBounds->bottom - lprcBounds->top, memDC, 0, 0, SRCCOPY))
			{
				SelectObject(memDC, oldBM);

				DeleteObject(memBM);
				DeleteObject(hDrawBrush);
				DeleteDC(memDC);

				return TRUE;
			}

			SelectObject(memDC, oldBM);

			DeleteObject(memBM);
			DeleteObject(hDrawBrush);
		}

		DeleteDC(memDC);
	}

	return FALSE;
}

void WMFPreviewDoc::OnDrawThumbnail(HDC hDrawDC, LPRECT lprcBounds)
{
#ifdef _DEBUG

	CString temp;
	CString strDebug = _T("Debug Info:");

#endif

	// Using GDI+ to draw
	Graphics graphics(hDrawDC);

	// Turn on AntiAliasing for best quality
	graphics.SetSmoothingMode(SmoothingModeAntiAlias);

	if (m_hMetaFile != NULL)
	{
		SIZE msize;
		ZeroMemory(&msize, sizeof(SIZE));

		// If we have a special WMF header take the dimensions from there
		if (m_HasSpecialWMFHeader)
		{
			msize.cx = m_SpecialWMFHeader.Right - m_SpecialWMFHeader.Left;
			msize.cy = m_SpecialWMFHeader.Bottom - m_SpecialWMFHeader.Top;
		}
		else
		{
			// Get WMF dimensions from metafile header
			UINT hdrlen = GetEnhMetaFileHeader(m_hMetaFile, 0, NULL);

			BYTE * buf = new BYTE[hdrlen];

			if (GetEnhMetaFileHeader(m_hMetaFile, hdrlen, (ENHMETAHEADER*)buf) != 0)
			{
				ENHMETAHEADER* mfhdr = (ENHMETAHEADER*)buf;

				msize.cx = mfhdr->rclFrame.right - mfhdr->rclFrame.left;
				msize.cy = mfhdr->rclFrame.bottom - mfhdr->rclFrame.top;
			}

			delete buf;
		}

		// Center WMF in output rectangle
		int dw = lprcBounds->right - lprcBounds->left;
		int dh = lprcBounds->bottom - lprcBounds->top;

		int w2 = dw;
		int h2 = dh;

		if (msize.cx > msize.cy && msize.cx > 0)
		{
			h2 = (int) (w2 * (float) ((float) msize.cy / (float) msize.cx));

			if (h2 > dh && h2 > 0)
			{
				w2 = (int) (w2 * (float) ((float) dh / (float) h2));
				h2 = dh;
			}
		}
		else if (msize.cy > 0)
		{
			w2 = (int) (h2 * (float) ((float) msize.cx / (float) msize.cy));

			if (w2 > dw && w2 > 0)
			{
				h2 = (int) (h2 * (float) ((float) dw / (float) w2));
				w2 = dw;
			}
		}

#ifdef _DEBUG

		temp.Format(_T(" WMFRECT: %d/%d WNDRECT: %d/%d  DRWRECT: %d/%d"), msize.cx, msize.cy, dw, dh, w2, h2);
		strDebug += temp;

		if (m_HasSpecialWMFHeader)
		{
			int checksum = CalcWMFHeaderChecksum(&m_SpecialWMFHeader);

			temp.Format(_T(" CHK: %d/%d"), checksum, m_SpecialWMFHeader.Checksum);
			strDebug += temp;
		}

#endif

		// Set the drawing area
		RECT prect;
		prect.left = ((dw - w2) / 2) + lprcBounds->left;
		prect.top = (dh - h2) / 2 + lprcBounds->top;
		prect.right = prect.left + w2;
		prect.bottom = prect.top + h2;
		
		// The line below draws the metafile using the older GDI function, 
		// but we'll use GDI+ instead for better (antialiased) quality (although slower)
		//PlayEnhMetaFile(hDrawDC, m_hMetaFile, &prect);

		Rect rect(prect.left, prect.top, w2, h2);

		// Draw the metafile
		graphics.DrawImage(m_MetaFile, rect);
	}
#ifdef _DEBUG
	else strDebug += _T(" NO_WMF_FILE");

	HFONT hStockFont = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
	LOGFONT lf;

	GetObject(hStockFont, sizeof(LOGFONT), &lf);
	lf.lfHeight = 24;
	HFONT hDrawFont = CreateFontIndirect(&lf);

	int bdept = GetDeviceCaps(hDrawDC, BITSPIXEL);

	temp.Format(_T(" %dBIT"), bdept);
	strDebug += temp;

	Font drawFont(hDrawDC, hDrawFont);
	SolidBrush drawBrush(Color::Red);

	StringFormat strFmt;

	// Create point for upper-left corner of drawing.
	RectF drawRect(PointF(0, 0), SizeF((REAL)lprcBounds->right - lprcBounds->left, (REAL)lprcBounds->bottom - lprcBounds->top));
	graphics.DrawString(strDebug, strDebug.GetLength(), &drawFont, drawRect, &strFmt, &drawBrush);

	DeleteObject(hDrawFont);
#endif
}
