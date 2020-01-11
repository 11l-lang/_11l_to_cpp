﻿#include <string>
#include <cstring> // for memcmp in GCC
#include <cwctype>
#include <array>

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

	void operator++() {++code;}
	Char operator+(int i) const {return Char(code + i);}
	Char operator-(int i) const {return Char(code - i);}
	int operator-(Char c) const {return Char(code - c.code);}

	Char    lowercase() const {return towlower(code);}
	bool is_lowercase() const {return iswlower(code);}
	Char    uppercase() const {return towupper(code);}
	bool is_uppercase() const {return iswupper(code);}
	bool is_alpha    () const {return iswalpha(code);}
};

namespace re {class RegEx;}

class String : public std::u16string
{
	String slice(int begin, int end) const
	{
		return String(c_str() + begin, end - begin);
	}
	String slice(int begin, int end, int step) const
	{
		String r;
		if (step > 0)
			for (int i = begin; i < end; i += step)
				r.append(1, at(i));
		else // for `[::-1]` [-TODO: fix `[0::-1]` and `[r:l:-1]`-]
			for (int i = end - 1; i >= begin; i += step)
				r.append(1, at(i));
		return r;
	}

public:
	String() {}
	String(Char c) : basic_string(1, c.code) {}
	String(std::u16string &&s) : std::u16string(std::forward<std::u16string>(s)) {}
	explicit String(char16_t c) : basic_string(1, c) {}
	explicit String(bool b) : basic_string(b ? u"1B" : u"0B", 2) {}
	explicit String(int num) {assign(num);}
	explicit String(int64_t num) {assign(num);}
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
	void assign(int     num) { assign_int(num); }
	void assign(int64_t num) { assign_int(num); }
	explicit String(float  num, int digits = 6, bool remove_trailing_zeroes = true) {assign(num, digits, remove_trailing_zeroes);}
	explicit String(double num, int digits = 9, bool remove_trailing_zeroes = true) {assign(num, digits, remove_trailing_zeroes);}
	explicit String(const char16_t *&s) : basic_string(s) {} // reference is needed here because otherwise String(const char16_t (&s)[N]) is never called (`String(u"str")` calls `String(const char16_t *s)`)
	String(const char16_t *s, size_t sz) : basic_string(s, sz) {}
	String(const char16_t *s) : basic_string(s) {}
	template <int N> String(const char16_t (&s)[N]) : basic_string(s, N-1) {}
	template <typename Ty> explicit String(const Ty s, const Ty e) : basic_string(s, e) {}
	template <typename Ty, typename = std::enable_if_t<std::is_enum<Ty>::value>> explicit String(const Ty e) : String(int(e)) {}
	template <typename Type, int dimension> explicit String(const Tvec<Type, dimension> &v)
	{
		assign(1, u'(');
		for (int i = 0; i < dimension; i++) {
			*this += String(v[i]);
			if (i < dimension - 1)
				*this += u", ";
		}
		append(1, u')');
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
		if ((int)num)
			assign((int)num);
		else // for numbers in range (-1; 0), as `(int)num` for them equals to 0
		{
			clear();
			if (num < 0) append(1, u'-');
			append(1, u'0');
		}
		num = fabs(num - (double)(int)num);
		if (digits > 0)
		{
			append(1, u'.');
			for (int i=0; i<digits; i++)
			{
				num *= 10.;
				append(1, u'0'+int(num));
				num -= (double)(int)num;
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
	String replace(const re::RegEx &regex, const String &repl) const;

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

	template <typename ValType> int index(const ValType &v) const;

	Nullable<int> rfind(const String &s, int start = (int)npos) const
	{
		size_t r = basic_string::rfind(s, start);
		return r == npos ? Nullable<int>() : Nullable<int>((int)r);
	}

	int rfindi(const String &sub, int start, int end) const
	{
		size_t r = basic_string::rfind(sub, end - 1);
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

	Array<String> split(const String &delim, Nullable<int> limit = nullptr, bool group_delimiters = false) const;
	template <typename ... Types> Array<String> split(const Tuple<Types...> &delim_tuple, Nullable<int> limit = nullptr, bool group_delimiters = false) const;
	Array<String> split(const re::RegEx &regex) const;
	Array<String> split_py() const;

	bool is_digit() const
	{
		if (empty()) return false;
		const char16_t *s = data();
		for (int i=0, n=len(); i<n; i++)
			if (!Char(s[i]).is_digit()) return false;
		return true;
	}

	bool is_alpha() const
	{
		if (empty()) return false;
		const char16_t *s = data();
		for (int i=0, n=len(); i<n; i++)
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

	String zfill(int width) const
	{
		return String(u"0") * max(width - len(), 0) + *this;
	}

	String center(int width, String fillchar = String(u' ')) const
	{
		if (fillchar.len() != 1)
			throw AssertionError();
		int w = width / 2 - len() / 2;
		return fillchar * (width - len() - w) + *this + fillchar * w;
	}

	String ljust(int width, String fillchar = String(u' ')) const
	{
		if (fillchar.len() != 1)
			throw AssertionError();
		return *this + fillchar * (width - len());
	}

	String rjust(int width, String fillchar = String(u' ')) const
	{
		if (fillchar.len() != 1)
			throw AssertionError();
		return fillchar * (width - len()) + *this;
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

	template <typename Func> auto map(Func &&func) const -> Array<decltype(func(std::declval<Char>()))>
	{
		Array<decltype(func(std::declval<Char>()))> r;
		r.reserve(len());
		for (auto &&el : *this)
			r.push_back(func(el));
		return r;
	}

	template <typename Func> Array<Char> filter(Func &&func) const;

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
	String operator[](const RangeEIWithStep<int>     range) const {return slice(max(range.b    , 0), len(), range.step);}

	String operator[](const range_e_llen    range) const {return (*this)[range_el(        range.b, len() + range.e)];}
	String operator[](const range_elen_elen range) const {return (*this)[range_ee(len() + range.b, len() + range.e)];}
	String operator[](const range_elen_llen range) const {return (*this)[range_el(len() + range.b, len() + range.e)];}
	String operator[](const range_elen_i    range) const {return (*this)[range_ei(len() + range.b)];}

	Char last() const
	{
		if (empty())
			throw IndexError(0);
		return (*this)[len() - 1];
	}

	Char at_plus_len(int i) const
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

	void operator+=(const char16_t *s) {append(s);}
	void operator+=(const String &s) {append(s);}
	void operator+=(Char ch) {append(1, ch.code);}
	void operator+=(char16_t ch) {append(1, ch);}
	void operator+=(int i)    {*this += String(i);}
	void operator+=(double n) {*this += String(n);}

	String operator+(const char16_t *s) const {String r(*this); r.append(s); return r;}
	String operator+(const String &s) const {String r(*this); r.append(s); return r;}
	String operator+(Char ch)         const {String r(*this); r.append(1, ch.code); return r;}
	String operator+(char16_t ch)     const {String r(*this); r.append(1, ch); return r;}

	String operator+(int i)    const {return *this + String(i);}
	String operator+(bool b)   const {return *this + String(b);}
	String operator+(double n) const {return *this + String(n);}

	template <typename Ty> String operator+(const Ty &obj) const {return *this + String(obj);}

	friend String operator+(int i, const String &s) {return String(i) + s;}
	friend String operator+(bool b, const String &s) {return String(b) + s;}
	friend String operator+(double n, const String &s) {return String(n) + s;}
	friend String operator+(Char ch, const String &s) {return String(ch) + s;}
	friend String operator+(char16_t ch, const String &s) {return String(ch) + s;}

	template <typename Ty> friend String operator+(const Ty &obj, const String &s) {return String(obj) + s;}

	std::string to_string() const
	{
		std::string s;
		s.reserve(length());
		for (char16_t c : *this)
			s.append(1, (char)c);
		return s;
	}

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
		int64_t i;
	};
	String s;

	void set(int n)
	{
		type = Type::INTEGER;
		i = n;
	}
	void set(int64_t n)
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
	for (int i=0; i<len();)
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
					r += *fa.string;
				r.resize(r.size() + max(before_period - fa.string->len(), 0), ' ');
				if (!left_align)
					r += *fa.string;
			}
			else {
				if (before_period == 0 && after_period == 0 && !there_are_digits_after_period) // #.
					r += fa.type == FormatArgument::Type::INTEGER ? String(fa.i) : String(fa.f);
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
						r += s;
					r.resize(r.size() + max(after_period + bool(after_period) + before_period - s.len(), 0), zero_padding ? '0' : ' ');
					if (!left_align)
						r += s;
				}
			}
		}
		else
			r += s[i++];

	if (argument_index != sizeof...(args))
		throw AssertionError();

	return r;
}

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
		if (!Char(*s).is_digit())
			throw ValueError(str);
		res = res * 10 + (*s - u'0');
	}
	return res * sign;
}

