#pragma once
#include <string>
#include <tchar.h>
#include <Windows.h>
#include <sstream>
#include <codecvt>

static std::string wstring2string(const std::wstring &str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.to_bytes(str);
}

static std::wstring string2wstring(const std::string &str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
	return converter.from_bytes(str);
}

#ifdef UNICODE
typedef std::wstring tstring;
typedef std::wstringstream tstringstream;
#define FROM_UTF16(str) (str)
#define FROM_UTF8(str) string2wstring(str)
#define TO_UTF8(str) wstring2string(str)
#else
typedef std::string tstring;
typedef std::stringstream tstringstream;
#define FROM_UTF16(str) wstring2string(str)
#define FROM_UTF8(str) (str)
#define TO_UTF8(str) (str)
#endif