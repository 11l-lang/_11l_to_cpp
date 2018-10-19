namespace fs
{
inline String get_temp_dir()
{
	size_t return_value;
	wchar_t buf[MAX_PATH];
	_wgetenv_s(&return_value, buf, L"TMPDIR");
	if (buf[0]) return String((char16_t*)buf, return_value);
	_wgetenv_s(&return_value, buf, L"TEMP");
	if (buf[0]) return String((char16_t*)buf, return_value);
	_wgetenv_s(&return_value, buf, L"TMP");
	if (buf[0]) return String((char16_t*)buf, return_value);
	return String(u".");
}
}
