#include <cstdio>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#undef far
#undef near
#undef IGNORE
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
#include "file_for_humans/IFile.hpp"
#include "file_for_humans/OFile.hpp"

class UnicodeDecodeError {};

void convert_utf8_string_to_String(String &r, const char *s, size_t len)
{
#ifdef _WIN32
	if (len != 0) {
		r.resize(len);
		SetLastError(ERROR_SUCCESS);
		r.resize(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, (int)len, (LPWSTR)r.data(), (int)r.size()));
		if (GetLastError() != ERROR_SUCCESS) {
			assert(GetLastError() == ERROR_NO_UNICODE_TRANSLATION);
			throw UnicodeDecodeError();
		}
	}
#else
	r = convert_utf8_to_utf16(s, len);
#endif
}

String convert_utf8_string_to_String(const char *s, size_t len)
{
	String r;
	convert_utf8_string_to_String(r, s, len);
	return r;
}

void convert_utf8_string_to_String(String &r, const std::string &s)
{
	convert_utf8_string_to_String(r, s.data(), s.size());
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

//class FileNotFoundError {};

class StdHandleTag {};

template <bool reading> class TFile;

template <> class TFile<true> : public IFile
{
	IFile *as_mutable() const
	{
		return const_cast<IFile*>(static_cast<const IFile*>(this));
	}

	mutable std::string line_buf;

public:
	TFile() {}
	TFile(StdHandleTag, decltype(detail::stdin_handle()) handle)
	{
		fh.assign_std_handle(handle);
	}
	TFile(TFile &&f) : IFile(std::forward<IFile>(f)) {} // for `V ff = File(‘...’); V f = move(ff)`
	TFile(const TFile &f)
	{
		fh.assign_std_handle(f.fh);
	}

	TFile(const String &name, const String &encoding = u"utf-8-sig"_S) : IFile(name)
	{
		assert(encoding == u"utf-8" || encoding == u"utf-8-sig"); // in 11l, utf-8 encoding works as utf-8-sig when reading a file by intent [so the default encoding for writing files is utf-8, but the default encoding for reading files is like utf-8-sig in Python]
	}

	TFile &operator=(TFile &&f)
	{
		IFile::operator=(std::forward<IFile>(f));
		return *this;
	}

	bool at_eof() const
	{
		return as_mutable()->at_eof();
	}

	String read() const
	{
		return convert_utf8_string_to_String(as_mutable()->read_text());
	}

	class Lines
	{
		IFile file;
		bool keep_newline;

		class Sentinel {};
		class Iterator
		{
			Lines *lines;
			std::string line_buf;
			String line;
			bool has_next;

		public:
			Iterator(Lines *lines) : lines(lines) {has_next = !lines->file.at_eof(); if (has_next) operator++();}
			bool operator!=(Sentinel) const {return has_next;}
			void operator++() {has_next = !lines->file.at_eof(); if (has_next) {lines->file.read_line(line_buf, lines->keep_newline); convert_utf8_string_to_String(line, line_buf);}}
			const String &operator*() const {return line;}
		};
#if 0
		class Iter
		{
			IFile file;
			bool keep_newline;
			std::string line_buf;
			String line;

		public:
			Iter(IFile &&file, bool keep_newline) : file(std::move(file)), keep_newline(keep_newline) {advance();}

			const String &current() {return line;}

			bool advance()
			{
				if (file.at_eof())
					return false;
				file.read_line(line_buf, keep_newline);
				convert_utf8_string_to_String(line, line_buf);
				return true;
			}
		};
#endif
	public:
		Lines(IFile &&file, bool keep_newline) : file(std::move(file)), keep_newline(keep_newline) {}

		Iterator begin()
		{
			return Iterator(this);
		}

		Sentinel end()
		{
			return Sentinel();
		}
#if 0
		std::optional<Iter> iter() &&
		{
			if (file.at_eof())
				return std::nullopt;
			return Iter(std::move(file), keep_newline);
		}
#endif
	};

	Lines read_lines(bool keep_newline = false) &&
	{
		return Lines(std::move(static_cast<IFile&&>(*this)), keep_newline);
	}

	String read_line(bool keep_newline = false) const
	{
		as_mutable()->read_line(line_buf, keep_newline);
		return convert_utf8_string_to_String(line_buf);
	}

	void read_line(String &r, bool keep_newline = false) const
	{
		as_mutable()->read_line(line_buf, keep_newline);
		convert_utf8_string_to_String(r, line_buf);
	}

	Array<Byte> read_bytes() const
	{
		return static_cast<Array<Byte>&&>(as_mutable()->read_bytes());
	}

	Array<Byte> read_bytes(size_t n) const
	{
		return static_cast<Array<Byte>&&>(as_mutable()->read_bytes(n));
	}

	Array<Byte> read_bytes_at_most(size_t n) const
	{
		return static_cast<Array<Byte>&&>(as_mutable()->read_bytes_at_most(n));
	}

	Char read(int n) const
	{
		assert(n == 1);
		char32_t c = as_mutable()->read_char();
		assert(c <= 0xFFFF);
		return (char16_t)c;
	}

	template <class Ty> TFile &operator>>(Ty &v)
	{
		assert(fh.is_std_handle());
		std::wcin >> v;
		return *this;
	}

	void seek(Int64 offset, Int whence = 0) const
	{
		switch (whence) {
		case SEEK_SET:
			break;
		case SEEK_CUR:
			offset += tell();
			break;
		case SEEK_END:
			offset += as_mutable()->get_file_size();
			break;
		default:
			assert(false);
		}
		as_mutable()->seek(offset);
	}

	bool is_associated_with_console() const
	{
		return fh.is_associated_with_console();
	}
};

template <> class TFile<false> : public OFile
{
	OFile *as_mutable() const
	{
		return const_cast<OFile*>(static_cast<const OFile*>(this));
	}

public:
	TFile() {}
	TFile(StdHandleTag, decltype(detail::stdout_handle()) handle)
	{
		fh.assign_std_handle(handle);
	}
	TFile(TFile &&f) : OFile(std::forward<OFile>(f)) {}
	TFile(const TFile &f)
	{
		fh.assign_std_handle(f.fh);
	}

	TFile(const String &name, const String &encoding = u"utf-8"_S, bool append = false) : OFile(name, append)
	{
		assert(encoding == u"utf-8" || encoding == u"utf-8-sig");

		if (encoding == u"utf-8-sig") {
			if (append) {
				if (fh.get_file_size() != 0)
					return; // it looks like BOM has already been written
			}
			unsigned char utf8bom[3] = {0xEF, 0xBB, 0xBF};
			OFile::write(utf8bom, 3);
		}
	}

	TFile &operator=(TFile &&f)
	{
		f.flush(); // `flush()` is needed for `V f = :stdout; f = File(‘...’, WRITE, encoding' ‘utf-8-sig’)`
		OFile::operator=(std::forward<OFile>(f));
		return *this;
	}

	void write(const String &s) const
	{
		as_mutable()->write(utf::as_str8(s));
	}

	void write_bytes(const Array<Byte> &bytes) const
	{
		as_mutable()->write(bytes);
	}

	void flush() const
	{
		as_mutable()->flush();
	}

	bool is_associated_with_console() const
	{
		return fh.is_associated_with_console();
	}
};

typedef TFile<true> File;
typedef TFile<false> FileWr;

File _stdin(StdHandleTag(), detail::stdin_handle());
FileWr _stdout(StdHandleTag(), detail::stdout_handle());
FileWr _stderr(StdHandleTag(), detail::stderr_handle());
