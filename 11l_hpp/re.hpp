#include <regex>

namespace re
{
class RegEx
{
	friend class String;
	std::wregex regex;
public:
	RegEx(const String &pattern) : regex((std::wstring&)pattern) {}

	class Match
	{
		friend class RegEx;
		std::wsmatch m;
	public:
		explicit operator bool() const {return !m.empty();}
		int start(int group = 0) const {return (int)m.position(group);}
		int end  (int group = 0) const {return int(m.position(group) + m.length(group));}
		String group(int group_= 0) const {return String((std::u16string&&)m.str(group_));}
	};

	Match search(const String &s) const
	{
		Match r;
		std::regex_search((std::wstring&)s, r.m, regex);
		return r;
	}

	Match match(const String &s) const
	{
		Match r;
		std::regex_match((std::wstring&)s, r.m, regex);
		return r;
	}

	Array<String> find_strings(const String &s) const
	{
		Array<String> r;
		for (std::wsregex_iterator it(((std::wstring&)s).begin(), ((std::wstring&)s).end(), regex), end; it != end; ++it) {
			int group = it->size() == 2 ? 1 : 0;
			r.append(s[range_el((int)it->position(group), int(it->position(group) + it->length(group)))]);
		}
		return r;
	}

	class Matches
	{
		String s;
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
		Matches(String &&s_, std::wregex &&regex_) : s(std::move(s_)), regex(std::move(regex_)), b(((std::wstring&)s).begin(), ((std::wstring&)s).end(), regex) {}

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
	for (std::wsregex_iterator it(((std::wstring*)this)->begin(), ((std::wstring*)this)->end(), regex.regex), end; it != end; ++it) {
		r.append(slice(begin, (int)it->position()));
		begin = int(it->position() + it->length());
	}
	r.append(slice(begin, len()));
	return r;
}

String String::replace(const re::RegEx &regex, const String &repl) const
{
	return String((std::u16string&&)std::regex_replace((const std::wstring&)*this, regex.regex, (const std::wstring&)repl));
}
