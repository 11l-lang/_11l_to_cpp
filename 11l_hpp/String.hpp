#include <string>
#include <cstring> // for memcmp in GCC
#include <cwctype>
#include <array>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

static class LocaleInitializer
{
public:
	LocaleInitializer() {
		setlocale(LC_CTYPE, ""); // for correct work of `towlower()` and `iswlower()` [example: `print('Ф'.lower() == 'ф')`]
#ifdef _WIN32
		_setmode(_fileno(stdout), _O_U8TEXT); // [https://stackoverflow.com/questions/2492077/output-unicode-strings-in-windows-console-app <- google:‘wcout cyrillic msvc widechartomultibyte’]
#endif
	}
} locale_initializer;

class Char
{
public:
	char16_t code;

	Char(char16_t code) : code(code) {}
	explicit Char(const class BigInt &b);
	explicit Char(const String &s);

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

	void operator++() {++code;}
//	Char operator+(Int i) const {return Char(code + (int)i);}
	Char operator-(Int i) const {return Char(code - (int)i);}
	Int operator-(Char c) const {return code - c.code;}

	Char    lowercase() const {return towlower(code);}
	bool is_lowercase() const {return iswlower(code);}
	Char    uppercase() const {return towupper(code);}
	bool is_uppercase() const {return iswupper(code);}
	bool is_alpha    () const {return iswalpha(code);}

	Array<Byte> encode(const String &encoding) const;
};

inline Char operator ""_C(char16_t c)
{
	return Char(c);
}

namespace re {class RegEx;}

class String : public std::u16string
{
	String slice(Int begin, Int end) const
	{
		return String(c_str() + begin, end - begin);
	}
	String slice(Int begin, Int end, Int step) const
	{
		String r;
		if (step > 0)
			for (Int i = begin; i < end; i += step)
				r.append(1, at(i));
		else
			for (Int i = begin; i > end; i += step)
				r.append(1, at(i));
		return r;
	}

public:
	String() {}
	String(Char c) : basic_string(1, c.code) {}
	String(std::u16string &&s) : std::u16string(std::forward<std::u16string>(s)) {}
	explicit String(char16_t c) {assign(int(c));}
	explicit String(bool b) : basic_string(b ? u"1B" : u"0B", 2) {}
	explicit String(int num) {assign(num);}
	explicit String(Int64 num) {assign(num);}
	template <typename IntType> void assign_int(IntType num)
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
	void assign(int   num) { assign_int(num); }
	void assign(Int64 num) { assign_int(num); }
	explicit String(float  num, int digits = 6, bool remove_trailing_zeroes = true) {assign(num, digits, remove_trailing_zeroes);}
	explicit String(double num, int digits = 9, bool remove_trailing_zeroes = true) {assign(num, digits, remove_trailing_zeroes);}
	explicit String(const char16_t *&s) : basic_string(s) {} // reference is needed here because otherwise `String(const char16_t (&s)[N])` is never called (`String(u"str")` calls `String(const char16_t *s)`)
	explicit String(const char16_t *&&s) : basic_string(s) {} // to fix `String(path.c_str() + sep_pos + 1)` in `fs::base_name(u"./file_name")` (otherwise [without this constructor] `explicit String(bool b)` is called)
	String(const char16_t *s, size_t sz) : basic_string(s, sz) {}
	//String(const char16_t *s) : basic_string(s) {} // commented out because otherwise `String(const char16_t (&s)[N])` is never called
	String(nullptr_t) = delete;
	template <int N> String(const char16_t (&s)[N]) : basic_string(s, N-1) {}
	template <typename Ty> explicit String(const Ty s, const Ty e) : basic_string(s, e) {}
	template <typename Ty, typename = std::enable_if_t<std::is_enum<Ty>::value>> explicit String(const Ty e) : String(int(e)) {}
	template <typename Type, int dimension> explicit String(const Tvec<Type, dimension> &v)
	{
		assign(1, u'(');
		for (int i = 0; i < dimension; i++) {
			*this &= String(v[i]);
			if (i < dimension - 1)
				*this &= u", ";
		}
		append(1, u')');
	}
	template <typename Type1, typename Type2> explicit String(const Tuple<Type1, Type2> &tuple)
	{
		assign(1, u'(');
		*this &= _get<0>(tuple);
		*this &= u", ";
		*this &= _get<1>(tuple);
		append(1, u')');
	}
	template <typename...Types> explicit String(const Tuple<Types...> &t)
	{
		assign(1, u'(');
		for (auto &&el : t) {
			if (len() > 1)
				*this &= u", ";
			*this &= String(el);
		}
		append(1, u')');
	}
	template <typename Type> explicit String(const Array<Type> &arr)
	{
		assign(1, u'[');
		for (Int i=0; i<arr.len(); i++) {
			*this &= String(arr[i]);
			if (i < arr.len()-1) *this &= u", ";
		}
		append(1, u']');
	}
	explicit String(Complex c)
	{
		if (abs(c.imag()) < 1e-9)
			assign(c.real());
		else {
			if (abs(c.real()) < 1e-9)
				assign(c.imag());
			else {
				assign(c.real());
				if (c.imag() > 0)
					append(1, u'+');
				(*this) &= String(c.imag());
			}
			append(1, u'i');
		}
	}

