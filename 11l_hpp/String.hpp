#include <string>
#include <cstring> // for memcmp in GCC
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

	bool is_digit() const {return in((int)code, range_ee((int)'0', (int)'9'));}

	bool operator< (Char c) const {return code <  c.code;}
	bool operator<=(Char c) const {return code <= c.code;}
	bool operator> (Char c) const {return code >  c.code;}
	bool operator>=(Char c) const {return code >= c.code;}

	bool operator==(Char c) const {return code == c.code;}
	bool operator!=(Char c) const {return code != c.code;}
	bool operator==(char16_t c) const {return code == c;}
	bool operator!=(char16_t c) const {return code != c;}

	Char    lowercase() const {return towlower(code);}
	bool is_lowercase() const {return iswlower(code);}
	Char    uppercase() const {return towupper(code);}
	bool is_uppercase() const {return iswupper(code);}
};

namespace re {class RegEx;}

class String : public std::u16string
{
	String slice(int begin, int end) const
	{
		return String(c_str() + begin, end - begin);
	}

public:
	String() {}
	String(Char c) : basic_string(1, c.code) {}
	String(std::u16string &&s) : std::u16string(std::forward<std::u16string>(s)) {}
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
	explicit String(float  num) {assign(num);}
	explicit String(double num) {assign(num);}
	explicit String(const char16_t *&s) : basic_string(s) {} // reference is needed here because otherwise String(const char16_t (&s)[N]) is never called (`String(u"str")` calls `String(const char16_t *s)`)
	String(const char16_t *s, size_t sz) : basic_string(s, sz) {}
	template <int N> String(const char16_t (&s)[N]): basic_string(s, N-1) {}
	explicit String(const char *s, const char *e) : basic_string(s, e) {}

	using std::u16string::assign;
	void assign(double num)
	{
		std::wstring ws = std::to_wstring(num);
		size_t l = ws.length() - 1;
		while (ws[l] == '0') l--;
		if (ws[l] == '.') l--;
		assign(ws.begin(), ws.begin() + l + 1);
	}

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

	int len() const { return (int)size(); } // return `int` (not `size_t`) to avoid warning C4018: '<': signed/unsigned mismatch

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
	template <typename Type> bool starts_with(const Tuple<Type, Type> &tuple)
	{
		return starts_with(_get<0>(tuple)) || starts_with(_get<1>(tuple));
	}
	template <typename Type> bool starts_with(const Tuple<Type, Type, Type> &tuple)
	{
		return starts_with(_get<0>(tuple)) || starts_with(_get<1>(tuple)) || starts_with(_get<2>(tuple));
	}
	template <typename Type> bool starts_with(const Tuple<Type, Type, Type, Type> &tuple)
	{
		return starts_with(_get<0>(tuple)) || starts_with(_get<1>(tuple)) || starts_with(_get<2>(tuple)) || starts_with(_get<3>(tuple));
	}

	bool ends_with(const char16_t *s, size_t sz) const
	{
		return len() >= (int)sz && memcmp(data() + len() - sz, s, sz*sizeof(char16_t)) == 0;
	}
	template <int N> bool ends_with(const char16_t (&s)[N]) const {return ends_with(s, N-1);}
	bool ends_with(const String &s) const {return ends_with(s.data(), s.len());}
	template <typename Type> bool ends_with(const Tuple<Type, Type> &tuple)
	{
		return ends_with(_get<0>(tuple)) || ends_with(_get<1>(tuple));
	}
	template <typename Type> bool ends_with(const Tuple<Type, Type, Type> &tuple)
	{
		return ends_with(_get<0>(tuple)) || ends_with(_get<1>(tuple)) || ends_with(_get<2>(tuple));
	}
	template <typename Type> bool ends_with(const Tuple<Type, Type, Type, Type> &tuple)
	{
		return ends_with(_get<0>(tuple)) || ends_with(_get<1>(tuple)) || ends_with(_get<2>(tuple)) || ends_with(_get<3>(tuple));
	}

	String replace(const String &old, const String &n) const
	{
		String str(*this);
		size_t start_pos = 0;
		while((start_pos = str.findi(old, (int)start_pos)) != -1) {
			str.std::u16string::replace(start_pos, old.length(), n);
			start_pos += n.length();
		}
		return str;
	}

	Nullable<int> find(Char c, int start = 0) const
	{
		const char16_t *s = data();
		for (int i=start, n=len(); i<n; i++)
			if (s[i] == c) return Nullable<int>(i);
		return Nullable<int>();
	}
	
