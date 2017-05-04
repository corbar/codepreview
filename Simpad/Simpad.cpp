// Simpad.cpp : Defines the entry point for the application.
//

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include "../scintilla/include/Scintilla.h"
#include "../scintilla/include/SciLexer.h"
#include "Simpad.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE g_hInst = nullptr;                    // current instance
HWND g_hwndScintilla = nullptr;
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

#pragma region Some crap here



char* todos[] = {
	{ "TODO: when there is folded code, add some grey text to signal the folding, on top of the horizontal line"},
	{ "TODO: set a color to macros" },
	{ "TODO: for included headers, color differently the standard ones " },
	{ "TODO: when folding, shouldn't include the spaces after the folded region" },
	{ "TODO: make the text readonly" },
	{ "TODO: highlight the current line" },
	{ "TODO: add a shortcut to beautify the text" },
	{ "TODO: " },
};


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

	iLineMarginWidthNow = (int)SendMessage(hWnd, SCI_GETMARGINWIDTHN, 0, 0);
	iLineMarginWidthFit = (int)SendMessage(hWnd, SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)tchLines);

	if (iLineMarginWidthNow != iLineMarginWidthFit) {
		//SendMessage(hwndEdit,SCI_SETMARGINWIDTHN,0,0);
		SendMessage(hWnd, SCI_SETMARGINWIDTHN, 0, iLineMarginWidthFit);
	}
}

#pragma endregion


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	HMODULE hmod = LoadLibrary(TEXT("SciLexer.DLL"));
	if (hmod == NULL)
	{
		MessageBox(NULL,
			TEXT("The Scintilla DLL could not be loaded."),
			TEXT("Error loading Scintilla"),
			MB_OK | MB_ICONERROR);
	}
//	else
//	{
//		MessageBox(NULL,
//			TEXT("The Scintilla DLL is be loaded."),
//			TEXT("Scintilla is loaded"),
//			MB_OK | MB_ICONASTERISK);
//	}

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SIMPAD, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIMPAD));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIMPAD));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	//wcex.hbrBackground = (HBRUSH)(CreateSolidBrush(RGB(70, 140, 255)));
    //wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SIMPAD);
	wcex.lpszMenuName   = nullptr,
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}
void SetCppLexer(HWND);
//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   g_hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   HWND hwndScintilla = CreateWindowEx(0,
	   TEXT("Scintilla"), TEXT(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_CLIPCHILDREN,
	   0, 0, 0, 0, hWnd, (HMENU)IDC_SCINTILLA, hInstance, NULL);
   g_hwndScintilla = hwndScintilla;

   char mch[256] = "Consolas";
   int a = 0;

   DWORD color = GetSysColor(COLOR_WINDOWTEXT);
   DWORD col = RGB(0xFF, 0, 0);


