/****************************** Module Header ******************************\
THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
\***************************************************************************/

#include <Shlwapi.h>
#include <assert.h>
#include "../scintilla/include/Scintilla.h"
#include "../scintilla/include/SciLexer.h"
#include "CodePreviewHandler.h"
#include "Styles.h"
#include "resources.h"

#pragma comment(lib, "Shlwapi.lib")


extern HINSTANCE   g_hInst;
extern long g_cDllRef;
extern SciLexer lexDefault;
extern SciLexer lexCpp;


void DebugPrintf(const wchar_t *format, ...);


inline int RECTWIDTH(const RECT &rc)
{
	return (rc.right - rc.left);
}

inline int RECTHEIGHT(const RECT &rc)
{
	return (rc.bottom - rc.top);
}


CodePreviewHandler::CodePreviewHandler() : m_cRef(1), m_pStream(NULL),
m_hwndParent(NULL), m_hwndPreview(NULL), m_punkSite(NULL), m_hinstScintilla(NULL)
{
	InterlockedIncrement(&g_cDllRef);
}

CodePreviewHandler::~CodePreviewHandler()
{
	if (m_hinstScintilla)
	{
		::FreeLibrary(m_hinstScintilla);
		m_hinstScintilla = NULL;
	}
	if (m_hwndPreview)
	{
		DestroyWindow(m_hwndPreview);
		m_hwndPreview = NULL;
	}
	if (m_punkSite)
	{
		m_punkSite->Release();
		m_punkSite = NULL;
	}
	if (m_pStream)
	{
		m_pStream->Release();
		m_pStream = NULL;
	}

	InterlockedDecrement(&g_cDllRef);
}


#pragma region IUnknown

