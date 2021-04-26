#include <algorithm>
//using std::min;
//using std::max;

#include <tuple>
template <typename...Types> using Tuple = std::tuple<Types...>;
using std::make_tuple;

//template <int n, typename Container> inline auto  _get(const Container &c) {return c[n];}
//template <int n, typename Type>      inline Type &_get(Array<Type> &arr) {return arr[n];}
template <int n, typename Container> inline auto _get(const Container &c) -> decltype(c[n]) {return c[n];}
template <int n, typename Container> inline auto _get(      Container &c) -> decltype(c[n]) {return c[n];}
template <int n, typename...Types> inline const auto &_get(const Tuple<Types...> &t) {return std::get<n>(t);}
template <int n, typename...Types> inline       auto &_get(      Tuple<Types...> &t) {return std::get<n>(t);}
template <int n, typename Container, typename Value> inline const Value &_set(Container       &c, const Value &v) {c.set(n, v); return v;}
template <int n, typename  ...Types, typename Value> inline const Value &_set(Tuple<Types...> &t, const Value &v) {return std::get<n>(t) = v;}

namespace std { // `namespace std` is necessary to avoid error C3312 in MSVC {>[https://stackoverflow.com/questions/32681697/range-based-for-loop-and-adl <- google:‘error C3312’]:‘`begin` and `end` from global namespace are not considered’}
// From [https://stackoverflow.com/questions/14261183/how-to-make-generic-computations-over-heterogeneous-argument-packs-of-a-variadic <- google:‘c++ homogeneous tuple range based for site:stackoverflow.com’]:
struct null_type {};
template<typename... Ts> struct homogeneous_type;
template<typename T>     struct homogeneous_type<T> {using type = T; static const bool is_homogeneous = true;};
template<typename T, typename... Ts>
struct homogeneous_type<T, Ts...>
{
	using type_of_remaining_parameters = typename homogeneous_type<Ts...>::type;
	static const bool is_homogeneous = std::is_same<T, type_of_remaining_parameters>::value;
	using type = typename std::conditional<is_homogeneous, T, null_type>::type;
};
template<typename... Ts> struct is_homogeneous_pack
{
	static const bool value = homogeneous_type<Ts...>::is_homogeneous;
};

template <typename...Types> class TupleIterator
{
	static_assert(is_homogeneous_pack<Types...>::value, "Tuple is not homogeneous!");
	using Type = typename tuple_element<0, tuple<Types...>>::type;
	const Type *ptr;
public:
	TupleIterator(const Type *ptr) : ptr(ptr) {}
	void operator++() {tuple<Type, Type> *t = nullptr; ptr += &get<1>(*t) - &get<0>(*t);} // pointer is used to support types without a default constructor (e.g. Char)
	const Type &operator*() {return *ptr;}
	bool operator!=(TupleIterator i) {return ptr != i.ptr;}
};
template <typename...Types> inline TupleIterator<Types...> begin(const tuple<Types...> &t) {return TupleIterator<Types...>(&get<0>(t));}
template <typename...Types> inline TupleIterator<Types...> end  (const tuple<Types...> &t) {
	tuple<typename tuple_element<0, tuple<Types...>>::type,
	      typename tuple_element<0, tuple<Types...>>::type> *tt = nullptr; // if `t` is a single element tuple, then `&get<1>(t) - &get<0>(t)` will not work
	return TupleIterator<Types...>(&get<tuple_size<tuple<Types...>>::value-1>(t) + (&get<1>(*tt) - &get<0>(*tt)));
}
}

#define TUPLE_ELEMENT_T(n, tuple) std::remove_const_t<std::remove_reference_t<decltype(_get<n>(tuple))>>