//   SendMessage(hwndScintilla, SCI_SETMARGINWIDTHN, 2, 32);
//   SendMessage(hwndScintilla, SCI_SETFOLDMARGINCOLOUR, true, RGB(0, 255, 0));

   SendMessage(hwndScintilla, SCI_SETLEXER, SCLEX_CPP, 0);
   SendMessage(hwndScintilla, SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
   SendMessage(hwndScintilla, SCI_SETPROPERTY, (WPARAM)"fold.compact", (LPARAM)"1");
   SendMessage(hwndScintilla, SCI_SETPROPERTY, (WPARAM)"fold.preprocessor", (LPARAM)"1");
   SendMessage(hwndScintilla, SCI_SETPROPERTY, (WPARAM)"fold.comment", (LPARAM)"1");
   SendMessage(hwndScintilla, SCI_SETPROPERTY, (WPARAM)"fold.cpp.comment.multiline", (LPARAM)"1");

   SendMessage(hwndScintilla, SCI_SETAUTOMATICFOLD, SC_AUTOMATICFOLD_SHOW | SC_AUTOMATICFOLD_CLICK | SC_AUTOMATICFOLD_CHANGE, 0);
   SendMessage(hwndScintilla, SCI_FOLDDISPLAYTEXTSETSTYLE, SC_FOLDDISPLAYTEXT_BOXED, 0);
   SendMessage(hwndScintilla, SCI_SETFOLDFLAGS, SC_FOLDFLAG_LINEAFTER_CONTRACTED, 0);
   SendMessage(hwndScintilla, SCI_TOGGLEFOLDSHOWTEXT, 0, (LPARAM)"SOME TEXT");

   SendMessage(hwndScintilla, SCI_STYLESETFONT, STYLE_DEFAULT, (LPARAM)"Consolas");
   SendMessage(hwndScintilla, SCI_STYLESETSIZE, STYLE_DEFAULT, (LPARAM)20);
   SendMessage(hwndScintilla, SCI_STYLECLEARALL, 0, 0);// needs to be called after setting the default style
   SendMessage(hwndScintilla, SCI_STYLESETSIZE, STYLE_LINENUMBER, (LPARAM)12);
   SendMessage(hwndScintilla, SCI_STYLESETFORE, STYLE_LINENUMBER, (LPARAM)col);
   SendMessage(hwndScintilla, SCI_SETWRAPMODE, SC_WRAP_WORD, 0);
   a = (int)SendMessage(hwndScintilla, SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)"_9_");
   SendMessage(hwndScintilla, SCI_SETMARGINWIDTHN, 0, a);



   //put some text in the control
   LoadControlContent(hwndScintilla, L"c:\\Users\\cornel.barna\\Seneca\\OOP\\OOPDEV\\FinalProject\\DEV\\OOP-Milestone6\\PosApp.cpp");
   UpdateLineNumberWidth(hwndScintilla);

   // set the cpp syntax highlight
   SetCppLexer(hwndScintilla);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
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
	SendMessage(hwnd, SCI_SETKEYWORDS, 0, (LPARAM)"int char long void double float bool return true false namespace const using if else while new delete nothrow");
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

	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_COMMENT, (LPARAM)RGB(0, 128, 0));
	SendMessage(hwnd, SCI_STYLESETITALIC, SCE_C_COMMENT, (LPARAM)TRUE);
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_COMMENTLINE, (LPARAM)RGB(0, 128, 0));
	SendMessage(hwnd, SCI_STYLESETITALIC, SCE_C_COMMENTLINE, (LPARAM)TRUE);

	// bad colors
	//SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_OPERATOR, (LPARAM)RGB(255, 102, 0));
	//SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_NUMBER, (LPARAM)RGB(51, 153, 102));
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_NUMBER, (LPARAM)RGB(255, 0, 0));
	SendMessage(hwnd, SCI_STYLESETBOLD, SCE_C_NUMBER, (LPARAM)TRUE);

	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_OPERATOR, (LPARAM)RGB(153, 0, 0));

	// properly created string
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_STRING,    (LPARAM)RGB(163, 21, 21));
	// a single character
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_CHARACTER, (LPARAM)RGB(163, 21, 21));
	// this is the bad string (unclosed quotations)
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_STRINGEOL, (LPARAM)RGB(163, 21, 21));
	// string that have @ in front of them (INCOMPLETE)
	SendMessage(hwnd, SCI_STYLESETFORE, SCE_C_VERBATIM,  (LPARAM)RGB(163, 21, 21));

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

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(g_hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
	case WM_NOTIFY:
	{
		LPNMHDR pnmh = (LPNMHDR)lParam;
		if (pnmh->idFrom == IDC_SCINTILLA && pnmh->code == SCN_ZOOM)
			UpdateLineNumberWidth(pnmh->hwndFrom);
		else if (pnmh->idFrom == IDC_SCINTILLA && pnmh->code == SCN_UPDATEUI)
		{
			struct SCNotification* scn = (struct SCNotification*)lParam;
			if (scn->updated & ~(SC_UPDATE_V_SCROLL | SC_UPDATE_H_SCROLL))
			{
				// Invalidate invalid selections
				// #pragma message("TODO: Remove check for invalid selections once fixed in Scintilla")
				if (SendMessage(g_hwndScintilla, SCI_GETSELECTIONS, 0, 0) > 1 &&
					SendMessage(g_hwndScintilla, SCI_GETSELECTIONMODE, 0, 0) != SC_SEL_RECTANGLE)
				{
					int iCurPos = (int)SendMessage(g_hwndScintilla, SCI_GETCURRENTPOS, 0, 0);
					SendMessage(g_hwndScintilla, WM_CANCELMODE, 0, 0);
					SendMessage(g_hwndScintilla, SCI_CLEARSELECTIONS, 0, 0);
					SendMessage(g_hwndScintilla, SCI_SETSELECTION, (WPARAM)iCurPos, (LPARAM)iCurPos);
				}

				// Brace Match
				if (true)
				{
					int iPos;
					char c;

					int iEndStyled = (int)SendMessage(g_hwndScintilla, SCI_GETENDSTYLED, 0, 0);
					if (iEndStyled < (int)SendMessage(g_hwndScintilla, SCI_GETLENGTH, 0, 0)) {
						int iLine = (int)SendMessage(g_hwndScintilla, SCI_LINEFROMPOSITION, iEndStyled, 0);
						int iEndStyled = (int)SendMessage(g_hwndScintilla, SCI_POSITIONFROMLINE, iLine, 0);
						SendMessage(g_hwndScintilla, SCI_COLOURISE, iEndStyled, -1);
					}

					iPos = (int)SendMessage(g_hwndScintilla, SCI_GETCURRENTPOS, 0, 0);
					c = (char)SendMessage(g_hwndScintilla, SCI_GETCHARAT, iPos, 0);
					if ('(' == c || ')' == c || '[' == c || ']' == c || '{' == c || '}' == c) {
						int iBrace2 = (int)SendMessage(g_hwndScintilla, SCI_BRACEMATCH, iPos, 0);
						if (iBrace2 != -1) {
							int col1 = (int)SendMessage(g_hwndScintilla, SCI_GETCOLUMN, iPos, 0);
							int col2 = (int)SendMessage(g_hwndScintilla, SCI_GETCOLUMN, iBrace2, 0);
							SendMessage(g_hwndScintilla, SCI_BRACEHIGHLIGHT, iPos, iBrace2);
							SendMessage(g_hwndScintilla, SCI_SETHIGHLIGHTGUIDE, min(col1, col2), 0);
						}
						else {
							SendMessage(g_hwndScintilla, SCI_BRACEBADLIGHT, iPos, 0);
							SendMessage(g_hwndScintilla, SCI_SETHIGHLIGHTGUIDE, 0, 0);
						}
					}
					// Try one before
					else
					{
						iPos = (int)SendMessage(g_hwndScintilla, SCI_POSITIONBEFORE, iPos, 0);
						c = (char)SendMessage(g_hwndScintilla, SCI_GETCHARAT, iPos, 0);
						if ('(' == c || ')' == c || '[' == c || ']' == c || '{' == c || '}' == c) {
							int iBrace2 = (int)SendMessage(g_hwndScintilla, SCI_BRACEMATCH, iPos, 0);
							if (iBrace2 != -1) {
								int col1 = (int)SendMessage(g_hwndScintilla, SCI_GETCOLUMN, iPos, 0);
								int col2 = (int)SendMessage(g_hwndScintilla, SCI_GETCOLUMN, iBrace2, 0);
								SendMessage(g_hwndScintilla, SCI_BRACEHIGHLIGHT, iPos, iBrace2);
								SendMessage(g_hwndScintilla, SCI_SETHIGHLIGHTGUIDE, min(col1, col2), 0);
							}
							else {
								SendMessage(g_hwndScintilla, SCI_BRACEBADLIGHT, iPos, 0);
								SendMessage(g_hwndScintilla, SCI_SETHIGHLIGHTGUIDE, 0, 0);
							}
						}
						else {
							SendMessage(g_hwndScintilla, SCI_BRACEHIGHLIGHT, (WPARAM)-1, (LPARAM)-1);
							SendMessage(g_hwndScintilla, SCI_SETHIGHLIGHTGUIDE, 0, 0);
						}
					}
				}
			}
		}
		break;
	}
	case WM_SIZE:
	{
		int cx = LOWORD(lParam);
		int cy = HIWORD(lParam);
		SetWindowPos(g_hwndScintilla, nullptr, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