inline int to_int(const String &str)
{
	return to_int_t<int>(str);
}

inline int64_t to_int64(const String &str)
{
	return to_int_t<int64_t>(str);
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
		res *= base;
		if (Char(*s).is_digit()) {
			if (*s - u'0' >= base)
				throw ValueError(str);
			res += (*s - u'0');
		}
		else if (base > 10) {
			if (in(*s, range_el(u'A', char16_t(u'A' + base - 10))))
				res += (*s - u'A' + 10);
			else if (in(*s, range_el(u'a', char16_t(u'a' + base - 10))))
				res += (*s - u'a' + 10);
			else
				throw ValueError(str);
		}
		else
			throw ValueError(str);
	}
	return res * sign;
}

inline int to_int(const String &str, int base)
{
	return to_int_t<int>(str, base);
}

inline int64_t to_int64(const String &str, int base)
{
	return to_int_t<int64_t>(str, base);
}

inline int to_int(Char ch)
{
	return ch.is_digit() ? ch.code - '0' : 0;
}

inline int to_int(double d)
{
	return (int)d;
}

inline int to_int(int i)
{
	return i;
}

inline int to_int(int64_t i)
{
	return (int)i;
}

inline int64_t to_int64(double d)
{
	return (int64_t)d;
}

inline int64_t to_int64(int i)
{
	return i;
}

inline uint64_t to_uint64( int     i) { return i; }
inline uint64_t to_uint64(uint32_t i) { return i; }
inline uint64_t to_uint64( int64_t i) { return i; }
inline uint64_t to_uint64(uint64_t i) { return i; }

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

inline double to_float(int     i) { return i; }
inline double to_float(int64_t i) { return (double)i; }
inline double to_float(float   f) { return f; }
inline double to_float(double  d) { return d; }

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
	return String(u'0');
}

inline String bin(int64_t n)
{
	char16_t r[64], *p = r;
	for (int i=0; i<64; i++, n <<= 1)
		if (n & 0x8000'0000'0000'0000) {
			*p++ = u'1';
			for (i++, n <<= 1; i<64; i++, n <<= 1)
				*p++ = u'0' + (uint64_t(n) >> 63);
			return String(r, p - r);
		}
	return String(u'0');
}

inline String reversed(const String &s)
{
	String r(s);
	std::reverse(r.std::u16string::begin(), r.std::u16string::end()); // can not use `std::reverse(r.begin(), r.end());` because of MSVC error C2794 in debug build
	return r;
}