template <typename Type0, typename Type1, typename Type> void assign_from_tuple(Type0 &v0, Type1 &v1, const Type &t)
{
	v0 = _get<0>(t);
	v1 = _get<1>(t);
}
template <typename Type0, typename Type1, typename Type2, typename Type> void assign_from_tuple(Type0 &v0, Type1 &v1, Type2 &v2, const Type &t)
{
	v0 = _get<0>(t);
	v1 = _get<1>(t);
	v2 = _get<2>(t);
}
template <typename Type0, typename Type1, typename Type2, typename Type3, typename Type> void assign_from_tuple(Type0 &v0, Type1 &v1, Type2 &v2, Type3 &v3, const Type &t)
{
	v0 = _get<0>(t);
	v1 = _get<1>(t);
	v2 = _get<2>(t);
	v3 = _get<3>(t);
}
template <typename Type0, typename Type1, typename Type2, typename Type3, typename Type4, typename Type> void assign_from_tuple(Type0 &v0, Type1 &v1, Type2 &v2, Type3 &v3, Type4 &v4, const Type &t)
{
	v0 = _get<0>(t);
	v1 = _get<1>(t);
	v2 = _get<2>(t);
	v3 = _get<3>(t);
	v4 = _get<4>(t);
}

#if __GNUC__ || __INTEL_COMPILER // || __clang__ // Clang already defines __GNUC__
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x) (x)
#define unlikely(x) (x)
#endif

class NullPointerException
{};

template <typename Ty> class Nullable
{
	bool has_value;
	Ty value;
public:
	Nullable() : has_value(false) {}
	Nullable(nullptr_t) : has_value(false) {}
	Nullable(const Ty &value) : has_value(true), value(value) {}
//	Nullable(const Nullable &n) : has_value(n.has_value), value(n.value) {}
//	template <typename Type> Nullable(Type &&value) : has_value(true), value(value) {} // for `Nullable<std::function<...>>`

	bool operator==(nullptr_t) const {return !has_value;}
	bool operator!=(nullptr_t) const {return  has_value;}

	const Ty &operator*() const {if (!has_value) throw NullPointerException(); return value;}
	      Ty &operator*()       {if (!has_value) throw NullPointerException(); return value;}
};

/*template <class Ty> class OptionalMutableArgument
{
	Ty object, *p;
public:
	OptionalMutableArgument(nullptr_t) : p(&object) {}
	OptionalMutableArgument(Ty &ref) : p(&ref) {}
//	OptionalMutableArgument(const Ty &ref) : p(&const_cast<Ty&>(ref)) {}

	const Ty &operator*() const {return *p;}
	      Ty &operator*()       {return *p;}
};*/

/*template <class Ty> class MutableArgument
{
	Ty *p;
public:
//	MutableArgument(nullptr_t) : p(&object) {}
	MutableArgument(Ty &ref) : p(&ref) {}
	MutableArgument(const Ty &ref) : p(&const_cast<Ty&>(ref)) {}

	const Ty &operator*() const {return *p;}
	      Ty &operator*()       {return *p;}

	Ty *operator->() const {return p;}
};*/

template <typename Ty> Ty &make_ref(const Ty &cref)
{
	return const_cast<Ty&>(cref);
}

typedef long long Int64; // this is needed because in GCC `int64_t` is `long` in 64-bit mode
typedef unsigned long long UInt64;
#ifdef INT_IS_INT64
typedef Int64 Int;
#else
typedef int32_t Int;
#endif

class IndexError
{
public:
	Int index;

	IndexError(Int index) : index(index) {}
};

#include <memory>
class String;
class AssertionError
{
	std::unique_ptr<String> message;
public:
	AssertionError() {}
	AssertionError(const String &message);
	operator String() const;
};

typedef  int32_t  Int32;
typedef uint32_t UInt32;
typedef  int16_t  Int16;
typedef uint16_t UInt16;
typedef   int8_t  Int8;
typedef unsigned char Byte;

