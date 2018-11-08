#include <regex>

namespace re
{
class RegEx
{
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