	using std::u16string::assign;
	void assign(double num, int digits = 9, bool remove_trailing_zeroes = true)
	{
		if (!isfinite(num)) {clear(); return;}

		if (fabs(num) > 1e9 || (fabs(num) < 1e-6 && num != 0))
		{
			double exponent = floor(log10(fabs(num)));
			num *= pow(.1, exponent);
			if (fabs(num) > 1e9) return; // infinite recursion elimination
			assign(num, digits);
			append(1, u'e');
			append(String(exponent));
			return;
		}

		num += sign(num) * pow(.1, digits) * .5; // adjust number (0.199999 -> 0.2)
		if ((Int64)num)
			assign((Int64)num);
		else // for numbers in range (-1; 0), as `(int)num` for them equals to 0
		{
			clear();
			if (num < 0) append(1, u'-');
			append(1, u'0');
		}
		num = fabs(num - (double)(Int64)num);
		if (digits > 0)
		{
			append(1, u'.');
			for (int i=0; i<digits; i++)
			{
				num *= 10.;
				append(1, u'0'+int(num));
				num -= (double)(Int64)num;
			}
			// Remove trailing zeroes ...
			if (remove_trailing_zeroes) {
				size_t l = length() - 1;
				while (at(l) == u'0') l--;
				if (at(l) == u'.') l--; // ... and dot/period.
				resize(l + 1);
			}
		}
	}

	class Iterator
	{
		char16_t *c;
	public:
		explicit Iterator(char16_t *c) : c(c) {}
		bool operator!=(Iterator i) {return c != i.c;}
		void operator++() {c++;}
//		Iterator operator--()    {return Iterator(--c);}         // prefix
//		Iterator operator--(int) {Iterator r(c); c--; return r;} // postfix
		Char &operator*() {return (Char&)*c;}
	};
	Iterator begin() {return Iterator(const_cast<char16_t*>(data()));}
	Iterator end()   {return Iterator(const_cast<char16_t*>(data()) + len());}
	Iterator begin() const {return Iterator(const_cast<char16_t*>(data()));}
	Iterator end()   const {return Iterator(const_cast<char16_t*>(data() + len()));}

	Int len() const { return (Int)size(); } // return `int` (not `size_t`) to avoid warning C4018: '<': signed/unsigned mismatch

