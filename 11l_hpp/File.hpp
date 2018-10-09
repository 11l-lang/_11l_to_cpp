#include <cstdio>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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
		_wfopen_s(&file, (wchar_t*)name.c_str(), (wchar_t*)(mode + u'b').c_str());
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
		utf8.resize(s.length() * 3);
		utf8.resize(WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)s.data(), s.length(), const_cast<char*>(utf8.data()), utf8.size(), NULL, NULL));
		fwrite(utf8.data(), utf8.size(), 1, file);
	}

	String read()
	{
		fseek(file, 0, SEEK_END);
		size_t file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		std::string file_str;
		unsigned char utf8bom[3] = {0xEF, 0xBB, 0xBF}, first3bytes[3] = {0};
		fread(first3bytes, 3, 1, file);
		if (memcmp(first3bytes, utf8bom, 3) == 0)
			file_size -= 3;
		else
			fseek(file, 0, SEEK_SET);
		file_str.resize(file_size);
		fread(const_cast<char*>(file_str.data()), file_size, 1, file);

		String r;
		r.resize(file_size);
		r.resize(MultiByteToWideChar(CP_UTF8, 0, file_str.data(), file_str.size(), (LPWSTR)r.data(), r.size()));

		return r;
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
