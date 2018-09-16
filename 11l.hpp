#include <algorithm>
using std::min;
using std::max;

#include <tuple>
template <typename...Types> using Tuple = std::tuple<Types...>;
using std::make_tuple;

#include "11l_hpp/Range.hpp"
#include "11l_hpp/String.hpp"
#include "11l_hpp/Array.hpp"
#include "11l_hpp/Dict.hpp"

#include <iostream>

#define assert(...) assert_file_line(__FILE__, __LINE__, __VA_ARGS__)

class AssertionError {};

inline void assert_file_line(const char *file_name, int line, bool expression, const String &message = String())
{
	if (!expression) {
		std::wcerr << "AssertionError";
		if (!message.empty())
			std::wcerr << " '" << std::wstring(message.begin(), message.end()) << "'";
		std::wcerr << " at file '" << file_name << "', line " << line << "\n";
		throw AssertionError();
	}
}

inline void print(const String &s, const String &end = u"\n", bool flush = false)
{
	std::wcout << std::wstring(s.begin(), s.end()) << std::wstring(end.begin(), end.end());
	if (flush)
		std::wcout.flush();
}

inline void print(int i, const String &end = u"\n", bool flush = false)
{
	std::wcout << i << std::wstring(end.begin(), end.end());
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
	std::wcout << L']' << std::wstring(end.begin(), end.end());
	if (flush)
		std::wcout.flush();
}

template <int n, typename Container> inline auto _get(const Container &c) {return c[n];}
template <int n, typename...Types> inline auto _get(const Tuple<Types...> &t) {return std::get<n>(t);}

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
