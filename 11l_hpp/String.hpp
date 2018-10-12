#include <string>
#include <cwctype>

static class LocaleInitializer
{
public:
	LocaleInitializer() {setlocale(LC_CTYPE, "");} // for correct work of `Char::towlower()` and `Char::iswlower()`
} locale_initializer;

class Char
{
public:
	char16_t code;

	Char(char16_t code) : code(code) {}

	operator char16_t() const {return code;} // for `switch (instr[i])` support

	bool isdigit() const {return in((int)code, range_ee((int)'0', (int)'9'));}

	bool operator< (Char c) const {return code <  c.code;}
	bool operator<=(Char c) const {return code <= c.code;}
	bool operator> (Char c) const {return code >  c.code;}
	bool operator>=(Char c) const {return code >= c.code;}

	bool operator==(Char c) const {return code == c.code;}
	bool operator!=(Char c) const {return code != c.code;}
	bool operator==(char16_t c) const {return code == c;}
	bool operator!=(char16_t c) const {return code != c;}

	Char   lower() const {return towlower(code);}
	bool islower() const {return iswlower(code);}
};

class String : public std::u16string
{
	String slice(int begin, int end) const
	{
		return String(c_str() + begin, end - begin);
	}

public:
	String() {}
	String(Char c) : basic_string(1, c.code) {}
	explicit String(char16_t c) : basic_string(1, c) {}
	explicit String(int num)
	{
		char16_t staticBuffer[30];
		int len = 0;
		if (num < 0)
		{
			staticBuffer[len++] = u'-';
			do {
				staticBuffer[len++] = u'0'-(num%10);
			} while (num /= 10);
		}
		else
		do
		{
			staticBuffer[len++] = u'0'+(num%10);
			num /= 10;
		} while (num);
		for (char16_t *start=staticBuffer[0]==u'-' ? staticBuffer+1 : staticBuffer,
			*end=staticBuffer+len-1; start<end; start++, end--) std::swap(*start, *end);
		assign(staticBuffer, len);
	}
	explicit String(const char16_t *&s) : basic_string(s) {} // reference is needed here because otherwise String(const char16_t (&s)[N]) is never called (`String(u"str")` calls `String(const char16_t *s)`)
	String(const char16_t *s, size_t sz) : basic_string(s, sz) {}
	template <int N> String(const char16_t (&s)[N]): basic_string(s, N-1) {}

	class Iterator
	{
		char16_t *c;
	public:
		explicit Iterator(char16_t *c) : c(c) {}
		bool operator!=(Iterator i) {return c != i.c;}
		void operator++() {c++;}
		Char operator*() {return Char(*c);}
	};
	Iterator begin() {return Iterator(const_cast<char16_t*>(data()));}
	Iterator end()   {return Iterator(const_cast<char16_t*>(data()) + len());}
	Iterator begin() const {return Iterator(const_cast<char16_t*>(data()));}
	Iterator end()   const {return Iterator(const_cast<char16_t*>(data() + len()));}

	int len() const { return size(); } // return `int` (not `size_t`) to avoid warning C4018: '<': signed/unsigned mismatch

	bool starts_with(const char16_t *s, size_t sz) const
	{
		return len() >= (int)sz && memcmp(data(), s, sz*sizeof(char16_t)) == 0;
	}
	bool starts_with(const char16_t *&s_) const
	{
		for (const char16_t *t = data(), *s = s_; ;s++, t++)
		{
			if (*s == 0) return true;
			if (*t == 0) return false;
			if (*s != *t) return false;
		}
	}
	template <int N> bool starts_with(const char16_t (&s)[N]) const {return starts_with(s, N-1);}
	bool starts_with(const String &s) const {return starts_with(s.data(), s.len());}

	String replace(const String &old, const String &n) const
	{
		String str(*this);
		size_t start_pos = 0;
		while((start_pos = str.find(old, start_pos)) != String::npos) {
			str.std::u16string::replace(start_pos, old.length(), n);
			start_pos += n.length();
		}
		return str;
	}

	int findi(Char c, int start = 0) const
	{
		const char16_t *s = data();
		for (int i=start, n=len(); i<n; i++)
			if (s[i] == c) return i;
		return -1;
	}
	
	int findi(const String &s, int start = 0) const
	{
		size_t r = find(s, start);
		return r != String::npos ? r : -1;
	}

	int rfindi(const String &sub, int start, int end) const
	{
		size_t r = rfind(sub, end - 1);
		if (r == String::npos || (int)r < start) return -1;
		return r;
	}