// Query to the interface the component supported.
IFACEMETHODIMP CodePreviewHandler::QueryInterface(REFIID riid, void **ppv)
{
	static const QITAB qit[] =
	{
		QITABENT(CodePreviewHandler, IPreviewHandler),
		QITABENT(CodePreviewHandler, IInitializeWithStream),
		QITABENT(CodePreviewHandler, IPreviewHandlerVisuals),
		QITABENT(CodePreviewHandler, IOleWindow),
		QITABENT(CodePreviewHandler, IObjectWithSite),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

// Increase the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) CodePreviewHandler::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

// Decrease the reference count for an interface on an object.
IFACEMETHODIMP_(ULONG) CodePreviewHandler::Release()
{
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (0 == cRef)
	{
		delete this;
	}

	return cRef;
}

#pragma endregion


#pragma region IInitializeWithStream

// Initializes the preview handler with a stream. 
// Store the IStream and mode parameters so that you can read the item's data 
// when you are ready to preview the item. Do not load the data in Initialize. 
// Load the data in IPreviewHandler::DoPreview just before you render.
IFACEMETHODIMP CodePreviewHandler::Initialize(IStream *pStream, DWORD grfMode)
{
	::OutputDebugStringW(L"► CodePreviewHandler::Initialize(...)\r\n");
	HRESULT hr = E_INVALIDARG;
	if (pStream)
	{
		// Initialize can be called more than once, so release existing valid 
		// m_pStream.
		if (m_pStream)
		{
			m_pStream->Release();
			m_pStream = NULL;
		}

		m_pStream = pStream;
		m_pStream->AddRef();
		hr = S_OK;
	}
	return hr;
}

#pragma endregion


#pragma region IPreviewHandler

// This method gets called when the previewer gets created. It sets the parent 
// window of the previewer window, as well as the area within the parent to be 
// used for the previewer window.
IFACEMETHODIMP CodePreviewHandler::SetWindow(HWND hwnd, const RECT *prc)
{
	if (hwnd && prc)
	{
		m_hwndParent = hwnd;  // Cache the HWND for later use
		m_rcParent = *prc;    // Cache the RECT for later use

		if (m_hwndPreview)
		{
			// Update preview window parent and rect information
			SetParent(m_hwndPreview, m_hwndParent);
			SetWindowPos(m_hwndPreview, NULL, m_rcParent.left, m_rcParent.top,
				RECTWIDTH(m_rcParent), RECTHEIGHT(m_rcParent),
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	return S_OK;
}

// Directs the preview handler to set focus to itself.
IFACEMETHODIMP CodePreviewHandler::SetFocus()
{
	HRESULT hr = S_FALSE;
	if (m_hwndPreview)
	{
		::SetFocus(m_hwndPreview);
		hr = S_OK;
	}
	return hr;
}

// Directs the preview handler to return the HWND from calling the GetFocus 
// function.
IFACEMETHODIMP CodePreviewHandler::QueryFocus(HWND *phwnd)
{
	HRESULT hr = E_INVALIDARG;
	if (phwnd)
	{
		*phwnd = ::GetFocus();
		if (*phwnd)
		{
			hr = S_OK;
		}
		else
		{
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
	}
	return hr;
}

// Directs the preview handler to handle a keystroke passed up from the 
// message pump of the process in which the preview handler is running.
HRESULT CodePreviewHandler::TranslateAccelerator(MSG *pmsg)
{
	HRESULT hr = S_FALSE;
	IPreviewHandlerFrame *pFrame = NULL;
	if (m_punkSite && SUCCEEDED(m_punkSite->QueryInterface(&pFrame)))
	{
		// If your previewer has multiple tab stops, you will need to do 
		// appropriate first/last child checking. This sample previewer has 
		// no tabstops, so we want to just forward this message out.
		hr = pFrame->TranslateAccelerator(pmsg);

		pFrame->Release();
	}
	return hr;
}

// This method gets called when the size of the previewer window changes 
// (user resizes the Reading Pane). It directs the preview handler to change 
// the area within the parent hwnd that it draws into.
IFACEMETHODIMP CodePreviewHandler::SetRect(const RECT *prc)
{
	::DebugPrintf(L"CodePreviewHandler::SetRect(...)\n");
	HRESULT hr = E_INVALIDARG;
	if (prc != NULL)
	{
		m_rcParent = *prc;
		if (m_hwndPreview)
		{
			// Preview window is already created, so set its size and position.
			SetWindowPos(m_hwndPreview, NULL, m_rcParent.left, m_rcParent.top,
				(m_rcParent.right - m_rcParent.left), // Width
				(m_rcParent.bottom - m_rcParent.top), // Height
				SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
		hr = S_OK;
	}
	return hr;
}

// The method directs the preview handler to load data from the source 
// specified in an earlier Initialize method call, and to begin rendering to 
// the previewer window.
IFACEMETHODIMP CodePreviewHandler::DoPreview()
{
	::DebugPrintf(L"CodePreviewHandler::DoPreview()\n");

	// Cannot call more than once.
	// (Unload should be called before another DoPreview)
	if (m_hwndPreview != NULL || !m_pStream)
	{
		return E_FAIL;
	}

	HRESULT hr = E_FAIL;

	if (m_hinstScintilla == NULL)
	{
		// load the lexer. The lexer should be in the same folder as the preview handler dll.
		wchar_t pszPathCurrent[MAX_PATH];
		wchar_t pszPathScintilla[MAX_PATH];
		::GetModuleFileNameW(g_hInst, pszPathCurrent, MAX_PATH);
		::PathRemoveFileSpecW(pszPathCurrent);
		::PathCombineW(pszPathScintilla, pszPathCurrent, L"scilexer.dll");
		::DebugPrintf(L"\t• Loading the lexer [%s]!\r\n", pszPathScintilla);
		m_hinstScintilla = LoadLibraryW(pszPathScintilla);
	}

	if (m_hinstScintilla == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		wchar_t pszError[1024];
		::FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, nullptr,
		                 GetLastError(), 0, pszError, 1024, nullptr);
		// the FormatMessage function appends the new line characters (\r\n) at the end
		// when the flag FORMAT_MESSAGE_FROM_SYSTEM is used. Simply remove them.
		while (pszError[lstrlenW(pszError) - 1] == '\r' || pszError[lstrlenW(pszError) - 1] == '\n')
			pszError[lstrlenW(pszError) - 1] = '\0';
		::DebugPrintf(L"\t• Failed to load the lexer. Error: [0x%X] [%s]\r\n", hr, pszError);
	}
	else
	{
		// Create the preview window.
		hr = CreatePreviewWindow();
		if (FAILED(hr))
		{
			// cleanup
			::OutputDebugString(TEXT("CodePreviewHandler::DoPreview()\n"));
			::OutputDebugString(TEXT("    TODO: add cleanup code.\n"));
		}
		else
		{
			STATSTG stat;
			m_pStream->Stat(&stat, STATFLAG_DEFAULT);
			ULONG bufSize = stat.cbSize.QuadPart + 32;
			ULONG readSize = 0;

			char* lpData = (char*)::GlobalAlloc(GPTR, bufSize);

			m_pStream->Read(lpData, bufSize, &readSize);
			SetPreviewText(lpData, readSize);
			::GlobalFree(lpData);
			hr = S_OK;
		}
	}

	return hr;
}

// This method gets called when a shell item is de-selected. It directs the 
// preview handler to cease rendering a preview and to release all resources 
// that have been allocated based on the item passed in during the 
// initialization.
HRESULT CodePreviewHandler::Unload()
{
	if (m_pStream)
	{
		m_pStream->Release();
		m_pStream = NULL;
	}

	if (m_hwndPreview)
	{
		DestroyWindow(m_hwndPreview);
		m_hwndPreview = NULL;
	}

	return S_OK;
}

#pragma endregion


#pragma region IPreviewHandlerVisuals (Optional)

// Sets the background color of the preview handler.
IFACEMETHODIMP CodePreviewHandler::SetBackgroundColor(COLORREF color)
{
	return S_OK;
}

// Sets the font attributes to be used for text within the preview handler.
IFACEMETHODIMP CodePreviewHandler::SetFont(const LOGFONTW *plf)
{
	return S_OK;
}

// Sets the color of the text within the preview handler.
IFACEMETHODIMP CodePreviewHandler::SetTextColor(COLORREF color)
{
	return S_OK;
}

#pragma endregion


#pragma region IOleWindow

// Retrieves a handle to one of the windows participating in in-place 
// activation (frame, document, parent, or in-place object window).
IFACEMETHODIMP CodePreviewHandler::GetWindow(HWND *phwnd)
{
	HRESULT hr = E_INVALIDARG;
	if (phwnd)
	{
		*phwnd = m_hwndParent;
		hr = S_OK;
	}
	return hr;
}

// Determines whether context-sensitive help mode should be entered during an 
// in-place activation session
IFACEMETHODIMP CodePreviewHandler::ContextSensitiveHelp(BOOL fEnterMode)
{
	return E_NOTIMPL;
}

#pragma endregion


#pragma region IObjectWithSite

// Provides the site's IUnknown pointer to the object.
IFACEMETHODIMP CodePreviewHandler::SetSite(IUnknown *punkSite)
{
	if (m_punkSite)
	{
		m_punkSite->Release();
		m_punkSite = NULL;
	}
	return punkSite ? punkSite->QueryInterface(&m_punkSite) : S_OK;
}

// Gets the last site set with IObjectWithSite::SetSite. If there is no known 
// site, the object returns a failure code.
IFACEMETHODIMP CodePreviewHandler::GetSite(REFIID riid, void **ppv)
{
	*ppv = NULL;
	return m_punkSite ? m_punkSite->QueryInterface(riid, ppv) : E_FAIL;
}

#pragma endregion


#pragma region Helper Functions


// Create the preview window based on the code information.
HRESULT CodePreviewHandler::CreatePreviewWindow()
{
	::DebugPrintf(L"CodePreviewHandler::CreatePreviewWindow()\r\n");

	assert(m_hwndPreview == NULL);
	assert(m_hwndParent != NULL);

	HRESULT hr = S_OK;

	// Create the preview window
	m_hwndPreview = CreateWindowExW(0,
		L"Scintilla", NULL,
		WS_CHILD | WS_VSCROLL | WS_VISIBLE | ES_MULTILINE | ES_READONLY,
		m_rcParent.left, m_rcParent.top, RECTWIDTH(m_rcParent), RECTHEIGHT(m_rcParent),
		m_hwndParent, (HMENU)IDC_SCINTILLA, NULL, NULL);

	if (m_hwndPreview == NULL)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
	}

	if (SUCCEEDED(hr))
	{
		// set folding
		SendMessage(m_hwndPreview, SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CLICK | SC_AUTOMATICFOLD_CHANGE, 0);
		SendMessage(m_hwndPreview, SCI_FOLDDISPLAYTEXTSETSTYLE, SC_FOLDDISPLAYTEXT_BOXED, 0);
		SendMessage(m_hwndPreview, SCI_SETFOLDFLAGS, SC_FOLDFLAG_LINEAFTER_CONTRACTED, 0);

		SendMessage(m_hwndPreview, SCI_SETMARGINWIDTHN, 2, 12);
		SendMessage(m_hwndPreview, SCI_SETFOLDMARGINCOLOUR, true, RGB(255, 255, 255));
		SendMessage(m_hwndPreview, SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
		SendMessage(m_hwndPreview, SCI_SETMARGINSENSITIVEN, 2, 1);
		SendMessage(m_hwndPreview, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
		SendMessage(m_hwndPreview, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
		SendMessage(m_hwndPreview, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
		SendMessage(m_hwndPreview, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
		SendMessage(m_hwndPreview, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
		SendMessage(m_hwndPreview, SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
		SendMessage(m_hwndPreview, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
		SendMessage(m_hwndPreview, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, RGB(255, 255, 255));
		SendMessage(m_hwndPreview, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, RGB(80, 80, 80));
		SendMessage(m_hwndPreview, SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, RGB(255, 255, 255));
		SendMessage(m_hwndPreview, SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, RGB(80, 80, 80));
		SendMessage(m_hwndPreview, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERSUB, RGB(255, 255, 255));
		SendMessage(m_hwndPreview, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, RGB(80, 80, 80));
		SendMessage(m_hwndPreview, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERTAIL, RGB(255, 255, 255));
		SendMessage(m_hwndPreview, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, RGB(80, 80, 80));
		SendMessage(m_hwndPreview, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERMIDTAIL, RGB(255, 255, 255));
		SendMessage(m_hwndPreview, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, RGB(80, 80, 80));
		SendMessage(m_hwndPreview, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPENMID, RGB(255, 255, 255));
		SendMessage(m_hwndPreview, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPENMID, RGB(80, 80, 80));
		SendMessage(m_hwndPreview, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEREND, RGB(255, 255, 255));
		SendMessage(m_hwndPreview, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEREND, RGB(80, 80, 80));



		// set default style
		SendMessage(m_hwndPreview, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas");
		SendMessage(m_hwndPreview, SCI_STYLESETSIZE, STYLE_DEFAULT, (LPARAM)12);
		//SendMessage(m_hwndPreview, SCI_STYLESETFORE, STYLE_DEFAULT, RGB(228, 35, 62));
		for (int i = 0; lexDefault.styles[i].iStyle != -1; ++i)
		{
			int color = lexDefault.styles[i].colFore;
			COLORREF rgb = RGB((color & 0xFF0000) >> 16, (color & 0xFF00) >> 8, color & 0xFF);
			SendMessage(m_hwndPreview, SCI_STYLESETFORE, lexDefault.styles[i].iStyle, rgb);

			color = lexDefault.styles[i].colBack;
			rgb = RGB((color & 0xFF0000) >> 16, (color & 0xFF00) >> 8, color & 0xFF);
			SendMessage(m_hwndPreview, SCI_STYLESETBACK, lexDefault.styles[i].iStyle, rgb);

			SendMessage(m_hwndPreview, SCI_STYLESETBOLD, lexDefault.styles[i].iStyle, lexDefault.styles[i].bold);
			SendMessage(m_hwndPreview, SCI_STYLESETITALIC, lexDefault.styles[i].iStyle, lexDefault.styles[i].italic);
			SendMessage(m_hwndPreview, SCI_STYLESETUNDERLINE, lexDefault.styles[i].iStyle, lexDefault.styles[i].undeline);
			SendMessage(m_hwndPreview, SCI_STYLESETEOLFILLED, lexDefault.styles[i].iStyle, lexDefault.styles[i].eolFill);
		}

		SendMessage(m_hwndPreview, SCI_STYLECLEARALL, 0, 0);// needs to be called after setting the default style
		SendMessage(m_hwndPreview, SCI_STYLESETSIZE, STYLE_LINENUMBER, (LPARAM)10);
		SendMessage(m_hwndPreview, SCI_STYLESETFORE, STYLE_LINENUMBER, (LPARAM)RGB(255, 0, 0));
		SendMessage(m_hwndPreview, SCI_SETWRAPMODE, SC_WRAP_WORD, 0);
		SendMessage(m_hwndPreview, SCI_SETMARGINWIDTHN, 1, 0);

		// set the cpp syntax highlight
		SetCppLexer();
		hr = S_OK;
	}

	return hr;
}


void CodePreviewHandler::SetCppLexer()
{
	SendMessage(m_hwndPreview, SCI_SETLEXER, SCLEX_CPP, 0);

	SendMessage(m_hwndPreview, SCI_CLEARDOCUMENTSTYLE, 0, 0);
	SendMessage(m_hwndPreview, SCI_STYLERESETDEFAULT, 0, 0);
	SendMessage(m_hwndPreview, SCI_STYLESETCHARACTERSET, STYLE_DEFAULT, (LPARAM)DEFAULT_CHARSET);

	SendMessage(m_hwndPreview, SCI_SETINDENTATIONGUIDES, SC_IV_LOOKBOTH, 0);


	// the properties must be set after the lexer is set
	SendMessage(m_hwndPreview, SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
	SendMessage(m_hwndPreview, SCI_SETPROPERTY, (WPARAM)"fold.compact", (LPARAM)"1");
	SendMessage(m_hwndPreview, SCI_SETPROPERTY, (WPARAM)"fold.preprocessor", (LPARAM)"1");
	SendMessage(m_hwndPreview, SCI_SETPROPERTY, (WPARAM)"fold.comment", (LPARAM)"1");
	SendMessage(m_hwndPreview, SCI_SETPROPERTY, (WPARAM)"fold.cpp.comment.multiline", (LPARAM)"1");
	SendMessage(m_hwndPreview, SCI_SETPROPERTY, (WPARAM)"styling.within.preprocessor", (LPARAM)"1");
	SendMessage(m_hwndPreview, SCI_SETPROPERTY, (WPARAM)"lexer.cpp.track.preprocessor", (LPARAM)"1");
	SendMessage(m_hwndPreview, SCI_SETPROPERTY, (WPARAM)"lexer.cpp.update.preprocessor", (LPARAM)"1");

	// set the cpp styles
	for (int i = 0; i <= KEYWORDSET_MAX; ++i)
	{
		::SendMessage(m_hwndPreview, SCI_SETKEYWORDS, i, (LPARAM)lexCpp.pKeywords->pszKeywords[i]);
	}

	for (int i = 0; lexCpp.styles[i].iStyle != -1; ++i)
	{
		int color = lexCpp.styles[i].colFore;
		COLORREF rgb = RGB((color & 0xFF0000) >> 16, (color & 0xFF00) >> 8, color & 0xFF);
		SendMessage(m_hwndPreview, SCI_STYLESETFORE, lexCpp.styles[i].iStyle, rgb);

		color = lexCpp.styles[i].colBack;
		rgb = RGB((color & 0xFF0000) >> 16, (color & 0xFF00) >> 8, color & 0xFF);
		SendMessage(m_hwndPreview, SCI_STYLESETBACK, lexCpp.styles[i].iStyle, rgb);

		SendMessage(m_hwndPreview, SCI_STYLESETBOLD,   lexCpp.styles[i].iStyle, lexCpp.styles[i].bold);
		SendMessage(m_hwndPreview, SCI_STYLESETITALIC, lexCpp.styles[i].iStyle, lexCpp.styles[i].italic);
		SendMessage(m_hwndPreview, SCI_STYLESETUNDERLINE, lexCpp.styles[i].iStyle, lexCpp.styles[i].undeline);
		SendMessage(m_hwndPreview, SCI_STYLESETEOLFILLED, lexCpp.styles[i].iStyle, lexCpp.styles[i].eolFill);
	}

	SendMessage(m_hwndPreview, SCI_STYLESETFORE, STYLE_BRACELIGHT, RGB(0xFF, 0x00, 0x00));
	SendMessage(m_hwndPreview, SCI_STYLESETBACK, STYLE_BRACELIGHT, RGB(0xFF, 0xDC, 0xDC));
	SendMessage(m_hwndPreview, SCI_STYLESETBOLD, STYLE_BRACELIGHT, (LPARAM)TRUE);

	// this is the style for disabled code by preprocessor instructions
	// the style is the default one +64
	SendMessage(m_hwndPreview, SCI_STYLESETFORE, SCE_C_WORD + 64, (LPARAM)RGB(185, 185, 255));
	SendMessage(m_hwndPreview, SCI_STYLESETFORE, SCE_C_STRING + 64, (LPARAM)RGB(200, 180, 180));
	SendMessage(m_hwndPreview, SCI_STYLESETFORE, SCE_C_NUMBER + 64, (LPARAM)RGB(255, 185, 185));
	SendMessage(m_hwndPreview, SCI_STYLESETFORE, SCE_C_OPERATOR + 64, (LPARAM)RGB(185, 185, 185));
	SendMessage(m_hwndPreview, SCI_STYLESETFORE, SCE_C_IDENTIFIER + 64, (LPARAM)RGB(185, 185, 185));
	SendMessage(m_hwndPreview, SCI_STYLESETFORE, SCE_C_COMMENT + 64, (LPARAM)RGB(150, 200, 150));
	SendMessage(m_hwndPreview, SCI_STYLESETFORE, SCE_C_COMMENTLINE + 64, (LPARAM)RGB(150, 200, 150));
	SendMessage(m_hwndPreview, SCI_STYLESETITALIC, SCE_C_COMMENT + 64, (LPARAM)TRUE);
	SendMessage(m_hwndPreview, SCI_STYLESETITALIC, SCE_C_COMMENTLINE + 64, (LPARAM)TRUE);


	SendMessage(m_hwndPreview, SCI_COLOURISE, 0, (LPARAM)-1);
}


void CodePreviewHandler::SetPreviewText(const char* pBuffer, size_t cb)
{
	int i = 0xFFFF;

	BOOL bIsTextUnicode = FALSE;

	BOOL bBOM;
	BOOL bReverse = FALSE;

	BOOL bHasBOM = FALSE;
	BOOL bHasRBOM = FALSE;

	if (pBuffer && cb >= 2)
	{
		bIsTextUnicode = IsTextUnicode(pBuffer, cb, &i);

		bHasBOM = (*((UNALIGNED PWCHAR)pBuffer) == 0xFEFF);
		bHasRBOM = (*((UNALIGNED PWCHAR)pBuffer) == 0xFFFE);

		if (i == 0xFFFF) // i doesn't seem to have been modified ...
			i = 0;

		if (bIsTextUnicode || bHasBOM || bHasRBOM ||
			((i & (IS_TEXT_UNICODE_UNICODE_MASK | IS_TEXT_UNICODE_REVERSE_MASK)) &&
				!((i & IS_TEXT_UNICODE_UNICODE_MASK) && (i & IS_TEXT_UNICODE_REVERSE_MASK)) &&
				!(i & IS_TEXT_UNICODE_ODD_LENGTH) &&
				!(i & IS_TEXT_UNICODE_ILLEGAL_CHARS && !(i & IS_TEXT_UNICODE_REVERSE_SIGNATURE)) &&
				!((i & IS_TEXT_UNICODE_REVERSE_MASK) == IS_TEXT_UNICODE_REVERSE_STATISTICS))) {

			if (bBOM)
				bBOM = (bHasBOM || bHasRBOM ||
				(i & (IS_TEXT_UNICODE_SIGNATURE | IS_TEXT_UNICODE_REVERSE_SIGNATURE)))
				? TRUE : FALSE;

			if (bReverse)
				bReverse = (bHasRBOM || (i & IS_TEXT_UNICODE_REVERSE_MASK)) ? TRUE : FALSE;
		}
		if (bIsTextUnicode)
		{
			char* lpDataUTF8 = (char*)::GlobalAlloc(GPTR, (cb * 3) + 2);
			cb = WideCharToMultiByte(CP_UTF8, 0, (bBOM) ? (LPWSTR)pBuffer + 1 : (LPWSTR)pBuffer,
				(bBOM) ? (cb) / sizeof(WCHAR) : cb / sizeof(WCHAR) + 1, lpDataUTF8, (int)GlobalSize(lpDataUTF8), NULL, NULL);

			if (cb == 0) {
				cb = WideCharToMultiByte(CP_ACP, 0, (bBOM) ? (LPWSTR)pBuffer + 1 : (LPWSTR)pBuffer,
					(-1), lpDataUTF8, (int)GlobalSize(lpDataUTF8), NULL, NULL);
				//*pbUnicodeErr = TRUE;
			}

			//put some text in the control
			SendMessage(m_hwndPreview, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
			SendMessage(m_hwndPreview, SCI_SETTEXT, NULL, (LPARAM)lpDataUTF8);
			UpdateLineNumberWidth();

			//SendMessage(hwnd, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
			//EditSetNewText(hwnd, "", 0);
			//FileVars_Init(lpDataUTF8, cbData - 1, &fvCurFile);
			//EditSetNewText(hwnd, lpDataUTF8, cbData - 1);
			//*iEOLMode = EditDetectEOLMode(hwnd, lpDataUTF8, cbData - 1);
			GlobalFree(lpDataUTF8);
		}
		else
		{
			SendMessage(m_hwndPreview, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
			SendMessage(m_hwndPreview, SCI_SETTEXT, NULL, (LPARAM)pBuffer);
			UpdateLineNumberWidth();
		}
		//SendMessage(m_hwndPreview, SCI_SETREADONLY, true, 0);
	}
}

void CodePreviewHandler::UpdateLineNumberWidth()
{
	char tchLines[32];
	int  iLineMarginWidthNow;
	int  iLineMarginWidthFit;

	wsprintfA(tchLines, "_%i_", SendMessage(m_hwndPreview, SCI_GETLINECOUNT, 0, 0));
	//wsprintfA(tchLines, "_999_", SendMessage(hWnd, SCI_GETLINECOUNT, 0, 0));

	iLineMarginWidthNow = (int)SendMessage(m_hwndPreview, SCI_GETMARGINWIDTHN, 0, 0);
	iLineMarginWidthFit = (int)SendMessage(m_hwndPreview, SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)tchLines);

	if (iLineMarginWidthNow != iLineMarginWidthFit) {
		//SendMessage(hwndEdit,SCI_SETMARGINWIDTHN,0,0);
		SendMessage(m_hwndPreview, SCI_SETMARGINWIDTHN, 0, iLineMarginWidthFit);
	}
}
#pragma endregion
