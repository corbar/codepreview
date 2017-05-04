#pragma comment(lib, "shlwapi.lib")

#include <shlwapi.h>
#include <shobjidl.h>   // IPreviewHandler, IShellItem, IInitializeWithItem, IParentAndItem
#include <richedit.h>   // MSFTEDIT_CLASS
#include <assert.h>     // for assert
#include <new>
#include "../scintilla/include/Scintilla.h"
#include "../scintilla/include/SciLexer.h"


#define IDC_SCINTILLA 0xC0FF
void SetCppLexer(HWND);
void UpdateLineNumberWidth(HWND);
void LoadControlContent(HWND hWnd, LPCWSTR lpszFile);

/* The codepage is not determined!*/
#define CPI_NONE              -1
#define CPI_DEFAULT            0
/*  "Original Equipment Manufacturer" code page.*/
#define CPI_OEM                1
#define CPI_UNICODEBOM         2
#define CPI_UNICODEBEBOM       3
#define CPI_UNICODE            4
#define CPI_UNICODEBE          5
#define CPI_UTF8               6
#define CPI_UTF8SIGN           7
#define CPI_UTF7               8


char* todos[] = {
	{ "TODO: when there is folded code, add some grey text to signal the folding, on top of the horizontal line" },
	{ "TODO: set a color to macros" },
	{ "TODO: for included headers, color differently the standard ones " },
	{ "TODO: when folding, shouldn't include the spaces after the folded region" },
	{ "TODO: make the text readonly" },
	{ "TODO: highlight the current line" },
	{ "TODO: add a shortcut to beautify the text" },
	{ "TODO: " },
};




template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

inline int RECTWIDTH(const RECT &rc)
{
    return (rc.right - rc.left);
}

inline int RECTHEIGHT(const RECT &rc )
{
    return (rc.bottom - rc.top);
}

