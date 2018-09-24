#include <cstdio>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

class File
{
	FILE *file;

public:
	File(FILE *file) : file(file) {}

	void write(const String &s)
	{
		std::string utf8;
		utf8.resize(s.length() * 2);
		utf8.resize(WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)s.data(), s.length(), utf8.data(), utf8.size(), NULL, NULL));
		fwrite(utf8.data(), utf8.size(), 1, file);
	}

	String read()
	{
		return String();
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
