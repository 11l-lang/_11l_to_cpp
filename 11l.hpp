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
template <int n, typename Container, typename Value> inline void _set(Container       &c, const Value &v) {c.set(n, v);}
template <int n, typename  ...Types, typename Value> inline void _set(Tuple<Types...> &t, const Value &v) {std::get<n>(t) = v;}

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
	Type *ptr;
public:
	TupleIterator(Type *ptr) : ptr(ptr) {}
	void operator++() {tuple<Type, Type> t; ptr += &get<1>(t) - &get<0>(t);}
	Type &operator*() {return *ptr;}
	bool operator!=(TupleIterator i) {return ptr != i.ptr;}
};
template <typename...Types> inline TupleIterator<Types...> begin(tuple<Types...> &t) {return TupleIterator<Types...>(&get<0>(t));}
template <typename...Types> inline TupleIterator<Types...> end  (tuple<Types...> &t) {
	tuple<typename tuple_element<0, tuple<Types...>>::type,
	      typename tuple_element<0, tuple<Types...>>::type> tt; // if `t` is a single element tuple, then `&get<1>(t) - &get<0>(t)` will not work
	return TupleIterator<Types...>(&get<tuple_size<tuple<Types...>>::value-1>(t) + (&get<1>(tt) - &get<0>(tt)));
}
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

class IndexError
{
public:
	int index;

	IndexError(int index) : index(index) {}
};

class AssertionError {};

typedef unsigned char Byte;

#include "11l_hpp/funcs.hpp"
#include "11l_hpp/Range.hpp"
#include <iostream> // for std::wcerr in String.hpp
#include "11l_hpp/String.hpp"
#include "11l_hpp/Array.hpp"
#include "11l_hpp/Dict.hpp"
#include "11l_hpp/pointers.hpp"

#include <functional>

#define assert(...) assert_file_line(__FILE__, __LINE__, __VA_ARGS__)

inline void assert_file_line(const char *file_name, int line, bool expression, const String &message = String())
{
	if (!expression) {
		std::wcerr << "AssertionError";
		if (!message.empty())
			std::wcerr << " '" << std::wstring(message.cbegin(), message.cend()) << "'";
		std::wcerr << " at file '" << file_name << "', line " << line << "\n";
		throw AssertionError();
	}
}

#include "11l_hpp/File.hpp"
#include "11l_hpp/os.hpp"
#include "11l_hpp/fs.hpp"
#include "11l_hpp/time.hpp"
#include "11l_hpp/re.hpp"
#include "11l_hpp/random.hpp"

#include <thread>

void sleep(double secs) // I could not pick up an appropriate namespace for this function, so left it in a global namespace (like in Ruby, Julia and Groovy)
{
	std::this_thread::sleep_for(std::chrono::duration<double>(secs));
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

inline void print(int i, const String &end = u"\n", bool flush = false)
{
	std::wcout << i << std::wstring(end.cbegin(), end.cend());
	if (flush)
		std::wcout.flush();
}

inline void print(int64_t i, const String &end = u"\n", bool flush = false)
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

inline String input()
{
	std::wstring s;
	std::getline(std::wcin, s);
	return String(s.cbegin(), s.cend());
}

// Note: solutions like this[https://gist.github.com/mortehu/373069390c75b02f98b655e3f7dbef9a <- google:‘zip vector c++’] can not handle temp arrays (array destructed after `zip(create_array(...)...)` call)
template <typename T1, typename T2> Array<Tuple<T1, T2>> zip(const Array<T1> &arr1, const Array<T2> &arr2)
{
	Array<Tuple<T1, T2>> r;
	r.reserve(min(arr1.len(), arr2.len()));
	auto it1 = arr1.begin();
	auto it2 = arr2.begin();
	for (; it1 != arr1.end() && it2 != arr2.end(); ++it1, ++it2)
		r.push_back(make_tuple(*it1, *it2));
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

#ifdef _WIN32
#define MAIN_WITH_ARGV() wmain(int argc, wchar_t *argv[])
#define INIT_ARGV() for (int i=0; i<argc; i++) ::argv.append(String((char16_t*)argv[i], wcslen(argv[i])))
#else
#define MAIN_WITH_ARGV() main(int argc, char *argv[])
#define INIT_ARGV() for (int i=0; i<argc; i++) ::argv.append(String(convert_utf8_to_utf16(argv[i])))
#endif

inline void exit(const String &msg)
{
	_stderr.write(msg);
	_stderr.write(u"\n");
	exit(1);
}
