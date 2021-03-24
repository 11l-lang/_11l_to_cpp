#include <regex>

namespace re
{
class RegEx
{
	friend class ::String;
	std::wregex regex;
public:
#ifdef _WIN32
	RegEx(const String &pattern) : regex((std::wstring&)pattern) {}
#else
	RegEx(const String &pattern) : regex(std::wstring(pattern.cbegin(), pattern.cend())) {}
#endif

	class Match
	{
		friend class RegEx;
		std::wsmatch m;
#ifndef _WIN32
		std::wstring ws;
#endif
	public:
		explicit operator bool() const {return !m.empty();}
		int start(int group = 0) const {return (int)m.position(group);}
		int end  (int group = 0) const {return int(m.position(group) + m.length(group));}
#ifdef _WIN32
		String group(int group_= 0) const {return String((std::u16string&&)m.str(group_));}
#else
		String group(int group_= 0) const {std::wstring s = m.str(group_); return String(s.begin(), s.end());}
#endif
	};

	Match search(const String &s) const
	{
		Match r;
#ifdef _WIN32
		std::regex_search((const std::wstring&)s, r.m, regex);
#else
		r.ws = std::wstring(s.cbegin(), s.cend());
		std::regex_search(r.ws, r.m, regex);
#endif
		return r;
	}

	Match match(const String &s) const
	{
		Match r;
#ifdef _WIN32
		std::regex_match((const std::wstring&)s, r.m, regex);
#else
		r.ws = std::wstring(s.cbegin(), s.cend());
		std::regex_match(r.ws, r.m, regex);
#endif
		return r;
	}

	Array<String> find_strings(const String &s) const
	{
		Array<String> r;
#ifdef _WIN32
		for (std::wsregex_iterator it(((std::wstring&)s).begin(), ((std::wstring&)s).end(), regex), end; it != end; ++it)
#else
		std::wstring ws(s.cbegin(), s.cend());
		for (std::wsregex_iterator it(ws.begin(), ws.end(), regex), end; it != end; ++it)
#endif
		{
			int group = it->size() == 2 ? 1 : 0;
			r.append(s[range_el((Int)it->position(group), Int(it->position(group) + it->length(group)))]);
		}
		return r;
	}

	class Matches
	{
#ifdef _WIN32
		String s;
#else
		std::wstring ws;
#endif
		std::wregex regex;
		std::wsregex_iterator b;
		class Iterator
		{
			std::wsregex_iterator rit;
		public:
			Iterator() {}
			Iterator(const std::wsregex_iterator &rit) : rit(rit) {}

			bool operator!=(const Iterator &it) const {return rit != it.rit;}
			void operator++() {++rit;}
			const decltype(*rit) &operator*() const {return *rit;}
		} e;
	public:
#ifdef _WIN32
		Matches(String &&s_, std::wregex &&regex_) : s(std::move(s_)), regex(std::move(regex_)), b(((std::wstring&)s).begin(), ((std::wstring&)s).end(), regex) {}
#else
		Matches(const String &s_, std::wregex &&regex_) : ws(s_.cbegin(), s_.cend()), regex(std::move(regex_)), b(ws.begin(), ws.end(), regex) {}
#endif

		Iterator begin() const {return Iterator(b);}
		Iterator  &end()       {return e;}
		const Iterator &end() const {return e;}

		template <typename Func> auto map(Func &&func) -> Array<decltype(func(Match()))>
		{
			Array<decltype(func(Match()))> r;
			for (auto el : *this)
				r.push_back(func((Match&)el));
			return r;
		}
	};

	Matches find_matches(String &&s) &&
	{
		return Matches(std::move(s), std::move(regex));
	}
};

inline RegEx _(const String &pattern)
{
	return RegEx(pattern);
}
}

inline Array<String> String::split(const re::RegEx &regex) const
{
	Array<String> r;
	int begin = 0;
#ifdef _WIN32
	for (std::wsregex_iterator it(((std::wstring*)this)->begin(), ((std::wstring*)this)->end(), regex.regex), end; it != end; ++it)
#else
	std::wstring ws(cbegin(), cend());
	for (std::wsregex_iterator it(ws.begin(), ws.end(), regex.regex), end; it != end; ++it)
#endif
	{
		r.append(slice(begin, (int)it->position()));
		begin = int(it->position() + it->length());
	}
	r.append(slice(begin, len()));
	return r;
}

String String::replace(const re::RegEx &regex, const String &repl) const
{
#ifdef _WIN32
	return String((std::u16string&&)std::regex_replace((const std::wstring&)*this, regex.regex, (const std::wstring&)repl));
#else
	std::wstring ws(cbegin(), cend()), rws(repl.cbegin(), repl.cend());
	std::wstring r = std::regex_replace(ws, regex.regex, rws);
	return String(r.begin(), r.end());
#endif
}