	int count(const char16_t *s, size_t sz) const
	{
		int c = 0;
		for (int i=0; i<len();)
			if (memcmp(s, data() + i, sz * sizeof(char16_t)) == 0) {
				c++;
				i += sz;
			} else
				i++;
		return c;
	}
	template <int N> int count(const char16_t(&s)[N]) const
	{
		return count(s, N-1);
	}
	int count(const String &s) const
	{
		return count(s.data(), s.len());
	}

	Array<String> split(const String &delim);
	
	bool isdigit() const
	{
		if (empty()) return false;
		const char16_t *s = data();
		for (int i=0, n=len(); i<n; i++)
			if (!Char(s[i]).isdigit()) return false;
		return true;
	}

	String upper() const
	{
		String r;
		r.resize(len());
		for (int i=0; i<len(); i++)
			r[i] = towupper(at(i));
		return r;
	}

	String zfill(int width) const
	{
		return String(u"0") * max(width - len(), 0) + *this;
	}

	//String &operator=(const String &s) {assign(s); return *this;}

	friend String &&operator*(String &&s, int n)
	{
		int s_len = s.length();
		if (n < 1) // mimic Python's behavior in which 's' * 0 = '' and 's' * -1 = ''
			s.clear();
		else {
			s.reserve(s_len * n);
			for (int i = 1; i < n; i++)
				s.append(s.c_str(), s_len);
		}
		return std::move(s);
	}
	friend String &&operator*(int n, String &&s)
	{
		return std::move(s) * n;
	}
	String operator*(int n) const
	{
		return String(*this) * n;
	}

	Char &operator[](int pos)
	{
		return (Char&)at(pos);
	}
	const Char operator[](int pos) const
	{
		return Char(at(pos));
	}

	String operator[](const Range<int, true,  true > &range) const {return slice(max(range.b    , 0), min((unsigned)range.e + 1u, (unsigned)len()));} // `(unsigned)` is needed when `instr` starts with left single quotation mark
	String operator[](const Range<int, true,  false> &range) const {return slice(max(range.b    , 0), min((unsigned)range.e     , (unsigned)len()));}
	String operator[](const Range<int, false, true > &range) const {return slice(max(range.b + 1, 0), min((unsigned)range.e + 1u, (unsigned)len()));}
	String operator[](const Range<int, false, false> &range) const {return slice(max(range.b + 1, 0), min((unsigned)range.e     , (unsigned)len()));}
	String operator[](const RangeEI<int>             &range) const {return slice(max(range.b    , 0), len());}

	bool operator==(Char ch) const {return   len() == 1 && at(0) == ch.code ;}
	bool operator!=(Char ch) const {return !(len() == 1 && at(0) == ch.code);}
	friend bool operator==(Char ch, const String &s) {return   s.len() == 1 && s.at(0) == ch.code ;}
	friend bool operator!=(Char ch, const String &s) {return !(s.len() == 1 && s.at(0) == ch.code);}

	template <int N> bool operator==(const char16_t (&s)[N]) const {return   len() == N-1 && memcmp(c_str(), s, (N-1)*sizeof(char16_t)) == 0 ;}
	template <int N> bool operator!=(const char16_t (&s)[N]) const {return !(len() == N-1 && memcmp(c_str(), s, (N-1)*sizeof(char16_t)) == 0);}

	String operator+(const String &s) const {String r(*this); r.append(s); return r;}
	String operator+(Char ch) {String r(*this); r.append(1, ch.code); return r;}

	String operator+(int i) {return *this + String(i);}
};

String operator+(Char ch1, Char ch2)
{
	return String(ch1) + ch2;
}

String operator ""_S(const char16_t *s, size_t sz)
{
	return String(s, sz);
}

Char operator ""_C(char16_t c)
{
	return Char(c);
}

inline int parse_int(const String &str)
{
	int res = 0, sign = 1;
	const char16_t *s = str.c_str();
	while (*s && (*s == u' ' || *s == u'\t')) s++;//skip whitespace
	if (*s == u'-') sign=-1, s++; else if (*s == u'+') s++;
	for (; Char(*s).isdigit(); s++)
		res = res * 10 + (*s - u'0');
	return res * sign;
}

inline bool in(Char c, const String &s) {return s.find(c.code) != String::npos;}

inline char hex_to_char(int c) {return (char)c + ((unsigned)c <= 9u ? '0' : 'A' - 10);}

inline String hex(int n)
{
	char16_t rr[8], *h = rr;
	for (const unsigned char *d = (unsigned char*)&n + sizeof(n)-1; d >= (unsigned char*)&n; d--)
		*h++ = hex_to_char(*d >> 4),
		*h++ = hex_to_char(*d & 0xF);
	const char16_t *s = rr;
	while (*s == u'0')
		s++;
	String r = u"0x";
	if (h > s)
		r.append(s, h-s);
	else
		r.append(1, u'0');
	return r;
}