#include "11l_hpp/funcs.hpp"
namespace std {
template <typename Type, int dimension> inline const Type *begin(const Tvec<Type, dimension> &v) {return &v[0];}
template <typename Type, int dimension> inline const Type *end  (const Tvec<Type, dimension> &v) {return &v[0] + dimension;}
}
#include "11l_hpp/Range.hpp"
#include <iostream> // for std::wcerr in String.hpp
#include <complex>
typedef std::complex<double> Complex;
#include "11l_hpp/String.hpp"
AssertionError::AssertionError(const String &message) : message(std::make_unique<String>(message)) {}
AssertionError::operator String() const {return message != nullptr ? *message : String();}
#include "11l_hpp/Array.hpp"
#include "11l_hpp/Dict.hpp"
#include "11l_hpp/Set.hpp"
#include "11l_hpp/Deque.hpp"
#include "11l_hpp/pointers.hpp"

#include <limits> // for `std::numeric_limits<double>::infinity()` and `std::numeric_limits<int>::min()/max()`

#include <functional>
using namespace std::complex_literals;
template <typename Ty, typename Ty2> inline std::complex<Ty> operator+(const std::complex<Ty> &c, const Ty2 &n) {return std::complex<Ty>(c.real() + n, c.imag());}
template <typename Ty, typename Ty2> inline std::complex<Ty> operator+(const Ty2 &n, const std::complex<Ty> &c) {return std::complex<Ty>(c.real() + n, c.imag());}
template <typename Ty, typename Ty2> inline std::complex<Ty> operator-(const std::complex<Ty> &c, const Ty2 &n) {return std::complex<Ty>(c.real() - n, c.imag());}
template <typename Ty, typename Ty2> inline std::complex<Ty> operator-(const Ty2 &n, const std::complex<Ty> &c) {return std::complex<Ty>(n - c.real(), -c.imag());}
template <typename Ty, typename Ty2> inline std::complex<Ty> operator*(const std::complex<Ty> &c, const Ty2 &n) {return std::complex<Ty>(c.real() * n, c.imag() * n);}
template <typename Ty, typename Ty2> inline std::complex<Ty> operator*(const Ty2 &n, const std::complex<Ty> &c) {return std::complex<Ty>(c.real() * n, c.imag() * n);}
template <typename Ty, typename Ty2> inline std::complex<Ty> operator/(const std::complex<Ty> &c, const Ty2 &n) {return std::complex<Ty>(c.real() / n, c.imag() / n);}
template <typename Ty, typename Ty2> inline std::complex<Ty> operator/(const Ty2 &n, const std::complex<Ty> &c) {return Ty(n) / c;}
template <typename Ty> inline std::complex<Ty> conjugate(const std::complex<Ty> &c) {return std::conj(c);}

#define assert(...) assert_file_line(__FILE__, __LINE__, __VA_ARGS__)

/*[[noreturn]]*/ inline void assert_file_line(const char *file_name, int line, bool expression, const String &message = String())
{
	if (!expression) {
		std::wcerr << "AssertionError";
		if (!message.empty())
			std::wcerr << " '" << std::wstring(message.cbegin(), message.cend()) << "'";
		std::wcerr << " at file '" << file_name << "', line " << line << "\n";
		throw AssertionError(message);
	}
}

#include "11l_hpp/File.hpp"
#include "11l_hpp/os.hpp"
#include "11l_hpp/fs.hpp"
#include "11l_hpp/time.hpp"
#include "11l_hpp/re.hpp"
#include "11l_hpp/random.hpp"
#include "11l_hpp/minmaxheap.hpp"
namespace std {
template <> class hash<String>
{
public:
	size_t operator()(const String &s) const { return hash<u16string>()(s); }
};
}
#include "11l_hpp/ordered_map.h"
#undef assert
#define assert(...) assert_file_line(__FILE__, __LINE__, __VA_ARGS__) // to fix warning C4002 for assert with 2 arguments
#include "11l_hpp/ldf.hpp"

#include <thread>

void sleep(double secs) // I could not pick up an appropriate namespace for this function, so left it in a global namespace (like in Ruby, Julia and Groovy)
{
	std::this_thread::sleep_for(std::chrono::duration<double>(secs));
}

template <typename Ty> auto hash(const Ty &obj)
{
	return std::hash<Ty>()(obj);
}