	Nullable<int> find(const String &s, int start = 0) const
	{
		size_t r = basic_string::find(s, start);
		return r == npos ? Nullable<int>() : Nullable<int>((int)r);
	}

	Nullable<int> find(const Tuple<String, String> &t, int start = 0) const
	{
		for (int i = start, l = len() - min(std::get<0>(t).len(), std::get<1>(t).len()); i <= l; i++) {
			if (i <= len() - std::get<0>(t).len() && memcmp(c_str() + i, std::get<0>(t).c_str(), std::get<0>(t).len() * sizeof(char16_t)) == 0) return i;
			if (i <= len() - std::get<1>(t).len() && memcmp(c_str() + i, std::get<1>(t).c_str(), std::get<1>(t).len() * sizeof(char16_t)) == 0) return i;
		}
		return nullptr;
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
		size_t r = basic_string::find(s, start);
		return r != String::npos ? (int)r : -1;
	}

	int rfindi(const String &sub, int start, int end) const
	{
		size_t r = rfind(sub, end - 1);
		if (r == String::npos || (int)r < start) return -1;
		return (int)r;
	}

	int count(const char16_t *s, size_t sz) const
	{
		int c = 0;
		for (int i=0; i<len();)
			if (memcmp(s, data() + i, sz * sizeof(char16_t)) == 0) {
				c++;
				i += (int)sz;
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

	Array<String> split(const String &delim) const;
	Array<String> split(const re::RegEx &regex) const;
	
	bool is_digit() const
	{
		if (empty()) return false;
		const char16_t *s = data();
		for (int i=0, n=len(); i<n; i++)
			if (!Char(s[i]).is_digit()) return false;
		return true;
	}

	String lowercase() const
	{
		String r;
		r.resize(len());
		for (int i=0; i<len(); i++)
			r[i] = towlower(at(i));
		return r;
	}

	String uppercase() const
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

	String ltrim(Char c, Nullable<int> limit = nullptr) const
	{
		const char16_t *s = data();
		int i = 0;
		for (int l=limit == nullptr ? len() : min(len(), *limit); i<l; i++)
			if (s[i] != c) break;
		return String(s + i, len() - i);
	}

	String ltrim(const String &str, Nullable<int> limit = nullptr) const
	{
		const char16_t *s = data();
		int i = 0, l = len()-str.len();
		for (int lim = limit == nullptr ? -1 : *limit; i<=l; i+=str.len())
			if (memcmp(str.c_str(), s+i, str.len()*sizeof(char16_t)) || lim-- == 0) break;
		return String(s + i, len() - i);
	}

	template <typename ... Types> String ltrim(const Tuple<Types...> &tuple, Nullable<int> limit = nullptr) const
	{
		const char16_t *s = data();
		int i = 0;
		for (int l=limit == nullptr ? len() : min(len(), *limit); i<l; i++)
			if (!in(s[i], tuple)) break;
		return String(s + i, len() - i);
	}

	String ltrim(const Array<Char> &arr, Nullable<int> limit = nullptr) const;

	String rtrim(Char c, Nullable<int> limit = nullptr) const
	{
		const char16_t *s = data();
		int l = len()-1;
		for (int ll=limit == nullptr ? 0 : max(0, len()-*limit); l>=ll; l--)
			if (s[l] != c) break;
		return String(s, l+1);
	}

	String rtrim(const String &str, Nullable<int> limit = nullptr) const
	{
		const char16_t *s = data();
		int l = len()-str.len();
		for (int lim = limit == nullptr ? -1 : *limit; l>=0; l-=str.len())
			if (memcmp(str.c_str(), s+l, str.len()*sizeof(char16_t)) || lim-- == 0) break;
		return String(s, l+str.len());
	}

	template <typename ... Types> String rtrim(const Tuple<Types...> &tuple, Nullable<int> limit = nullptr) const
	{
		const char16_t *s = data();
		int l = len()-1;
		for (int ll=limit == nullptr ? 0 : max(0, len()-*limit); l>=ll; l--)
			if (!in(s[l], tuple)) break;
		return String(s, l+1);
	}

	String rtrim(const Array<Char> &arr, Nullable<int> limit = nullptr) const;

	template <typename Ty> String trim(const Ty &s) const
	{
		return ltrim(s).rtrim(s);
	}

	//String &operator=(const String &s) {assign(s); return *this;}

	friend String &&operator*(String &&s, int n)
	{
		size_t s_len = s.length();
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

	String operator[](const Range<int, true,  true > range) const {return slice(max(range.b    , 0), min((unsigned)range.e + 1u, (unsigned)len()));} // `(unsigned)` is needed when `instr` starts with left single quotation mark
	String operator[](const Range<int, true,  false> range) const {return slice(max(range.b    , 0), min((unsigned)range.e     , (unsigned)len()));}
	String operator[](const Range<int, false, true > range) const {return slice(max(range.b + 1, 0), min((unsigned)range.e + 1u, (unsigned)len()));}
	String operator[](const Range<int, false, false> range) const {return slice(max(range.b + 1, 0), min((unsigned)range.e     , (unsigned)len()));}
	String operator[](const RangeEI<int>             range) const {return slice(max(range.b    , 0), len());}

	String operator[](const range_e_llen    range) const {return (*this)[range_el(        range.b, len() + range.e)];}
	String operator[](const range_elen_elen range) const {return (*this)[range_ee(len() + range.b, len() + range.e)];}
	String operator[](const range_elen_llen range) const {return (*this)[range_el(len() + range.b, len() + range.e)];}
	String operator[](const range_elen_i    range) const {return (*this)[range_ei(len() + range.b)];}

	char16_t at_plus_len(int i) const
	{
		return (*this)[len() + i];
	}

	void set(int i, char16_t c)
	{
		if (in(i, range_el(0, len())))
			at(i) = c;
		else
			throw IndexError(i);
	}

	void set_plus_len(int i, char16_t c)
	{
		set(len() + i, c);
	}

	bool operator==(Char ch) const {return   len() == 1 && at(0) == ch.code ;}
	bool operator!=(Char ch) const {return !(len() == 1 && at(0) == ch.code);}
	friend bool operator==(Char ch, const String &s) {return   s.len() == 1 && s.at(0) == ch.code ;}
	friend bool operator!=(Char ch, const String &s) {return !(s.len() == 1 && s.at(0) == ch.code);}

	template <int N> bool operator==(const char16_t (&s)[N]) const {return   len() == N-1 && memcmp(c_str(), s, (N-1)*sizeof(char16_t)) == 0 ;}
	template <int N> bool operator!=(const char16_t (&s)[N]) const {return !(len() == N-1 && memcmp(c_str(), s, (N-1)*sizeof(char16_t)) == 0);}

	String operator+(const String &s) const {String r(*this); r.append(s); return r;}
	String operator+(Char ch) {String r(*this); r.append(1, ch.code); return r;}
	String operator+(char16_t ch) {String r(*this); r.append(1, ch); return r;}

	String operator+(int i) {return *this + String(i);}
	friend String operator+(int i, const String &s) {return String(i) + s;}
	friend String operator+(Char ch, const String &s) {return String(ch) + s;}
	friend String operator+(char16_t ch, const String &s) {return String(ch) + s;}
};

inline String operator+(Char ch1, Char ch2)
{
	return String(ch1) + ch2;
}

inline String operator ""_S(const char16_t *s, size_t sz)
{
	return String(s, sz);
}

inline Char operator ""_C(char16_t c)
{
	return Char(c);
}

inline int parse_int(const String &str)
{
	int res = 0, sign = 1;
	const char16_t *s = str.c_str();
	while (*s && (*s == u' ' || *s == u'\t')) s++; // skip whitespace
	if (*s == u'-') sign=-1, s++; else if (*s == u'+') s++;
	for (; Char(*s).is_digit(); s++)
		res = res * 10 + (*s - u'0');
	return res * sign;
}

inline double parse_float(const char16_t *s)
{
	double res = 0, f = 1, sign = 1;
	while (*s && (*s == u' ' || *s == u'\t')) s++; // skip whitespace
	// Sign
	if (*s == u'-') sign=-1, s++; else if (*s == u'+') s++;
	// Integer part
	for (; Char(*s).is_digit(); s++) // check for '\0' is not needed because it is included in `is_digit()`
		res = res * 10 + (*s - u'0');
	// Fractional part
	if (*s == '.')
		for (s++; Char(*s).is_digit(); s++)
			res += (f *= 0.1) * (*s - u'0');
	return res * sign;
}
inline double parse_float(const String &s) {return parse_float(s.c_str());}

inline bool in(Char c, const String &s) {return s.find(c) != nullptr;}

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
