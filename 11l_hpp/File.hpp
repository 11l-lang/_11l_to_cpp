#include <cstdio>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#undef far
#undef near
#else
#include <codecvt>
#include <locale>

std::string convert_utf16_to_utf8(const std::u16string &u16)
{
	return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.to_bytes(u16);
}

std::u16string convert_utf8_to_utf16(const char *s, size_t len)
{
	return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t>{}.from_bytes(s, s + len);
}
#endif

class UnicodeDecodeError {};

String convert_utf8_string_to_String(const char *s, size_t len)
{
#ifdef _WIN32
	String r;
	if (len != 0) {
		r.resize(len);
		SetLastError(ERROR_SUCCESS);
		r.resize(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, (int)len, (LPWSTR)r.data(), (int)r.size()));
		if (GetLastError() != ERROR_SUCCESS) {
			assert(GetLastError() == ERROR_NO_UNICODE_TRANSLATION);
			throw UnicodeDecodeError();
		}
	}
	return r;
#else
	return String(convert_utf8_to_utf16(s, len));
#endif
}

String convert_utf8_string_to_String(const std::string &s)
{
	return convert_utf8_string_to_String(s.data(), s.size());
}

class UnicodeEncodeError {};

Array<Byte> String::encode(const String &encoding) const
{
	assert(encoding == u"utf-8");

	if (empty())
		return Array<Byte>();

#ifdef _WIN32
	SetLastError(ERROR_SUCCESS);
	int r = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, (LPCWCH)data(), (int)size(), NULL, 0, NULL, NULL);
	if (GetLastError() != ERROR_SUCCESS) {
		assert(GetLastError() == ERROR_NO_UNICODE_TRANSLATION);
		throw UnicodeEncodeError();
	}

	Array<Byte> bytes;
	bytes.resize(r);
	WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, (LPCWCH)data(), (int)size(), (LPSTR)bytes.data(), r, NULL, NULL);
	return bytes;
#else
	std::string utf8 = convert_utf16_to_utf8(*this);
	Array<Byte> bytes;
	bytes.resize(utf8.size());
	memcpy(bytes.data(), utf8.data(), utf8.size());
	return bytes;
#endif
}

inline Array<Byte> Char::encode(const String &encoding = u"utf-8") const
{
	return String(*this).encode(encoding);
}

class FileNotFoundError {};

class File
{
	FILE *file;
	bool check_bom = true;

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
		file = NULL;
		_wfopen_s(&file, (wchar_t*)name.c_str(), (wchar_t*)(mode & u'b'_C).c_str());
#else
		file = fopen(convert_utf16_to_utf8(name).c_str(), convert_utf16_to_utf8(mode & u'b'_C).c_str());
#endif
		if (file == NULL)
			throw FileNotFoundError();
	}

	File &operator=(File &&f)
	{
		close();
		file = f.file;
		f.file = nullptr;
		return *this;
	}

	void write(const String &s) const
	{
		std::string utf8;
#ifdef _WIN32
		utf8.resize(s.length() * 3);
		utf8.resize(WideCharToMultiByte(CP_UTF8, 0, (LPCWCH)s.data(), (int)s.len(), const_cast<char*>(utf8.data()), (int)utf8.size(), NULL, NULL));
#else
		utf8 = convert_utf16_to_utf8(s);
#endif
		fwrite(utf8.data(), utf8.size(), 1, file);
	}

	void write_bytes(const Array<Byte> &bytes)
	{
		fwrite(bytes.data(), bytes.size(), 1, file);
	}

	String read() const
	{
		if (file == stdin) {
			String s;
			while (!feof(stdin)) {
				char buf[64*1024];
				size_t n = fread(buf, 1, sizeof(buf), stdin);
				assert(n > 0);
				size_t oldsz = s.size();
				s.resize(oldsz + n);
				for (size_t i = 0; i < n; i++)
					s[oldsz + i] = buf[i];
			}
			return s;
		}

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
		_ = fread(file_str.data(), file_size, 1, file);

		size_t cr_pos = file_str.find('\r');
		if (cr_pos != std::string::npos) {
			char *dest = file_str.data() + cr_pos;
			const char *src = dest + 1, *end = file_str.data() + file_str.size();
			while (src < end) {
				if (*src == '\r') {
					src++;
					continue;
				}
				*dest = *src;
				dest++;
				src++;
			}
			file_str.resize(dest - file_str.data());
		}

		return convert_utf8_string_to_String(file_str);
	}

	Array<String> read_lines(bool keep_newline = false)
	{
		Array<String> r = read().split(u"\n");
		if (keep_newline) {
			if (r[r.len()-1].empty()) {
				r.resize(r.len()-1);
				for (Int i=0, n=r.len(); i < n; i++)
					r[i] &= u'\n'_C;
			}
			else
				for (Int i=0, n=r.len()-1; i < n; i++)
					r[i] &= u'\n'_C;
		}
		else
			if (r[r.len()-1].empty())
				r.resize(r.len()-1);
		return r;
	}

	String read_line(bool keep_newline = false)
	{
		std::string line;

		while (true) {
			char buf[64*1024];
			buf[0] = '\0';
			char *s = fgets(buf, sizeof(buf), file);
			if (buf[0] == '\0')
				break;
			assert(buf == s);
			int len = (int)strlen(s);
			int orig_len = len;
			if (buf[len-1] == '\r')
				len--;
			else if (buf[len-1] == '\n' && len >= 2 && buf[len-2] == '\r') {
				buf[len-2] = '\n';
				len--;
			}
			if (!keep_newline && buf[len-1] == '\n')
				len--;

			if (check_bom) {
				check_bom = false;
				unsigned char utf8bom[3] = {0xEF, 0xBB, 0xBF};
				if (memcmp(buf, utf8bom, 3) == 0)
					s += 3, len -= 3;
			}

			line.append(s, len);

			if (orig_len < sizeof(buf)-1)
				break;
		}

		return convert_utf8_string_to_String(line);
	}

	Array<Byte> read_bytes()
	{
		fseek(file, 0, SEEK_END);
		size_t file_size = ftell(file);
		fseek(file, 0, SEEK_SET);

		Array<Byte> file_bytes;
		file_bytes.resize(file_size);
		size_t _ = fread(file_bytes.data(), file_size, 1, file);
		return file_bytes;
	}

	Array<Byte> read_bytes(size_t n)
	{
		Array<Byte> file_bytes;
		file_bytes.resize(n);
		size_t read = fread(file_bytes.data(), 1, n, file);
		assert(read == n);
		return file_bytes;
	}

	Char read(int n)
	{
		assert(n == 1);
		char c;
		size_t _ = fread(&c, 1, 1, file);
		return c;
	}

	void seek(Int offset, Int whence = 0)
	{
		fseek(file, (long)offset, (int)whence);
	}

	void flush()
	{
		fflush(file);
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
