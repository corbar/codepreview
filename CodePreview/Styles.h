#pragma once

// Scintilla supports maximum 9 lists of keywords
// The lists are numbered from 0 to KEYWORDSET_MAX
struct SciKeywords
{
	char* pszKeywords[9];
};
struct SciStyle
{
	int iStyle;
	wchar_t* pszFont;
	int fontSize;
	int colFore;
	int colBack;
	bool eolFill;
	bool bold;
	bool italic;
	bool undeline;
};
struct SciLexer
{
	int iLexer;
	wchar_t* pszName;
	wchar_t* pszDefExt;
	SciKeywords* pKeywords;
	SciStyle     styles[];
};