class CCodePreviewHandler : public IObjectWithSite,
                              public IPreviewHandler,
                              public IOleWindow,
                              public IInitializeWithStream
{
public:
    CCodePreviewHandler() : _cRef(1), _hwndParent(NULL), _hwndPreview(NULL),  _hinstEditLibrary(NULL),
        _pStyleSheetNode(NULL), _pStream(NULL), _punkSite(NULL)
    {
    }

    virtual ~CCodePreviewHandler()
    {
        if (_hwndPreview)
        {
            DestroyWindow(_hwndPreview);
        }
        if (_hinstEditLibrary)
        {
            FreeLibrary(_hinstEditLibrary);
        }
        SafeRelease(&_punkSite);
        SafeRelease(&_pStream);
        SafeRelease(&_pStyleSheetNode);
    }

    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void **ppv)
    {
        *ppv = NULL;
        static const QITAB qit[] =
        {
            QITABENT(CCodePreviewHandler, IObjectWithSite),
            QITABENT(CCodePreviewHandler, IOleWindow),
            QITABENT(CCodePreviewHandler, IInitializeWithStream),
            QITABENT(CCodePreviewHandler, IPreviewHandler),
            { 0 },
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    IFACEMETHODIMP_(ULONG) Release()
    {
        ULONG cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
        {
            delete this;
        }
        return cRef;
    }

    // IObjectWithSite
    IFACEMETHODIMP SetSite(IUnknown *punkSite);
    IFACEMETHODIMP GetSite(REFIID riid, void **ppv);

    // IPreviewHandler
    IFACEMETHODIMP SetWindow(HWND hwnd, const RECT *prc);
    IFACEMETHODIMP SetFocus();
    IFACEMETHODIMP QueryFocus(HWND *phwnd);
    IFACEMETHODIMP TranslateAccelerator(MSG *pmsg);
    IFACEMETHODIMP SetRect(const RECT *prc);
    IFACEMETHODIMP DoPreview();
    IFACEMETHODIMP Unload();

    // IOleWindow
    IFACEMETHODIMP GetWindow(HWND *phwnd);
    IFACEMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(IStream *pStream, DWORD grfMode);

private:
    //HRESULT _CreatePreviewWindow(PCWSTR pszRtf);
	HRESULT _CreatePreviewWindow();
    HRESULT _CreateStyleSheetNode();

	void SetPreviewText(const char* pBuffer, size_t cb);

    long _cRef;
    HWND                    _hwndParent;        // parent window that hosts the previewer window; do NOT DestroyWindow this
    RECT                    _rcParent;          // bounding rect of the parent window
    HWND                    _hwndPreview;       // the actual previewer window
    HINSTANCE               _hinstEditLibrary;  // the library that lets us create a richedit control
    IUnknown                *_punkSite;         // site pointer from host, used to get _pFrame
    IXMLDOMNode             *_pStyleSheetNode;  // an xml node representing the style-sheet to use for formatting
    IStream                 *_pStream;          // the stream for the file we are previewing
};

// IPreviewHandler
// This method gets called when the previewer gets created
HRESULT CCodePreviewHandler::SetWindow(HWND hwnd, const RECT *prc)
{
	::OutputDebugString(TEXT("CCodePreviewHandler::SetWindow(...)\n"));
    if (hwnd && prc)
    {
        _hwndParent = hwnd; // cache the HWND for later use
        _rcParent   = *prc; // cache the RECT for later use

        if (_hwndPreview)
        {
            // Update preview window parent and rect information
            SetParent(_hwndPreview, _hwndParent);
            SetWindowPos(_hwndPreview, NULL, _rcParent.left, _rcParent.top,
                RECTWIDTH(_rcParent), RECTHEIGHT(_rcParent), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
    }
    return S_OK;
}

HRESULT CCodePreviewHandler::SetFocus()
{
	::OutputDebugString(TEXT("CCodePreviewHandler::SetFocus()\n"));
    HRESULT hr = S_FALSE;
    if (_hwndPreview)
    {
        ::SetFocus(_hwndPreview);
        hr = S_OK;
    }
    return hr;
}

HRESULT CCodePreviewHandler::QueryFocus(HWND *phwnd)
{
	::OutputDebugString(TEXT("CCodePreviewHandler::QueryFocus(...)\n"));
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

HRESULT CCodePreviewHandler::TranslateAccelerator(MSG *pmsg)
{
	::OutputDebugString(TEXT("CCodePreviewHandler::TranslateAccelerator(...)\n"));
    HRESULT hr = S_FALSE;
    IPreviewHandlerFrame *pFrame = NULL;
    if (_punkSite && SUCCEEDED(_punkSite->QueryInterface(&pFrame)))
    {
        // If your previewer has multiple tab stops, you will need to do appropriate first/last child checking.
        // This particular sample previewer has no tabstops, so we want to just forward this message out
        hr = pFrame->TranslateAccelerator(pmsg);
        SafeRelease(&pFrame);
    }
    return hr;
}

// This method gets called when the size of the previewer window changes (user resizes the Reading Pane)
HRESULT CCodePreviewHandler::SetRect(const RECT *prc)
{
	::OutputDebugString(TEXT("CCodePreviewHandler::SetRect(...)\n"));
    HRESULT hr = E_INVALIDARG;
    if (prc)
    {
        _rcParent = *prc;
        if (_hwndPreview)
        {
            // Preview window is already created, so set its size and position
            SetWindowPos(_hwndPreview, NULL, _rcParent.left, _rcParent.top,
                RECTWIDTH(_rcParent), RECTHEIGHT(_rcParent), SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        }
        hr = S_OK;
    }

    return hr;
}

// CPI codepage information

// The main method that extracts relevant information from the file stream and
// draws content to the previewer window
HRESULT CCodePreviewHandler::DoPreview()
{
	::OutputDebugString(TEXT("CCodePreviewHandler::DoPreview()\n"));
    HRESULT hr = E_FAIL;
    if (_hwndPreview == NULL && _pStream) // cannot call more than once (Unload should be called before another DoPreview)
    {
		STATSTG stat;
		_pStream->Stat(&stat, STATFLAG_DEFAULT);
		ULONG bufSize = stat.cbSize.QuadPart + 32;
		ULONG readSize = 0;
		
		char* lpData = (char*)::GlobalAlloc(GPTR, bufSize);

		_pStream->Read(lpData, bufSize, &readSize);
		hr = _CreatePreviewWindow();
		//SendMessage(_hwndPreview, SCI_SETTEXT, NULL, (LPARAM)lpData);
		SetPreviewText(lpData, readSize);
		::GlobalFree(lpData);
		hr = S_OK;
    }
    return hr;
}

// This method gets called when a shell item is de-selected in the listview
HRESULT CCodePreviewHandler::Unload()
{
	::OutputDebugString(TEXT("CCodePreviewHandler::Unload()\n"));
    SafeRelease(&_pStream);
    if (_hwndPreview)
    {
        DestroyWindow(_hwndPreview);
        _hwndPreview = NULL;
    }
    return S_OK;
}

// IObjectWithSite methods
HRESULT CCodePreviewHandler::SetSite(IUnknown *punkSite)
{
	::OutputDebugString(TEXT("CCodePreviewHandler::SetSite(...)\n"));
    SafeRelease(&_punkSite);
    return punkSite ? punkSite->QueryInterface(&_punkSite) : S_OK;
}

HRESULT CCodePreviewHandler::GetSite(REFIID riid, void **ppv)
{
	::OutputDebugString(TEXT("CCodePreviewHandler::GetSite(...)\n"));
    *ppv = NULL;
    return _punkSite ? _punkSite->QueryInterface(riid, ppv) : E_FAIL;
}

// IOleWindow methods
HRESULT CCodePreviewHandler::GetWindow(HWND* phwnd)
{
	::OutputDebugString(TEXT("CCodePreviewHandler::GetWindow(...)\n"));
    HRESULT hr = E_INVALIDARG;
    if (phwnd)
    {
        *phwnd = _hwndParent;
        hr = S_OK;
    }
    return hr;
}

HRESULT CCodePreviewHandler::ContextSensitiveHelp(BOOL)
{
	::OutputDebugString(TEXT("CCodePreviewHandler::ContextSensitiveHelp(...)\n"));
    return E_NOTIMPL;
}

// IInitializeWithStream methods
// This method gets called when an item gets selected in listview
HRESULT CCodePreviewHandler::Initialize(IStream *pStream, DWORD)
{
	::OutputDebugString(TEXT("CCodePreviewHandler::Initialize(...)\n"));
    HRESULT hr = E_INVALIDARG;
    if (pStream)
    {
        // Initialize can be called more than once, so release existing valid _pStream
        SafeRelease(&_pStream);

        _pStream = pStream;
        _pStream->AddRef();
        hr = S_OK;
    }

    return hr;
}

extern HINSTANCE g_hInst;
// Helper method to post text content to the previewer window
HRESULT CCodePreviewHandler::_CreatePreviewWindow()
{
	::OutputDebugString(TEXT("CCodePreviewHandler::_CreatePreviewWindow(...)\n"));
    assert(_hwndPreview == NULL);

    HRESULT hr = E_FAIL;
    if (_hinstEditLibrary == NULL)
    {
        // MSFTEDIT_CLASS used below comes from this binary
        //_hinstEditLibrary = LoadLibraryW(L"msftedit.dll");
		_hinstEditLibrary = LoadLibraryW(L"c:\\Users\\cornel.barna\\Documents\\Visual Studio 2015\\Projects\\Preview Handlers\\x64\\Debug\\scilexer.dll");
    }

    if (_hinstEditLibrary == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {
		// Create the preview window
		_hwndPreview = CreateWindowExW(0,
			                           L"Scintilla", NULL,
			                           WS_CHILD | WS_VSCROLL | WS_VISIBLE | ES_MULTILINE | ES_READONLY,
			                           _rcParent.left, _rcParent.top, RECTWIDTH(_rcParent), RECTHEIGHT(_rcParent), 
			                           _hwndParent, (HMENU)IDC_SCINTILLA, NULL, NULL);

        if (_hwndPreview == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
			SendMessage(_hwndPreview, SCI_SETLEXER, SCLEX_CPP, 0);
			SendMessage(_hwndPreview, SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
			SendMessage(_hwndPreview, SCI_SETPROPERTY, (WPARAM)"fold.compact", (LPARAM)"1");
			SendMessage(_hwndPreview, SCI_SETPROPERTY, (WPARAM)"fold.preprocessor", (LPARAM)"1");
			SendMessage(_hwndPreview, SCI_SETPROPERTY, (WPARAM)"fold.comment", (LPARAM)"1");
			SendMessage(_hwndPreview, SCI_SETPROPERTY, (WPARAM)"fold.cpp.comment.multiline", (LPARAM)"1");

			SendMessage(_hwndPreview, SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CLICK | SC_AUTOMATICFOLD_CHANGE, 0);
			SendMessage(_hwndPreview, SCI_FOLDDISPLAYTEXTSETSTYLE, SC_FOLDDISPLAYTEXT_BOXED, 0);
			SendMessage(_hwndPreview, SCI_SETFOLDFLAGS, SC_FOLDFLAG_LINEAFTER_CONTRACTED, 0);
			SendMessage(_hwndPreview, SCI_TOGGLEFOLDSHOWTEXT, 0, (LPARAM)"SOME TEXT");

			SendMessage(_hwndPreview, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas");
			SendMessage(_hwndPreview, SCI_STYLESETSIZE, STYLE_DEFAULT, (LPARAM)12);
			SendMessage(_hwndPreview, SCI_STYLECLEARALL, 0, 0);// needs to be called after setting the default style
			SendMessage(_hwndPreview, SCI_STYLESETSIZE, STYLE_LINENUMBER, (LPARAM)10);
			SendMessage(_hwndPreview, SCI_STYLESETFORE, STYLE_LINENUMBER, (LPARAM)RGB(255, 0, 0));
			SendMessage(_hwndPreview, SCI_SETWRAPMODE, SC_WRAP_WORD, 0);
			SendMessage(_hwndPreview, SCI_SETMARGINWIDTHN, 1, 0);

			// set the cpp syntax highlight
			SetCppLexer(_hwndPreview);
			hr = S_OK;
        }
    }
    return hr;
}

// Helper method to create an xml node from the an xml string that represents the style-sheet
HRESULT CCodePreviewHandler::_CreateStyleSheetNode()
{
    IXMLDOMDocument *pDomDoc;
    HRESULT hr = CoCreateInstance(__uuidof(DOMDocument60), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDomDoc));
    if (SUCCEEDED(hr))
    {
        // The style-sheet to use for formatting xml as rich text
        // We do this only once
        BSTR bstrStyleSheet = SysAllocString(L"<xsl:stylesheet version=\"1.0\" xmlns:xsl=\"http://www.w3.org/1999/XSL/Transform\">\n"
                                             L"    <xsl:output method=\"text\" version=\"1.0\" encoding=\"UTF-8\" indent=\"no\"/>\n"
                                             L"    <xsl:template match=\"Recipe\">{\\rtf1\\ansi\\ansicpg1252\\deff0\\deflang1033{\\fonttbl{\\f0\\fswiss\\fprq2\\fcharset0 Segoe UI;}{\\f1\\fnil\\fprq2\\fcharset0 Segoe Print;}}\\viewkind4\\uc1\\pard\\f0\\fs36 <xsl:value-of select=\"Title\"/>\\par\n"
                                             L"\\fs20 <xsl:value-of select=\"Background/Author\"/>\\par\n"
                                             L"<xsl:value-of select=\"Comments\"/>\\f1\\par\n"
                                             L"\\par\n"
                                             L"\\b\\f0 Ingredients:\\par\n"
                                             L"\\pard\\tx270\\tx720\\b0\n"
                                             L"<xsl:for-each select=\"Ingredients/Item\">\n"
                                             L"<xsl:value-of select=\"@Quantity\"/>\\tab <xsl:value-of select=\"@Unit\"/>\\tab <xsl:value-of select=\".\"/>\\par\n"
                                             L"</xsl:for-each>\n"
                                             L"\\pard\\par\n"
                                             L"\\b Directions:\\par\n"
                                             L"\\b0\n"
                                             L"<xsl:for-each select=\"Directions/Step\">\n"
                                             L"<xsl:value-of select=\".\"/>\\par\n"
                                             L"\\par\n"
                                             L"</xsl:for-each>\n"
                                             L"Yield: <xsl:value-of select=\"RecipeInfo/Yield\"/>\\par\n"
                                             L"Preparation Time: <xsl:value-of select=\"RecipeInfo/PreparationTime\"/>\\par\n"
                                             L"Cook Time: <xsl:value-of select=\"RecipeInfo/CookTime\"/>\\par\n"
                                             L"}\n"
                                             L"    </xsl:template>\n"
                                             L"</xsl:stylesheet>\n");

        if (bstrStyleSheet == NULL)
        {
            hr = E_OUTOFMEMORY;
        }
        else
        {
            VARIANT_BOOL vfSuccess = VARIANT_FALSE;
            hr = pDomDoc->loadXML(bstrStyleSheet, &vfSuccess);
            if (vfSuccess != VARIANT_TRUE)
            {
                hr = FAILED(hr) ? hr : E_FAIL; // keep failed hr
            }

            if (SUCCEEDED(hr))
            {
                hr = pDomDoc->QueryInterface(&_pStyleSheetNode);
            }
            SysFreeString(bstrStyleSheet);
        }
        pDomDoc->Release();
    }
    return hr;
}

HRESULT CCodePreviewHandler_CreateInstance(REFIID riid, void **ppv)
{
    *ppv = NULL;

    CCodePreviewHandler *pNew = new (std::nothrow) CCodePreviewHandler();
    HRESULT hr = pNew ? S_OK : E_OUTOFMEMORY;
    if (SUCCEEDED(hr))
    {
        hr = pNew->QueryInterface(riid, ppv);
        pNew->Release();
    }
    return hr;
}




















void CCodePreviewHandler::SetPreviewText(const char* pBuffer, size_t cb)
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
			SendMessage(_hwndPreview, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
			SendMessage(_hwndPreview, SCI_SETTEXT, NULL, (LPARAM)lpDataUTF8);
			UpdateLineNumberWidth(_hwndPreview);

			//SendMessage(hwnd, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
			//EditSetNewText(hwnd, "", 0);
			//FileVars_Init(lpDataUTF8, cbData - 1, &fvCurFile);
			//EditSetNewText(hwnd, lpDataUTF8, cbData - 1);
			//*iEOLMode = EditDetectEOLMode(hwnd, lpDataUTF8, cbData - 1);
			GlobalFree(lpDataUTF8);
		}
		else
		{
			SendMessage(_hwndPreview, SCI_SETCODEPAGE, SC_CP_UTF8, 0);
			SendMessage(_hwndPreview, SCI_SETTEXT, NULL, (LPARAM)pBuffer);
			UpdateLineNumberWidth(_hwndPreview);
		}
	}
}

void LoadControlContent(HWND hWnd, LPCWSTR lpszFile)
{
	char* lpData = nullptr;
	DWORD  dwFileSize;
	DWORD  dwBufSize;
	DWORD  dwReadSize;

	HANDLE hFile = CreateFile(lpszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	dwFileSize = GetFileSize(hFile, NULL);
	dwBufSize = dwFileSize + 32;
	lpData = new char[dwBufSize];
	memset(lpData, 0, dwBufSize);
	ReadFile(hFile, lpData, dwBufSize, &dwReadSize, NULL);
	CloseHandle(hFile);
	SendMessage(hWnd, SCI_ADDTEXT, 4, (LPARAM)"/*\r\n");
	for (int i = 0; i < sizeof(todos) / sizeof(todos[0]); ++i)
	{
		SendMessage(hWnd, SCI_ADDTEXT, 4, (LPARAM)" *  ");
		SendMessage(hWnd, SCI_ADDTEXT, strlen(todos[i]), (LPARAM)todos[i]);
		SendMessage(hWnd, SCI_ADDTEXT, 2, (LPARAM)"\r\n");
	}
	SendMessage(hWnd, SCI_ADDTEXT, 5, (LPARAM)" */\r\n");
	SendMessage(hWnd, SCI_ADDTEXT, dwReadSize, (LPARAM)lpData);

	//SendMessage(hWnd, SCI_SETTEXT, NULL, (LPARAM)lpData);
	delete[] lpData;
}

void UpdateLineNumberWidth(HWND hWnd)
{
	char tchLines[32];
	int  iLineMarginWidthNow;
	int  iLineMarginWidthFit;

	wsprintfA(tchLines, "_%i_", SendMessage(hWnd, SCI_GETLINECOUNT, 0, 0));
	//wsprintfA(tchLines, "_999_", SendMessage(hWnd, SCI_GETLINECOUNT, 0, 0));

	iLineMarginWidthNow = (int)SendMessage(hWnd, SCI_GETMARGINWIDTHN, 0, 0);
	iLineMarginWidthFit = (int)SendMessage(hWnd, SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)tchLines);

	int a0 = (int)SendMessage(hWnd, SCI_GETMARGINWIDTHN, 0, 0);
	int a1 = (int)SendMessage(hWnd, SCI_GETMARGINWIDTHN, 1, 0);
	int a2 = (int)SendMessage(hWnd, SCI_GETMARGINWIDTHN, 2, 0);
	int a3 = (int)SendMessage(hWnd, SCI_GETMARGINWIDTHN, 3, 0);
	int a4 = (int)SendMessage(hWnd, SCI_GETMARGINWIDTHN, 4, 0);
	int a5 = (int)SendMessage(hWnd, SCI_GETMARGINWIDTHN, 5, 0);
	int a6 = (int)SendMessage(hWnd, SCI_GETMARGINWIDTHN, 6, 0);
	int a7 = (int)SendMessage(hWnd, SCI_GETMARGINWIDTHN, 7, 0);


	if (iLineMarginWidthNow != iLineMarginWidthFit) {
		//SendMessage(hwndEdit,SCI_SETMARGINWIDTHN,0,0);
		SendMessage(hWnd, SCI_SETMARGINWIDTHN, 0, iLineMarginWidthFit);
	}
}

/*
typedef struct _editstyle
{
union
{
INT32 iStyle;
UINT8 iStyle8[4];
};
int rid;
WCHAR* pszName;
WCHAR* pszDefault;
WCHAR  szValue[128];
} EDITSTYLE;

typedef struct _editlexer
{
int iLexer;
int rid;
WCHAR* pszName;
WCHAR* pszDefExt;
WCHAR  szExtensions[128];
PKEYWORDLIST pKeyWords;
EDITSTYLE    Styles[];
} EDITLEXER;

KEYWORDLIST KeyWords_CPP = {
"__abstract __alignof __asm __assume __based __box __cdecl __declspec __delegate __event "
"__except __except__try __fastcall __finally __forceinline __gc __hook __identifier "
"__if_exists __if_not_exists __inline __int16 __int32 __int64 __int8 __interface __leave "
"__m128 __m128d __m128i __m64 __multiple_inheritance __nogc __noop __pin __property __raise "
"__sealed __single_inheritance __stdcall __super __try __try_cast __unhook __uuidof __value "
"__virtual_inheritance __wchar_t auto bool break case catch char class const const_cast "
"continue default defined delete do double dynamic_cast else enum explicit extern false float "
"for friend goto if inline int long mutable naked namespace new operator private protected "
"public register reinterpret_cast return short signed size_t sizeof static static_cast struct "
"switch template this throw true try typedef typeid typename union unsigned using uuid "
"virtual void volatile wchar_t while",
"",
"", "", "", "", "", "", "" };


EDITLEXER lexCPP = { SCLEX_CPP, 63004, L"C/C++ Source Code", L"c; cpp; cxx; cc; h; hpp; hxx; hh; m; mm; idl; inl; odl", L"", &KeyWords_CPP, {
{ STYLE_DEFAULT, 63126, L"Default", L"", L"" },
//{ SCE_C_DEFAULT, L"Default", L"", L"" },
{ SCE_C_COMMENT, 63127, L"Comment", L"fore:#008000", L"" },
{ SCE_C_WORD, 63128, L"Keyword", L"bold; fore:#0A246A", L"" },
{ SCE_C_IDENTIFIER, 63129, L"Identifier", L"", L"" },
{ MULTI_STYLE(SCE_C_STRING,SCE_C_CHARACTER,SCE_C_STRINGEOL,SCE_C_VERBATIM), 63131, L"String", L"fore:#008000", L"" },
{ SCE_C_NUMBER, 63130, L"Number", L"fore:#FF0000", L"" },
{ SCE_C_OPERATOR, 63132, L"Operator", L"fore:#B000B0", L"" },
{ SCE_C_PREPROCESSOR, 63133, L"Preprocessor", L"fore:#FF8000", L"" },
//{ SCE_C_UUID, L"UUID", L"", L"" },
//{ SCE_C_REGEX, L"Regex", L"", L"" },
//{ SCE_C_WORD2, L"Word 2", L"", L"" },
//{ SCE_C_GLOBALCLASS, L"Global Class", L"", L"" },
{ -1, 00000, L"", L"", L"" } } };
*/

// font SCI_STYLESETFONT
// size SCI_STYLESETSIZE
// fore SCI_STYLESETFORE
// back SCI_STYLESETBACK
// bold SCI_STYLESETBOLD
// ital SCI_STYLESETITALIC
// unde SCI_STYLESETUNDERLINE
// eolf SCI_STYLESETEOLFILLED
// case SCI_STYLESETCASE
// char SCI_STYLESETCHARACTERSET
void SetCppLexer(HWND hwnd)
{
	//	int iStyleBits = 0;
	//	iStyleBits = (int)SendMessage(hwnd, SCI_GETSTYLEBITSNEEDED, 0, 0);
	//	SendMessage(hwnd, SCI_SETSTYLEBITS, (WPARAM)iStyleBits, 0);

	SendMessage(hwnd, SCI_CLEARDOCUMENTSTYLE, 0, 0);
	SendMessage(hwnd, SCI_STYLERESETDEFAULT, 0, 0);
	SendMessage(hwnd, SCI_STYLESETCHARACTERSET, STYLE_DEFAULT, (LPARAM)DEFAULT_CHARSET);

	SendMessage(hwnd, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas");

	SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM)"styling.within.preprocessor", (LPARAM)"1");
	SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM)"lexer.cpp.track.preprocessor", (LPARAM)"1");
	SendMessage(hwnd, SCI_SETPROPERTY, (WPARAM)"lexer.cpp.update.preprocessor", (LPARAM)"1");

	SendMessage(hwnd, SCI_SETINDENTATIONGUIDES, SC_IV_LOOKBOTH, 0);

	// folding
	SendMessage(hwnd, SCI_SETMARGINWIDTHN, 2, 12);
	SendMessage(hwnd, SCI_SETFOLDMARGINCOLOUR, true, RGB(255, 255, 255));
	SendMessage(hwnd, SCI_SETMARGINMASKN, 2, SC_MASK_FOLDERS);
	SendMessage(hwnd, SCI_SETMARGINSENSITIVEN, 2, 1); // FIXME: doesn't work
	SendMessage(hwnd, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_BOXPLUSCONNECTED);
	SendMessage(hwnd, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_BOXMINUSCONNECTED);
	SendMessage(hwnd, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNER);
	SendMessage(hwnd, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNER);
	SendMessage(hwnd, SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
	SendMessage(hwnd, SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_BOXPLUS);
	SendMessage(hwnd, SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_BOXMINUS);
	SendMessage(hwnd, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPEN, RGB(255, 255, 255));
	SendMessage(hwnd, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPEN, RGB(80, 80, 80));
	SendMessage(hwnd, SCI_MARKERSETFORE, SC_MARKNUM_FOLDER, RGB(255, 255, 255));
	SendMessage(hwnd, SCI_MARKERSETBACK, SC_MARKNUM_FOLDER, RGB(80, 80, 80));
	SendMessage(hwnd, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERSUB, RGB(255, 255, 255));
	SendMessage(hwnd, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, RGB(80, 80, 80));
	SendMessage(hwnd, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERTAIL, RGB(255, 255, 255));
	SendMessage(hwnd, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, RGB(80, 80, 80));
	SendMessage(hwnd, SCI_MARKERSETFORE, SC_MARKNUM_FOLDERMIDTAIL, RGB(255, 255, 255));
	SendMessage(hwnd, SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, RGB(80, 80, 80));
	SendMessage(hwnd, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEROPENMID, RGB(255, 255, 255));
	SendMessage(hwnd, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEROPENMID, RGB(80, 80, 80));
	SendMessage(hwnd, SCI_MARKERSETFORE, SC_MARKNUM_FOLDEREND, RGB(255, 255, 255));
	SendMessage(hwnd, SCI_MARKERSETBACK, SC_MARKNUM_FOLDEREND, RGB(80, 80, 80));





	// Primary keywords and identifiers. STATE: SCE_C_WORD
	SendMessage(hwnd, SCI_SETKEYWORDS, 0, (LPARAM)"int char long void double float bool return true false namespace const using if else while new delete nothrow class struct private protected public extern static");
	// Secondary keywords and identifiers. STATE: SCE_C_WORD2
	SendMessage(hwnd, SCI_SETKEYWORDS, 1, (LPARAM)"strlen strcpy strncpy strcat");
	// Documentation comment keywords. STATE: SCE_C_COMMENTDOCKEYWORD
	SendMessage(hwnd, SCI_SETKEYWORDS, 2, (LPARAM)"");
	// Global classes and typedefs. STATE: SCE_C_GLOBALCLASS
	SendMessage(hwnd, SCI_SETKEYWORDS, 3, (LPARAM)"istream ostream ofstream ifstream fstream ios string std vector");
	// Preprocessor definitions
	SendMessage(hwnd, SCI_SETKEYWORDS, 4, (LPARAM)"");
	// Task marker and error marker keywords. STATE: SCE_C_TASKMARKER
	SendMessage(hwnd, SCI_SETKEYWORDS, 5, (LPARAM)"TODO FIXME");

	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_WORD, (LPARAM)RGB(0, 0, 255));
	SendMessage(hwnd, SCI_STYLESETBOLD, SCE_C_WORD, (LPARAM)TRUE);

	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_GLOBALCLASS, (LPARAM)RGB(45, 145, 175));

	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_WORD2, (LPARAM)RGB(51, 153, 102));

	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_TASKMARKER, (LPARAM)RGB(255, 0, 0));
	SendMessage(hwnd, SCI_STYLESETBACK, SCE_C_TASKMARKER, (LPARAM)RGB(255, 220, 220));
	SendMessage(hwnd, SCI_STYLESETBOLD, SCE_C_TASKMARKER, (LPARAM)TRUE);
	SendMessage(hwnd, SCI_STYLESETITALIC, SCE_C_TASKMARKER, (LPARAM)FALSE);

	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_PREPROCESSOR, (LPARAM)RGB(128, 128, 137));
	//SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_PREPROCESSORCOMMENT, (LPARAM)RGB(255, 0, 0));
	//SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_PREPROCESSORCOMMENTDOC, (LPARAM)RGB(255, 0, 0));

	// comments that start with "/*"
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_COMMENT, (LPARAM)RGB(0, 128, 0));
	SendMessage(hwnd, SCI_STYLESETITALIC, SCE_C_COMMENT, (LPARAM)TRUE);
	// comments that start with "//"
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_COMMENTLINE, (LPARAM)RGB(0, 128, 0));
	SendMessage(hwnd, SCI_STYLESETITALIC, SCE_C_COMMENTLINE, (LPARAM)TRUE);
	// comments that start with "/**" or "/*!"
	SendMessage(hwnd, SCI_STYLESETFORE,   SCE_C_COMMENTDOC, (LPARAM)RGB(0x00, 0x80, 0x00));
	SendMessage(hwnd, SCI_STYLESETITALIC, SCE_C_COMMENTDOC, (LPARAM)TRUE);

	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_PREPROCESSORCOMMENT, (LPARAM)RGB(51, 153, 102));
	SendMessage(hwnd, SCI_STYLESETITALIC, SCE_C_PREPROCESSORCOMMENT, (LPARAM)TRUE);
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_PREPROCESSORCOMMENTDOC, (LPARAM)RGB(0, 128, 0));
	SendMessage(hwnd, SCI_STYLESETITALIC, SCE_C_PREPROCESSORCOMMENTDOC, (LPARAM)TRUE);

	// bad colors
	//SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_OPERATOR, (LPARAM)RGB(255, 102, 0));
	//SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_NUMBER, (LPARAM)RGB(51, 153, 102));
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_NUMBER, (LPARAM)RGB(255, 0, 0));
	SendMessage(hwnd, SCI_STYLESETBOLD, SCE_C_NUMBER, (LPARAM)TRUE);

	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_OPERATOR, (LPARAM)RGB(153, 0, 0));

	// properly created string
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_STRING, (LPARAM)RGB(163, 21, 21));
	// a single character
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_CHARACTER, (LPARAM)RGB(163, 21, 21));
	// this is the bad string (unclosed quotations)
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_STRINGEOL, (LPARAM)RGB(163, 21, 21));
	// string that have @ in front of them (INCOMPLETE)
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_VERBATIM, (LPARAM)RGB(163, 21, 21));

	SendMessage(hwnd, SCI_STYLESETFORE, STYLE_BRACELIGHT, (LPARAM)RGB(255, 0, 0));
	SendMessage(hwnd, SCI_STYLESETBACK, STYLE_BRACELIGHT, (LPARAM)RGB(255, 220, 220));
	SendMessage(hwnd, SCI_STYLESETBOLD, STYLE_BRACELIGHT, (LPARAM)TRUE);

	// this is the style for disabled code by preprocessor instructions
	// the style is the default one +64
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_WORD + 64, (LPARAM)RGB(185, 185, 255));
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_STRING + 64, (LPARAM)RGB(200, 180, 180));
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_NUMBER + 64, (LPARAM)RGB(255, 185, 185));
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_OPERATOR + 64, (LPARAM)RGB(185, 185, 185));
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_IDENTIFIER + 64, (LPARAM)RGB(185, 185, 185));
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_COMMENT + 64, (LPARAM)RGB(150, 200, 150));
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_COMMENTLINE + 64, (LPARAM)RGB(150, 200, 150));
	SendMessage(hwnd, SCI_STYLESETITALIC, SCE_C_COMMENT + 64, (LPARAM)TRUE);
	SendMessage(hwnd, SCI_STYLESETITALIC, SCE_C_COMMENTLINE + 64, (LPARAM)TRUE);


	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_USERLITERAL, (LPARAM)RGB(255, 102, 0));
	SendMessage(hwnd, SCI_STYLESETBOLD, SCE_C_USERLITERAL, (LPARAM)TRUE);

	//SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_STRING, (LPARAM)RGB(255, 102, 0));
	//SendMessage(hwnd, SCI_STYLESETBOLD, SCE_C_STRING, (LPARAM)TRUE);



	SendMessage(hwnd, SCI_COLOURISE, 0, (LPARAM)-1);
}
