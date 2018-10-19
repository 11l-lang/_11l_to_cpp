namespace fs
{
inline String get_temp_dir()
{
	size_t return_value;
	wchar_t buf[MAX_PATH];
	_wgetenv_s(&return_value, buf, L"TMPDIR");
	if (buf[0]) return String((char16_t*)buf, return_value - 1);
	_wgetenv_s(&return_value, buf, L"TEMP");
	if (buf[0]) return String((char16_t*)buf, return_value - 1);
	_wgetenv_s(&return_value, buf, L"TMP");
	if (buf[0]) return String((char16_t*)buf, return_value - 1);
	return String(u".");
}

namespace path
{
static const char16_t sep =
#ifdef _WIN32
u'\\'
#else
u'/'
#endif
;
String join(const String &path1, const String &path2)
{
	String r(path1);
	if (!(r.ends_with(u"\\") || r.ends_with(u"/")))
		r += sep;
	return r + path2;
}
}
}