inline void print(const String &s = u"", const String &end = u"\n", bool flush = false)
{
	std::wcout << std::wstring(s.cbegin(), s.cend()) << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

inline void print(Char c, const String &end = u"\n", bool flush = false)
{
	print(String(c), end, flush);
}

inline void print(bool b, const String &end = u"\n", bool flush = false)
{
	std::wcout << (int)b << L'B' << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

inline void print(int i, const String &end = u"\n", bool flush = false)
{
	std::wcout << i << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

inline void print(uint32_t i, const String &end = u"\n", bool flush = false)
{
	std::wcout << i << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

inline void print(Int64 i, const String &end = u"\n", bool flush = false)
{
	std::wcout << i << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

inline void print(UInt64 i, const String &end = u"\n", bool flush = false)
{
	std::wcout << i << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

inline void print(double i, const String &end = u"\n", bool flush = false)
{
	std::wcout << i << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

inline void print(Complex c, const String &end = u"\n", bool flush = false)
{
	if (c.real() != 0.0)
		std::wcout << c.real() << std::showpos << c.imag() << std::noshowpos;
	else
		std::wcout << c.imag();
	std::wcout << L'i' << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

template <typename Ty> inline void print(const Array<Ty> &arr, const String &end = u"\n", bool flush = false)
{
	std::wcout << L'[';
	for (int i=0; i<arr.len(); i++) {
		print(arr[i], u""_S);
		if (i < arr.len()-1) std::wcout << L", ";
	}
	std::wcout << L']' << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

template <typename Key, typename Value> inline void print(const Dict<Key, Value> &dict, const String &end = u"\n", bool flush = false)
{
	std::wcout << L'[';
	bool first = true;
	for (auto &&[key, value] : dict) {
		if (!first) std::wcout << L", "; else first = false;
		print(key, u" = "_S);
		print(value, u""_S);
	}
	std::wcout << L']' << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

template <typename Key, typename Value> inline void print(const DefaultDict<Key, Value> &dict, const String &end = u"\n", bool flush = false)
{
	std::wcout << L"DefaultDict([";
	bool first = true;
	for (auto &&[key, value] : dict) {
		if (!first) std::wcout << L", "; else first = false;
		print(key, u" = "_S);
		print(value, u""_S);
	}
	std::wcout << L"])" << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

template <typename Ty> inline void print(const Set<Ty> &set, const String &end = u"\n", bool flush = false)
{
	std::wcout << L"Set([";
	bool first = true;
	for (auto &&el : set) {
		if (!first) std::wcout << L", "; else first = false;
		print(el, u""_S);
	}
	std::wcout << L"])" << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

template <typename Ty> inline void print(const Deque<Ty> &d, const String &end = u"\n", bool flush = false)
{
	std::wcout << L"Deque([";
	for (int i=0; i<d.len(); i++) {
		print(d[i], u""_S);
		if (i < d.len()-1) std::wcout << L", ";
	}
	std::wcout << L"])" << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

template <typename Ty1, typename Ty2> inline void print(const Tuple<Ty1, Ty2> &t, const String &end = u"\n", bool flush = false)
{
	std::wcout << L'(';
	print(_get<0>(t), u""_S);
	std::wcout << L", ";
	print(_get<1>(t), u""_S);
	std::wcout << L')' << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

template <typename Type, int dimension> inline void print(const Tvec<Type, dimension> &v, const String &end = u"\n", bool flush = false)
{
	std::wcout << L'(';
	for (int i = 0; i < dimension; i++) {
		print(v[i], u""_S);
		if (i < dimension - 1)
			std::wcout << L", ";
	}
	std::wcout << L')' << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

template <typename Ty> inline void print_elements(const Ty &container, const String &sep = u" ", const String &end = u"\n", bool flush = false)
{
	bool first_element = true;
	std::wstring sepws(sep.cbegin(), sep.cend());
	for (auto &&el : container) {
		if (!first_element) std::wcout << sepws;
		else first_element = false;
		print(el, u""_S);
	}
	std::wcout << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

inline String input(const String &prompt = String())
{
	std::wcout << std::wstring(prompt.cbegin(), prompt.cend());
	std::wstring s;
	std::getline(std::wcin, s);
	return String(s.cbegin(), s.cend());
}

// Note: solutions like this[https://gist.github.com/mortehu/373069390c75b02f98b655e3f7dbef9a <- google:‘zip vector c++’] can not handle temp arrays (array destructed after `zip(create_array(...)...)` call)
template <typename T1, typename T2> auto zip(const T1 &arr1, const T2 &arr2)
{
	Array<decltype(make_tuple(*std::begin(arr1), *std::begin(arr2)))> r; // do not forget that `make_tuple(1, 2)` returns `Tvec<int, 2>(1, 2)` rather than `Tuple<int, int>(1, 2)`
	r.reserve(min(arr1.len(), arr2.len()));
	auto it1 = arr1.begin();
	auto it2 = arr2.begin();
	for (; it1 != arr1.end() && it2 != arr2.end(); ++it1, ++it2)
		r.push_back(make_tuple(*it1, *it2));
	return r;
}

template <typename T1, typename T2, typename T3> auto zip(const T1 &arr1, const T2 &arr2, const T3 &arr3)
{
	Array<decltype(make_tuple(*std::begin(arr1), *std::begin(arr2), *std::begin(arr3)))> r; // do not forget that `make_tuple(1, 2, 3)` returns `Tvec<int, 3>(1, 2, 3)` rather than `Tuple<int, int, int>(1, 2, 3)`
	r.reserve(min(arr1.len(), arr2.len(), arr3.len()));
	auto it1 = arr1.begin();
	auto it2 = arr2.begin();
	auto it3 = arr3.begin();
	for (; it1 != arr1.end() && it2 != arr2.end() && it3 != arr3.end(); ++it1, ++it2, ++it3)
		r.push_back(make_tuple(*it1, *it2, *it3));
	return r;
}

template <typename T1, typename T2> auto cart_product(const T1 &arr1, const T2 &arr2)
{
	Array<decltype(make_tuple(*std::begin(arr1), *std::begin(arr2)))> r;
	r.reserve(arr1.len() * arr2.len());
	for (auto &&el1 : arr1)
		for (auto &&el2 : arr2)
			r.push_back(make_tuple(el1, el2));
	return r;
}

template <typename T1, typename T2, typename T3> auto cart_product(const T1 &arr1, const T2 &arr2, const T3 &arr3)
{
	Array<decltype(make_tuple(*std::begin(arr1), *std::begin(arr2), *std::begin(arr3)))> r;
	r.reserve(arr1.len() * arr2.len() * arr3.len());
	for (auto &&el1 : arr1)
		for (auto &&el2 : arr2)
			for (auto &&el3 : arr3)
				r.push_back(make_tuple(el1, el2, el3));
	return r;
}

template <typename Type1, typename Type2, typename Func> auto multiloop(const Array<Type1> &arr1, const Array<Type2> &arr2, Func &&func) -> Array<decltype(func(std::declval<Type1>(), std::declval<Type2>()))>
{
	Array<decltype(func(std::declval<Type1>(), std::declval<Type2>()))> r;
	r.reserve(arr1.len() * arr2.len());
	for (auto &&el1 : arr1)
		for (auto &&el2 : arr2)
			r.push_back(func(el1, el2));
	return r;
}

template <typename Func> auto multiloop(const String &str1, const String &str2, Func &&func) -> Array<decltype(func(std::declval<Char>(), std::declval<Char>()))>
{
	Array<decltype(func(std::declval<Char>(), std::declval<Char>()))> r;
	r.reserve(str1.len() * str2.len());
	for (Char c1 : str1)
		for (Char c2 : str2)
			r.push_back(func(c1, c2));
	return r;
}

template <typename Type1, bool include_beginning1, bool include_ending1, typename Type2, bool include_beginning2, bool include_ending2, typename Func> auto multiloop(const Range<Type1, include_beginning1, include_ending1> &range1, const Range<Type2, include_beginning2, include_ending2> &range2, Func &&func) -> Array<decltype(func(std::declval<Type1>(), std::declval<Type2>()))>
{
	Array<decltype(func(std::declval<Type1>(), std::declval<Type2>()))> r;
	r.reserve(range1.len() * range2.len());
	for (auto v1 : range1)
		for (auto v2 : range2)
			r.push_back(func(v1, v2));
	return r;
}

template <typename Type1, typename Type2, typename FilterFunc, typename Func> auto multiloop_filtered(const Array<Type1> &arr1, const Array<Type2> &arr2, FilterFunc &&filter_func, Func &&func) -> Array<decltype(func(std::declval<Type1>(), std::declval<Type2>()))>
{
	Array<decltype(func(std::declval<Type1>(), std::declval<Type2>()))> r;
//	r.reserve(arr1.len() * arr2.len());
	for (auto &&el1 : arr1)
		for (auto &&el2 : arr2)
			if (filter_func(el1, el2))
				r.push_back(func(el1, el2));
	return r;
}

template <typename Type1, typename Type2, typename Type3, typename Func> auto multiloop(const Array<Type1> &arr1, const Array<Type2> &arr2, const Array<Type3> &arr3, Func &&func) -> Array<decltype(func(std::declval<Type1>(), std::declval<Type2>(), std::declval<Type3>()))>
{
	Array<decltype(func(std::declval<Type1>(), std::declval<Type2>(), std::declval<Type3>()))> r;
	r.reserve(arr1.len() * arr2.len() * arr3.len());
	for (auto &&el1 : arr1)
		for (auto &&el2 : arr2)
			for (auto &&el3 : arr3)
				r.push_back(func(el1, el2, el3));
	return r;
}

template <typename Type1, typename Type2, typename Type3, typename FilterFunc, typename Func> auto multiloop_filtered(const Array<Type1> &arr1, const Array<Type2> &arr2, const Array<Type3> &arr3, FilterFunc &&filter_func, Func &&func) -> Array<decltype(func(std::declval<Type1>(), std::declval<Type2>(), std::declval<Type3>()))>
{
	Array<decltype(func(std::declval<Type1>(), std::declval<Type2>(), std::declval<Type3>()))> r;
//	r.reserve(arr1.len() * arr2.len() * arr3.len());
	for (auto &&el1 : arr1)
		for (auto &&el2 : arr2)
			for (auto &&el3 : arr3)
				if (filter_func(el1, el2, el3))
					r.push_back(func(el1, el2, el3));
	return r;
}

template <typename Iterable> bool all(const Iterable &i)
{
	for (auto &&el : i)
		if (!el)
			return false;
	return true;
}

template <typename Iterable> bool any(const Iterable &i)
{
	for (auto &&el : i)
		if (el)
			return true;
	return false;
}

template <typename Ty> Ty copy(const Ty &obj)
{
	return obj;
}

#ifdef _WIN32
#define MAIN_WITH_ARGV() wmain(int argc, wchar_t *argv[])
#define INIT_ARGV() for (int i=0; i<argc; i++) ::argv.append(String((char16_t*)argv[i], wcslen(argv[i])))
#else
#define MAIN_WITH_ARGV() main(int argc, char *argv[])
#define INIT_ARGV() for (int i=0; i<argc; i++) ::argv.append(convert_utf8_string_to_String(argv[i]))
#endif

inline void exit(const String &msg)
{
	std::wcerr << std::wstring(msg.cbegin(), msg.cend()) << "\n";
	exit(1);
}

inline void exit()
{
	exit(0);
}

class NotImplementedError {};

class RuntimeError
{
	String message;
public:
	RuntimeError(const String &message) : message(message) {}
	operator String() const {return message;}
};
