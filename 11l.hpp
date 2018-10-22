#include <algorithm>
using std::min;
using std::max;

#include <tuple>
template <typename...Types> using Tuple = std::tuple<Types...>;
using std::make_tuple;

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

#include "11l_hpp/Range.hpp"
#include "11l_hpp/String.hpp"
#include "11l_hpp/Array.hpp"
#include "11l_hpp/Dict.hpp"

#include <functional>
#include <iostream>

#define assert(...) assert_file_line(__FILE__, __LINE__, __VA_ARGS__)

class AssertionError {};

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
#include "11l_hpp/fs.hpp"
#include "11l_hpp/os.hpp"

inline void print(const String &s, const String &end = u"\n", bool flush = false)
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

//template <int n, typename Container> inline auto  _get(const Container &c) {return c[n];}
//template <int n, typename Type>      inline Type &_get(Array<Type> &arr) {return arr[n];}
template <int n, typename Container> inline auto _get(const Container &c) -> decltype(c[n]) {return c[n];}
template <int n, typename Container> inline auto _get(      Container &c) -> decltype(c[n]) {return c[n];}
template <int n, typename...Types> inline const auto &_get(const Tuple<Types...> &t) {return std::get<n>(t);}

// Note: solutions like this[https://gist.github.com/mortehu/373069390c75b02f98b655e3f7dbef9a <- google:‘zip vector c++’] can not handle temp arrays (array destructed after `zip(create_array(...)...)` call)
template <typename T1, typename T2> Array<Tuple<T1, T2>> zip(const Array<T1> &arr1, const Array<T2> &arr2)
{
	Array<Tuple<T1, T2>> r;
	auto it1 = arr1.begin();
	auto it2 = arr2.begin();
	for (; it1 != arr1.end() && it2 != arr2.end(); ++it1, ++it2)
		r.push_back(make_tuple(*it1, *it2));
	return r;
}

#define MAIN_WITH_ARGV() wmain(int argc, wchar_t *argv[])
#define INIT_ARGV() for (int i=0; i<argc; i++) ::argv.append(String((char16_t*)argv[i], wcslen(argv[i])))

inline void exit(const String &msg)
{
	_stderr.write(msg);
	_stderr.write(u"\n");
	exit(1);
}
