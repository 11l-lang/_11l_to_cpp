#include <algorithm>
using std::min;
using std::max;

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

inline void print(const String &s, const String &end = u"\n")
{
	std::wcout << std::wstring(s.begin(), s.end()) << std::wstring(end.begin(), end.end());
}

inline void print(int i, const String &end = u"\n")
{
	std::wcout << i << std::wstring(end.begin(), end.end());
}

template <typename Ty> inline void print(const Array<Ty> &arr, const String &end = u"\n")
{
	std::wcout << L'[';
	for (int i=0; i<arr.len(); i++) {
		print(arr[i], u""_S);
		if (i < arr.len()-1) std::wcout << L", ";
	}
	std::wcout << L']' << std::wstring(end.begin(), end.end());
}

#include <tuple>
template <typename...Types> using Tuple = std::tuple<Types...>;
using std::make_tuple;

template <int n, typename Container> inline auto _get(const Container &c) {return c[n];}
template <int n, typename...Types> inline auto _get(const Tuple<Types...> &t) {return std::get<n>(t);}
