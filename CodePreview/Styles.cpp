#include "../scintilla/include/Scintilla.h"
#include "../scintilla/include/SciLexer.h"
#include "Styles.h"

SciKeywords keywords_NULL = { "", "", "", "", "", "", "", "", "" };

SciLexer lexDefault =
{
	SCLEX_NULL, L"Default Text", L"txt; log; nfo", &keywords_NULL,
	{
		// Style Id             Font         Size   Fore      Back      EOL    Bold   Italic Underline
		{ STYLE_DEFAULT,        L"Consolas", 12,    0x000000, 0xFFFFFF, true,  false, false, false },
		{ STYLE_LINENUMBER,     L"",         10,    0xFF0000, 0xFFFFFF, false, false, false, false },
		{ STYLE_BRACELIGHT,     L"",         12,    0xFF0000, 0xFFDCDC, false, true,  false, false },
		{ STYLE_BRACEBAD,       L"",         13,    0x000080, 0xFFFFFF, false, false, false, false },
		{ STYLE_CONTROLCHAR,    L"",         11,    0x000000, 0xFFFFFF, false, false, false, false },
		{ STYLE_INDENTGUIDE,    L"",         12,    0xA0A0A0, 0xFFFFFF, false, false, false, false },
		{ SCI_SETCARETLINEBACK, L"",         12,    0x000000, 0xFFFF00, false, false, false, false },
		{ SCI_SETEDGECOLOUR,    L"",         12,    0xFFC000, 0xFFFFFF, false, false, false, false },
		{ -1 }
	}
};

SciKeywords keywords_CPP =
{
	// Primary keywords and identifiers. STATE: SCE_C_WORD
	"__abstract __alignof __asm __assume __based __box __cdecl __declspec __delegate __event "
	"__except __except__try __fastcall __finally __forceinline __gc __hook __identifier "
	"__if_exists __if_not_exists __inline __int16 __int32 __int64 __int8 __interface __leave "
	"__m128 __m128d __m128i __m64 __multiple_inheritance __nogc __noop __pin __property __raise "
	"__sealed __single_inheritance __stdcall __super __try __try_cast __unhook __uuidof __value "
	"__virtual_inheritance __wchar_t auto bool break case catch char class const const_cast "
	"continue default defined delete do double dynamic_cast else enum explicit extern false float "
	"for friend goto if inline int long mutable naked namespace new nothrow operator private "
	"protected public register reinterpret_cast return short signed size_t sizeof static static_cast "
	"struct switch template this throw true try typedef typeid typename union unsigned using uuid "
	"virtual void volatile wchar_t while",
	// Secondary keywords and identifiers. STATE: SCE_C_WORD2
	"strlen strcpy strncpy strcat",
	// Documentation comment keywords. STATE: SCE_C_COMMENTDOCKEYWORD
	"",
	// Global classes and typedefs. STATE: SCE_C_GLOBALCLASS
	"istream ostream ofstream ifstream fstream ios string std vector",
	// Preprocessor definitions -- not really sure how the lexer is using them
	"",
	// Task marker and error marker keywords. STATE: SCE_C_TASKMARKER
	"TODO FIXME",
	// Not used by the cpp lexer
	"", "", ""
};

