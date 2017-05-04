/****************************** Module Header ******************************\
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#pragma once

#include <windows.h>
#include <shlobj.h>


class CodePreviewHandler :
	public IInitializeWithStream,
	public IPreviewHandler,
	public IPreviewHandlerVisuals,
	public IOleWindow,
	public IObjectWithSite
{
public:
	// IUnknown
	IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv);
	IFACEMETHODIMP_(ULONG) AddRef();
	IFACEMETHODIMP_(ULONG) Release();

	// IInitializeWithStream
	IFACEMETHODIMP Initialize(IStream *pStream, DWORD grfMode);

	// IPreviewHandler
	IFACEMETHODIMP SetWindow(HWND hwnd, const RECT *prc);
	IFACEMETHODIMP SetFocus();
	IFACEMETHODIMP QueryFocus(HWND *phwnd);
	IFACEMETHODIMP TranslateAccelerator(MSG *pmsg);
	IFACEMETHODIMP SetRect(const RECT *prc);
	IFACEMETHODIMP DoPreview();
	IFACEMETHODIMP Unload();

	// IPreviewHandlerVisuals (Optional)
	IFACEMETHODIMP SetBackgroundColor(COLORREF color);
	IFACEMETHODIMP SetFont(const LOGFONTW *plf);
	IFACEMETHODIMP SetTextColor(COLORREF color);

	// IOleWindow
	IFACEMETHODIMP GetWindow(HWND *phwnd);
	IFACEMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

	// IObjectWithSite
	IFACEMETHODIMP SetSite(IUnknown *punkSite);
	IFACEMETHODIMP GetSite(REFIID riid, void **ppv);

	CodePreviewHandler();

protected:
	~CodePreviewHandler();

private:
	// Reference count of component.
	long m_cRef;

	// Provided during initialization.
	IStream *m_pStream;

	// the scintilla library
	HMODULE m_hinstScintilla;

	// Parent window that hosts the previewer window.
	// Note: do NOT DestroyWindow this.
	HWND m_hwndParent;

	// Bounding rect of the parent window.
	RECT m_rcParent;

	// The actual previewer window.
	HWND m_hwndPreview;

	// Site pointer from host, used to get IPreviewHandlerFrame.
	IUnknown *m_punkSite;

	HRESULT CreatePreviewWindow();

	void SetCppLexer();
	void SetPreviewText(const char* pBuffer, size_t cb);
	void UpdateLineNumberWidth();
};
