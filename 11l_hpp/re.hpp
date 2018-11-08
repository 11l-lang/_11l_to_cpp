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