	bool starts_with(const char16_t *s, size_t sz) const
	{
		return len() >= (Int)sz && memcmp(data(), s, sz*sizeof(char16_t)) == 0;
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
		return len() >= (Int)sz && memcmp(data() + len() - sz, s, sz*sizeof(char16_t)) == 0;
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

	/*
	String replace(const String &old, const String &n) const
	{
		String str(*this);
		size_t start_pos = 0;
		while((start_pos = str.std::u16string::find(old, start_pos)) != str.npos) {
			str.std::u16string::replace(start_pos, old.length(), n);
			start_pos += n.length();
		}
		return str;
	}
	*/
	String replace(const String &from, const String &to) const // [https://stackoverflow.com/a/29752943/2692494 <- google:‘replace_all c++’]
	{
		String newString;
		newString.reserve(length());

		size_t lastPos = 0;
		size_t findPos;

		while ((findPos = std::u16string::find(from, lastPos)) != std::u16string::npos) {
			newString.append(*this, lastPos, findPos - lastPos);
			newString &= to;
			lastPos = findPos + from.length();
		}

		newString.append(*this, lastPos, length() - lastPos);
		return newString;
	}

	String replace(const re::RegEx &regex, const String &repl) const;

	Nullable<Int> find(Char c, Int start = 0) const
	{
		const char16_t *s = data();
		for (Int i=start, n=len(); i<n; i++)
			if (s[i] == c) return Nullable<Int>(i);
		return Nullable<Int>();
	}

	Nullable<Int> find(const String &s, Int start = 0) const
	{
		size_t r = basic_string::find(s, start);
		return r == npos ? Nullable<Int>() : Nullable<Int>((Int)r);
	}

	Nullable<Int> find(const Tuple<String, String> &t, Int start = 0) const
	{
		for (Int i = start, l = len() - min(std::get<0>(t).len(), std::get<1>(t).len()); i <= l; i++) {
			if (i <= len() - std::get<0>(t).len() && memcmp(c_str() + i, std::get<0>(t).c_str(), std::get<0>(t).len() * sizeof(char16_t)) == 0) return i;
			if (i <= len() - std::get<1>(t).len() && memcmp(c_str() + i, std::get<1>(t).c_str(), std::get<1>(t).len() * sizeof(char16_t)) == 0) return i;
		}
		return nullptr;
	}

	Int findi(Char c, Int start = 0) const
	{
		const char16_t *s = data();
		for (Int i=start, n=len(); i<n; i++)
			if (s[i] == c) return i;
		return -1;
	}

	Int findi(const String &s, Int start = 0) const
	{
		size_t r = basic_string::find(s, start);
		return r != String::npos ? (Int)r : -1;
	}

	template <typename ValType> Int index(const ValType &v) const;

	Nullable<Int> rfind(const String &s, Int start = (Int)npos) const
	{
		size_t r = basic_string::rfind(s, start);
		return r == npos ? Nullable<Int>() : Nullable<Int>((Int)r);
	}

	Int rfindi(const String &sub, Int start, Int end) const
	{
		if (end < sub.len())
			return -1;
		size_t r = basic_string::rfind(sub, end - sub.len());
		if (r == String::npos || (Int)r < start) return -1;
		return (Int)r;
	}

	Int count(const char16_t *s, size_t sz) const
	{
		Int c = 0;
		for (Int i=0; i<len();)
			if (memcmp(s, data() + i, sz * sizeof(char16_t)) == 0) {
				c++;
				i += (Int)sz;
			} else
				i++;
		return c;
	}
	template <int N> Int count(const char16_t(&s)[N]) const
	{
		return count(s, N-1);
	}
	Int count(const String &s) const
	{
		return count(s.data(), s.len());
	}

	Array<String> split(const String &delim, Nullable<Int> limit = nullptr, bool group_delimiters = false) const;
	template <typename ... Types> Array<String> split(const Tuple<Types...> &delim_tuple, Nullable<Int> limit = nullptr, bool group_delimiters = false) const;
	Array<String> split(const re::RegEx &regex) const;
	Array<String> split_py() const;

	bool is_digit() const
	{
		if (empty()) return false;
		const char16_t *s = data();
		for (Int i=0, n=len(); i<n; i++)
			if (!Char(s[i]).is_digit()) return false;
		return true;
	}

	bool is_alpha() const
	{
		if (empty()) return false;
		const char16_t *s = data();
		for (Int i=0, n=len(); i<n; i++)
			if (!Char(s[i]).is_alpha()) return false;
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

	String capitalize() const
	{
		String r(*this);
		r[0] = towupper(at(0));
		return r;
	}

	String zfill(Int width) const
	{
		return String(u"0") * max(width - len(), Int(0)) & *this;
	}

	String center(Int width, String fillchar = String(u' '_C)) const
	{
		if (fillchar.len() != 1)
			throw AssertionError();
		Int w = width / 2 - len() / 2;
		return fillchar * (width - len() - w) & *this & fillchar * w;
	}

	String ljust(Int width, String fillchar = String(u' '_C)) const
	{
		if (fillchar.len() != 1)
			throw AssertionError();
		return *this & fillchar * (width - len());
	}

	String rjust(Int width, String fillchar = String(u' '_C)) const
	{
		if (fillchar.len() != 1)
			throw AssertionError();
		return fillchar * (width - len()) & *this;
	}

	String ltrim(Char c, Nullable<Int> limit = nullptr) const
	{
		const char16_t *s = data();
		int i = 0;
		for (Int l=limit == nullptr ? len() : min(len(), *limit); i<l; i++)
			if (s[i] != c) break;
		return String(s + i, len() - i);
	}

	String ltrim(const String &str, Nullable<Int> limit = nullptr) const
	{
		const char16_t *s = data();
		Int i = 0, l = len()-str.len();
		for (Int lim = limit == nullptr ? -1 : *limit; i<=l; i+=str.len())
			if (memcmp(str.c_str(), s+i, str.len()*sizeof(char16_t)) || lim-- == 0) break;
		return String(s + i, len() - i);
	}

	template <typename ... Types> String ltrim(const Tuple<Types...> &tuple, Nullable<Int> limit = nullptr) const
	{
		const char16_t *s = data();
		Int i = 0;
		for (Int l=limit == nullptr ? len() : min(len(), *limit); i<l; i++)
			if (!in(s[i], tuple)) break;
		return String(s + i, len() - i);
	}

	String ltrim(const Array<Char> &arr, Nullable<Int> limit = nullptr) const;

	String rtrim(Char c, Nullable<Int> limit = nullptr) const
	{
		const char16_t *s = data();
		Int l = len()-1;
		for (Int ll=limit == nullptr ? 0 : max(Int(0), len()-*limit); l>=ll; l--)
			if (s[l] != c) break;
		return String(s, l+1);
	}

	String rtrim(const String &str, Nullable<Int> limit = nullptr) const
	{
		const char16_t *s = data();
		Int l = len()-str.len();
		for (Int lim = limit == nullptr ? -1 : *limit; l>=0; l-=str.len())
			if (memcmp(str.c_str(), s+l, str.len()*sizeof(char16_t)) || lim-- == 0) break;
		return String(s, l+str.len());
	}

	template <typename ... Types> String rtrim(const Tuple<Types...> &tuple, Nullable<Int> limit = nullptr) const
	{
		const char16_t *s = data();
		Int l = len()-1;
		for (Int ll=limit == nullptr ? 0 : max(Int(0), len()-*limit); l>=ll; l--)
			if (!in(s[l], tuple)) break;
		return String(s, l+1);
	}

	String rtrim(const Array<Char> &arr, Nullable<Int> limit = nullptr) const;

	template <typename Ty> String trim(const Ty &s) const
	{
		return ltrim(s).rtrim(s);
	}

	template <typename Func> auto map(Func &&func) const -> Array<decltype(func(std::declval<Char>()))>
	{
		Array<decltype(func(std::declval<Char>()))> r;
		r.reserve(len());
		for (Char c : *this)
			r.push_back(func(c));
		return r;
	}

	template <typename Func> Array<Char> filter(Func &&func) const;

	template <typename Type, typename Func> Type reduce(const Type &initial, Func &&func) const
	{
		Type r = initial;
		for (Char c : *this)
			r = func(r, c);
		return r;
	}

	//String &operator=(const String &s) {assign(s); return *this;}

	friend String &&operator*(String &&s, Int n)
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
	friend String &&operator*(Int n, String &&s)
	{
		return std::move(s) * n;
	}
	String operator*(Int n) const
	{
		return String(*this) * n;
	}

	Char &operator[](size_t pos)
	{
		return (Char&)at(pos);
	}
	const Char operator[](size_t pos) const
	{
		return Char(at(pos));
	}

	String operator[](const Range<Int, true,  true > range) const {return slice(max(range.b    , Int(0)), min((unsigned)range.e + 1u, (unsigned)len()));} // `(unsigned)` is needed when `instr` starts with left single quotation mark
	String operator[](const Range<Int, true,  false> range) const {return slice(max(range.b    , Int(0)), min((unsigned)range.e     , (unsigned)len()));}
	String operator[](const Range<Int, false, true > range) const {return slice(max(range.b + 1, Int(0)), min((unsigned)range.e + 1u, (unsigned)len()));}
	String operator[](const Range<Int, false, false> range) const {return slice(max(range.b + 1, Int(0)), min((unsigned)range.e     , (unsigned)len()));}
	String operator[](const RangeEI<Int>             range) const {return slice(max(range.b    , Int(0)), len());}
	String operator[](const RangeEIWithStep<Int>     range) const {return slice(max(range.b    , Int(0)), range.step > 0 ? len() : -1, range.step);}

	String operator[](const range_e_llen    range) const {return (*this)[range_el(        range.b, len() + range.e)];}
	String operator[](const range_elen_elen range) const {return (*this)[range_ee(len() + range.b, len() + range.e)];}
	String operator[](const range_elen_llen range) const {return (*this)[range_el(len() + range.b, len() + range.e)];}
	String operator[](const range_elen_i    range) const {return (*this)[range_ei(len() + range.b)];}
	String operator[](const range_elen_i_wstep range) const {return (*this)[range_ei(len() + range.b).step(range.s)];}

	Char last() const
	{
		if (empty())
			throw IndexError(0);
		return (*this)[len() - 1];
	}

	Char at_plus_len(Int i) const
	{
		return (*this)[len() + i];
	}

	void set(Int i, char16_t c)
	{
		if (in(i, range_el(Int(0), len())))
			at(i) = c;
		else
			throw IndexError(i);
	}

	void set_plus_len(Int i, char16_t c)
	{
		set(len() + i, c);
	}

	bool operator==(Char ch) const {return   len() == 1 && at(0) == ch.code ;}
	bool operator!=(Char ch) const {return !(len() == 1 && at(0) == ch.code);}
	friend bool operator==(Char ch, const String &s) {return   s.len() == 1 && s.at(0) == ch.code ;}
	friend bool operator!=(Char ch, const String &s) {return !(s.len() == 1 && s.at(0) == ch.code);}

	template <int N> bool operator==(const char16_t (&s)[N]) const {return   len() == N-1 && memcmp(c_str(), s, (N-1)*sizeof(char16_t)) == 0 ;}
	template <int N> bool operator!=(const char16_t (&s)[N]) const {return !(len() == N-1 && memcmp(c_str(), s, (N-1)*sizeof(char16_t)) == 0);}

	String operator+(const String &s) const = delete;
	template <typename Ty> String operator+(const Ty &obj) const = delete;
	template <typename Ty> void operator+=(const Ty &obj) const = delete;
	template <typename Ty> friend String operator+(const Ty &obj, const String &s) = delete;

	void operator&=(const char16_t *s) {append(s);}
	void operator&=(const String &s) {append(s);}
	void operator&=(Char ch) {append(1, ch.code);}
	void operator&=(char16_t ch) {*this &= String(ch);}
	void operator&=(int i)    {*this &= String(i);}
	void operator&=(double n) {*this &= String(n);}
	template <typename Ty> void operator&=(const Ty &obj) {*this &= String(obj);}

	String operator&(const char16_t *s) const {String r(*this); r.append(s); return r;}
	String operator&(const String &s) const {String r(*this); r.append(s); return r;}
	String operator&(Char ch)         const {String r(*this); r.append(1, ch.code); return r;}

	String operator&(char16_t ch) const {return *this & String(ch);}
	String operator&(int i)    const {return *this & String(i);}
	String operator&(bool b)   const {return *this & String(b);}
	String operator&(double n) const {return *this & String(n);}

	template <typename Ty> String operator&(const Ty &obj) const {return *this & String(obj);}

	friend String operator&(int i, const String &s) {return String(i) & s;}
	friend String operator&(bool b, const String &s) {return String(b) & s;}
	friend String operator&(double n, const String &s) {return String(n) & s;}
	friend String operator&(Char ch, const String &s) {return String(ch) & s;}
	friend String operator&(char16_t ch, const String &s) {return String(ch) & s;}

	template <typename Ty> friend String operator&(const Ty &obj, const String &s) {return String(obj) & s;}

	std::string to_string() const
	{
		std::string s;
		s.reserve(length());
		for (char16_t c : *this)
			s.append(1, (char)c);
		return s;
	}

	Array<Byte> encode(const String &encoding = u"utf-8") const;

private:
	struct FormatArgument;//Field

	template <size_t size> static void fill_in_format_arguments(std::array<FormatArgument, size> &arr_args, int index)
	{
		if (index != size)
			throw AssertionError();
	}

	template <size_t size, typename Type, typename ... Types> static void fill_in_format_arguments(std::array<FormatArgument, size> &arr_args, int index, const Type &arg, const Types&... args)
	{
		arr_args[index].set(arg);
		fill_in_format_arguments(arr_args, index + 1, args...);
	}
public:
	template <typename ... Types> String format(const Types&... args) const;
};

struct String::FormatArgument//Field
{
	enum class Type {STRING, INTEGER, FLOAT} type;
	union {
		const String *string;
		double f;
		Int64 i;
	};
	String s;

	void set(int n)
	{
		type = Type::INTEGER;
		i = n;
	}
	void set(Int64 n)
	{
		type = Type::INTEGER;
		i = n;
	}
	void set(char16_t n)
	{
		type = Type::INTEGER;
		i = n;
	}
	void set(float n)
	{
		type = Type::FLOAT;
		f = n;
	}
	void set(double n)
	{
		type = Type::FLOAT;
		f = n;
	}
	void set(const String &s)
	{
		type = Type::STRING;
		string = &s;
	}
	template <typename Ty> void set(const Ty &v)
	{
		type = Type::STRING;
		s = String(v);
		string = &s;
	}
};

template <typename ... Types> String String::format(const Types&... args) const
{
	std::array<FormatArgument, sizeof...(args)> format_arguments;
	fill_in_format_arguments(format_arguments, 0, args...);

	int argument_index = 0;
	String r;
	const char16_t *s = data();
	for (int i=0; i<len();) {
		if (s[i] == '#' && s[i+1] == '#') {
			r &= u'#'_C;
			i += 2;
			continue;
		}
		if (s[i] == '#' && (s[i+1] == '.' || s[i+1] == '<' || Char(s[i+1]).is_digit())) {
			int before_period = 0,
			     after_period = 0;
			bool left_align = false,
			     there_are_digits_after_period = false,
			     zero_padding = false;
			i++;
			if (s[i] == '<') {
				left_align = true;
				i++;
			}
			if (s[i] == '0')
				zero_padding = true;
			if (s[i] == '.' && !Char(s[i+1]).is_digit()) // #.
				i++;
			else {
				//if (Char(s[i]).is_digit())
					for (; Char(s[i]).is_digit(); i++)
						before_period = before_period*10 + (s[i] - '0');
				if (s[i] == '.' && Char(s[i+1]).is_digit()) { // the second condition is needed for a such case: `x, y: #6, #6.`
					there_are_digits_after_period = true;
					for (i++; Char(s[i]).is_digit(); i++)
						after_period = after_period*10 + (s[i] - '0');
				}
			}

			FormatArgument fa = format_arguments[argument_index++];
			if (fa.type == FormatArgument::Type::STRING) {
				if (there_are_digits_after_period)
					throw AssertionError();
				if (left_align)
					r &= *fa.string;
				r.resize(r.size() + max(before_period - fa.string->len(), Int(0)), ' ');
				if (!left_align)
					r &= *fa.string;
			}
			else {
				if (before_period == 0 && after_period == 0 && !there_are_digits_after_period) // #.
					r &= fa.type == FormatArgument::Type::INTEGER ? String(fa.i) : String(fa.f);
				else {
					String s; // (
					if (!there_are_digits_after_period) // && fract(fa.number) != 0)
						fa.type == FormatArgument::Type::INTEGER ? s.assign(fa.i) : s.assign(fa.f);
					else if (fa.type == FormatArgument::Type::INTEGER) {
						if (after_period != 0)
							throw AssertionError();
						s.assign(fa.i);
					}
					else
						s.assign(fa.f, after_period, false);
					if (left_align)
						r &= s;
					r.resize(r.size() + max(after_period + bool(after_period) + before_period - s.len(), Int(0)), zero_padding ? '0' : ' ');
					if (!left_align)
						r &= s;
				}
			}
		}
		else
			r &= Char(s[i++]);
	}

	if (argument_index != sizeof...(args))
		throw AssertionError();

	return r;
}

inline Char::Char(const String &s)
{
	if (s.len() != 1)
		throw AssertionError();
	code = s[0].code;
}

inline String operator&(Char ch1, Char ch2)
{
	return String(ch1) & ch2;
}

inline String operator*(Char c, Int n)
{
	return String(c) * n;
}

inline String operator*(Int n, Char c)
{
	return String(c) * n;
}

inline String operator ""_S(const char16_t *s, size_t sz)
{
	return String(s, sz);
}

template <typename Type> String to_string(const Type &v)
{
	return String(v);
}

class ValueError
{
public:
	String value;

	template <typename ValueTy> ValueError(const ValueTy &value) : value(to_string(value)) {}
};

template <typename TInt> inline TInt to_int_t(const String &str)
{
	TInt res = 0, sign = 1;
	const char16_t *s = str.c_str();
	while (*s && (*s == u' ' || *s == u'\t')) s++; // skip whitespace
	if (*s == u'-') sign=-1, s++; else if (*s == u'+') s++;
	for (; *s; s++) {
		if (!Char(*s).is_digit()) {
			while (*s == u' ' || *s == u'\t' || *s == u'\n') // why check for \n only at the end of the string with integer: \n at the beginning is very strange, while it's ok to have a file with integer ending with \n (so `int(open('...').read())` must work in this case)
				s++;
			if (*s == 0)
				break;
			throw ValueError(str);
		}
		res = res * 10 + (*s - u'0');
	}
	return res * sign;
}

inline Int to_int(const String &str)
{
	return to_int_t<Int>(str);
}

inline Int64 to_int64(const String &str)
{
	return to_int_t<Int64>(str);
}

template <typename TInt> inline TInt to_int_t(const String &str, int base)
{
	if (base > 36)
		throw ValueError(str);

	TInt res = 0, sign = 1;
	const char16_t *s = str.c_str();
	while (*s && (*s == u' ' || *s == u'\t')) s++; // skip whitespace
	if (*s == u'-') sign=-1, s++; else if (*s == u'+') s++;
	for (; *s; s++) {
		if (Char(*s).is_digit()) {
			if (*s - u'0' >= base)
				throw ValueError(str);
			res = res * base + (*s - u'0');
		}
		else if (base > 10) {
			if (in(*s, range_el(u'A', char16_t(u'A' + base - 10))))
				res = res * base + (*s - u'A' + 10);
			else if (in(*s, range_el(u'a', char16_t(u'a' + base - 10))))
				res = res * base + (*s - u'a' + 10);
			else {
				while (*s == u' ' || *s == u'\t' || *s == u'\n')
					s++;
				if (*s == 0)
					break;
				throw ValueError(str);
			}
		}
		else {
			while (*s == u' ' || *s == u'\t' || *s == u'\n')
				s++;
			if (*s == 0)
				break;
			throw ValueError(str);
		}
	}
	return res * sign;
}

inline Int to_int(const String &str, int base)
{
	return to_int_t<Int>(str, base);
}

inline Int64 to_int64(const String &str, int base)
{
	return to_int_t<Int64>(str, base);
}

inline Int to_int(Char ch)
{
	return ch.is_digit() ? ch.code - '0' : throw ValueError(ch);
}

inline Int to_int(double d)
{
	return (Int)d;
}

inline Int to_int(int i)
{
	return i;
}

inline Int to_int(uint32_t i)
{
	return i;
}

template <typename Ty, typename = std::enable_if_t<std::is_enum<Ty>::value>> Int to_int(Ty e)
{
	return Int(e);
}

inline Int to_int(Int64 i)
{
	return (Int)i;
}

inline Int64 to_int64(double d)
{
	return (Int64)d;
}

inline Int64 to_int64(int i)
{
	return i;
}

inline Int64 to_int64(Int64 i)
{
	return i;
}

inline UInt64 to_uint64( int     i) { return i; }
inline UInt64 to_uint64(uint32_t i) { return i; }
inline UInt64 to_uint64( Int64   i) { return i; }
inline UInt64 to_uint64(UInt64   i) { return i; }

inline uint32_t to_uint32(int i)
{
	return i;
}

inline double to_float(const String &str)
{
	return atof(str.to_string().c_str());
}

inline double to_float(Char ch)
{
	return ch.is_digit() ? ch.code - '0' : 0;
}

inline double to_float(int      i) { return i; }
inline double to_float(uint32_t i) { return i; }
inline double to_float(Int64    i) { return (double)i; }
inline double to_float(UInt64   i) { return (double)i; }
inline double to_float(float    f) { return f; }
inline double to_float(double   d) { return d; }

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

inline String format_float(float  x, int precision) {return String(x, precision);}
inline String format_float(double x, int precision) {return String(x, precision);}

inline bool in(Char c, const String &s) {return s.find(c) != nullptr;}
inline bool in(const String &c, const String &s) {return s.find(c) != nullptr;}

template <bool include_beginning, bool include_ending> inline bool in(const String &s, const Range<Char, include_beginning, include_ending> &range)
{
	if (s.len() == 0) return false;
	if (s.len() != 1) throw AssertionError();
	return in(s[0], range);
}

inline char hex_to_char(int c) {return (char)c + ((unsigned)c <= 9u ? '0' : 'A' - 10);}

inline String hex(int n)
{
	char16_t rr[9], *h = rr;
	rr[8] = 0;
	for (const unsigned char *d = (unsigned char*)&n + sizeof(n)-1; d >= (unsigned char*)&n; d--)
		*h++ = hex_to_char(*d >> 4),
		*h++ = hex_to_char(*d & 0xF);
	const char16_t *s = rr;
	while (*s == u'0')
		s++;
	String r;
	if (h > s)
		r.append(s, h-s);
	else
		r.append(1, u'0');
	return r;
}

inline String bin(int n)
{
	char16_t r[32], *p = r;
	for (int i=0; i<32; i++, n <<= 1)
		if (n & 0x8000'0000) {
			*p++ = u'1';
			for (i++, n <<= 1; i<32; i++, n <<= 1)
				*p++ = u'0' + (unsigned(n) >> 31);
			return String(r, p - r);
		}
	return String(u'0'_C);
}

inline String bin(Int64 n)
{
	char16_t r[64], *p = r;
	for (int i=0; i<64; i++, n <<= 1)
		if (n & 0x8000'0000'0000'0000) {
			*p++ = u'1';
			for (i++, n <<= 1; i<64; i++, n <<= 1)
				*p++ = u'0' + (UInt64(n) >> 63);
			return String(r, p - r);
		}
	return String(u'0'_C);
}

inline String reversed(const String &s)
{
	String r(s);
	std::reverse(r.std::u16string::begin(), r.std::u16string::end()); // can not use `std::reverse(r.begin(), r.end());` because of MSVC error C2794 in debug build
	return r;
}