SciLexer lexCpp = 
{
	SCLEX_CPP, L"C/C++ source code", L"h; c; cpp; cxx; cc; hpp; hxx", &keywords_CPP,
	{
		// Style Id                     Font         Size   Fore      Back      EOL    Bold   Italic Underline
		{ STYLE_DEFAULT,                L"Consolas", 12,    0x000000, 0xFFFFFF, true,  false, false, false },
		// comments that start with "/*"
		{ SCE_C_COMMENT,                L"",         12,    0x008000, 0xFFFFFF, false, false, true,  false },
		// comments that start with "//"
		{ SCE_C_COMMENTLINE,            L"",         12,    0x008000, 0xFFFFFF, false, false, true,  false },
		// comments that start with "/**" or "/*!"
		{ SCE_C_COMMENTDOC,             L"",         12,    0x008000, 0xF6F7FA, true,  false, true,  false },
		// the numbers that appear in the code
		{ SCE_C_NUMBER,                 L"",         12,    0xFF0000, 0xFFFFFF, false, true,  false, false },
		// the first list of keywords: keywords_CPP[0]
		{ SCE_C_WORD,                   L"",         12,    0x0000FF, 0xFFFFFF, false, true,  false, false },
		// properly created string (enclosed between double quotation marks)
		{ SCE_C_STRING,                 L"",         12,    0xA31515, 0xFFFFFF, false, false, false, false },
		// a single character (enclosed between single qutation marks)
		{ SCE_C_CHARACTER,              L"",         12,    0xA31515, 0xFFFFFF, false, false, false, false },
		{ SCE_C_UUID,                   L"",         12,    0x000000, 0xFFFFFF, false, false, false, false },
		// preprocessor directives (things that start with #)
		{ SCE_C_PREPROCESSOR,           L"",         12,    0x808089, 0xFFFFFF, false, false, false, false },
		// the operators that appear in the code. TODO: show here the list
		{ SCE_C_OPERATOR,               L"",         12,    0x990000, 0xFFFFFF, false, false, false, false },
		{ SCE_C_IDENTIFIER,             L"",         12,    0x000000, 0xFFFFFF, false, false, false, false },
		// badly formatted string (unclosed double quotations)
		{ SCE_C_STRINGEOL,              L"",         12,    0xA31515, 0xFFFFFF, false, false, false, false },
		// string that have @ in front of them (INCOMPLETE)
		{ SCE_C_VERBATIM,               L"",         12,    0xA31515, 0xFFFFFF, false, false, false, false },
		{ SCE_C_REGEX,                  L"",         12,    0x000000, 0xFFFFFF, false, false, false, false },
		{ SCE_C_COMMENTLINEDOC,         L"",         12,    0x000000, 0xFFFFFF, false, false, false, false },
		// the second list of keywords: keywords_CPP[1]
		{ SCE_C_WORD2,                  L"",         12,    0x339966, 0xFFFFFF, false, false, false, false },
		{ SCE_C_COMMENTDOCKEYWORD,      L"",         12,    0x000000, 0xFFFFFF, false, false, false, false },
		{ SCE_C_COMMENTDOCKEYWORDERROR, L"",         12,    0x000000, 0xFFFFFF, false, false, false, false },
		// the fourth list of keywords: keywords_CPP[3]
		{ SCE_C_GLOBALCLASS,            L"",         12,    0x2D91AF, 0xFFFFFF, false, false, false, false },
		{ SCE_C_STRINGRAW,              L"",         12,    0x000000, 0xFFFFFF, false, false, false, false },
		{ SCE_C_TRIPLEVERBATIM,         L"",         12,    0x000000, 0xFFFFFF, false, false, false, false },
		{ SCE_C_HASHQUOTEDSTRING,       L"",         12,    0x000000, 0xFFFFFF, false, false, false, false },
		// the comment that appears in a preprocessor instruction
		{ SCE_C_PREPROCESSORCOMMENT,    L"",         12,    0x339966, 0xFFFFFF, false, false, true,  false },
		// TODO: figure out where this style goes
		{ SCE_C_PREPROCESSORCOMMENTDOC, L"",         12,    0x008000, 0xFFFFFF, false, false, true,  false },
		// TODO: figure out where this style goes
		{ SCE_C_USERLITERAL,            L"",         12,    0xFF6600, 0xFFFFFF, false, true, false, false },
		// the sixth list of keywords: keywords_CPP[5]
		{ SCE_C_TASKMARKER,             L"",         12,    0xFF0000, 0xFFDCDC, false, true,  false, false },
		{ SCE_C_ESCAPESEQUENCE,         L"",         12,    0x000000, 0xFFFFFF, false, false, false, false },
		{ -1 }
	}
};

/*
	// style for disabled code by preprocessor instructions
	// (the style is the default one +64)
	SendMessage(m_hwndScintilla, SCI_STYLESETFORE,   SCE_C_WORD        + 64, (LPARAM)RGB(185, 185, 255));
	SendMessage(m_hwndScintilla, SCI_STYLESETFORE,   SCE_C_STRING      + 64, (LPARAM)RGB(200, 180, 180));
	SendMessage(m_hwndScintilla, SCI_STYLESETFORE,   SCE_C_NUMBER      + 64, (LPARAM)RGB(255, 185, 185));
	SendMessage(m_hwndScintilla, SCI_STYLESETFORE,   SCE_C_OPERATOR    + 64, (LPARAM)RGB(185, 185, 185));
	SendMessage(m_hwndScintilla, SCI_STYLESETFORE,   SCE_C_IDENTIFIER  + 64, (LPARAM)RGB(185, 185, 185));
	SendMessage(m_hwndScintilla, SCI_STYLESETFORE,   SCE_C_COMMENT     + 64, (LPARAM)RGB(150, 200, 150));
	SendMessage(m_hwndScintilla, SCI_STYLESETFORE,   SCE_C_COMMENTLINE + 64, (LPARAM)RGB(150, 200, 150));
	SendMessage(m_hwndScintilla, SCI_STYLESETITALIC, SCE_C_COMMENT     + 64, (LPARAM)TRUE);
	SendMessage(m_hwndScintilla, SCI_STYLESETITALIC, SCE_C_COMMENTLINE + 64, (LPARAM)TRUE);
*/
