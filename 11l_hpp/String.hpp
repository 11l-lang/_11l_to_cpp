#include <string>

class Char
{
public:
	int code;

	Char(int code) : code(code) {}

	bool operator< (Char c) const {return code <  c.code;}
	bool operator<=(Char c) const {return code <= c.code;}
	bool operator> (Char c) const {return code >  c.code;}
	bool operator>=(Char c) const {return code >= c.code;}

	bool operator==(Char c) const {return code == c.code;}
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
	String(char16_t c) : basic_string(1, c) {}
	String(const char16_t *&s) : basic_string(s) {} // reference is needed here because otherwise String(const char16_t (&s)[N]) is never called (`String(u"str")` calls `String(const char16_t *s)`)
	String(const char16_t *s, size_t sz) : basic_string(s, sz) {}
	template <int N> String(const char16_t (&s)[N]): basic_string(s, N-1) {}

	int len() const { return size(); } // return `int` (not `size_t`) to avoid warning C4018: '<': signed/unsigned mismatch

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

	Char operator[](int pos) const
	{
		return Char(at(pos));
	}

	String operator[](const Range<int, true,  true > &range) const {return slice(max(range.b    , 0), min(range.e + 1, len()));}
	String operator[](const Range<int, true,  false> &range) const {return slice(max(range.b    , 0), min(range.e    , len()));}
	String operator[](const Range<int, false, true > &range) const {return slice(max(range.b + 1, 0), min(range.e + 1, len()));}
	String operator[](const Range<int, false, false> &range) const {return slice(max(range.b + 1, 0), min(range.e    , len()));}
	String operator[](const RangeEI<int>             &range) const {return slice(max(range.b    , 0),                    len() );}

	bool operator==(Char ch) const {return   len() == 1 && at(0) == ch.code ;}
	bool operator!=(Char ch) const {return !(len() == 1 && at(0) == ch.code);}
	friend bool operator==(Char ch, const String &s) {return   s.len() == 1 && s.at(0) == ch.code ;}
	friend bool operator!=(Char ch, const String &s) {return !(s.len() == 1 && s.at(0) == ch.code);}

	template <int N> bool operator==(const char16_t (&s)[N]) const {return   len() == N-1 && memcmp(c_str(), s, (N-1)*sizeof(char16_t)) == 0 ;}
	template <int N> bool operator!=(const char16_t (&s)[N]) const {return !(len() == N-1 && memcmp(c_str(), s, (N-1)*sizeof(char16_t)) == 0);}
};

String operator ""_S(const char16_t *s, size_t sz)
{
	return String(s, sz);
}

Char operator ""_C(char16_t c)
{
	return Char(c);
}
