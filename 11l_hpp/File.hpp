#include <cstdio>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <codecvt>
#include <locale>

std::string convert_utf16_to_utf8(const std::u16string &u16)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16);
}

std::u16string convert_utf8_to_utf16(const std::string &u8)
{
	return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(u8);
}
#endif

class File
{
	FILE *file;

public:
	File(FILE *file) : file(file) {}

	File(const File &f)
	{
		assert(f.file == stdin || f.file == stdout || f.file == stderr);
		file = f.file;
	}

	File(const String &name, const String &mode = u"r"_S, const String &encoding = u"utf-8"_S, const String &newline = u""_S)
	{
#ifdef _WIN32
		_wfopen_s(&file, (wchar_t*)name.c_str(), (wchar_t*)(mode + u'b').c_str());
#else
		file = fopen(convert_utf16_to_utf8(name).c_str(), convert_utf16_to_utf8(mode + u'b').c_str());
#endif
	}

	File &operator=(File &&f)
	{
		close();
		file = f.file;
		f.file = nullptr;
		return *this;
	}

	void write(const String &s)
	{
		std::string utf8;
#ifdef _WIN32
		utf8.resize(s.length() * 3);
		utf8.resize(WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)s.data(), s.length(), const_cast<char*>(utf8.data()), utf8.size(), NULL, NULL));
#else
		utf8 = convert_utf16_to_utf8(s);
#endif
		fwrite(utf8.data(), utf8.size(), 1, file);
	}

	String read()
	{
		fseek(file, 0, SEEK_END);
		size_t file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		std::string file_str;
		unsigned char utf8bom[3] = {0xEF, 0xBB, 0xBF}, first3bytes[3] = {0};
		size_t _ = fread(first3bytes, 3, 1, file); // `size_t _` to suppress GCC warning ‘ignoring return value of ‘size_t fread(void*, size_t, size_t, FILE*)’, declared with attribute warn_unused_result [-Wunused-result]’
		if (memcmp(first3bytes, utf8bom, 3) == 0)
			file_size -= 3;
		else
			fseek(file, 0, SEEK_SET);
		file_str.resize(file_size);
		_ = fread(const_cast<char*>(file_str.data()), file_size, 1, file);

#ifdef _WIN32
		String r;
		r.resize(file_size);
		r.resize(MultiByteToWideChar(CP_UTF8, 0, file_str.data(), file_str.size(), (LPWSTR)r.data(), r.size()));
		return r;
#else
		return String(convert_utf8_to_utf16(file_str));
#endif
	}

	void close()
	{
		if (file && file != stdin && file != stdout && file != stderr)
			fclose(file);
		file = nullptr;
	}

	~File() {close();}
};

File _stdin(stdin), _stdout(stdout), _stderr(stderr);
