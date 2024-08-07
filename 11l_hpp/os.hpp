namespace os
{
static const Char env_path_sep =
#ifdef _WIN32
u';'
#else
u':'
#endif
;

static const String name =
#ifdef _WIN32
u"nt"
#else
u"posix"
#endif
;

inline String getenv(const String &name, const String &def = String())
{
#ifdef _WIN32
	size_t return_value;
	_wgetenv_s(&return_value, nullptr, 0, (wchar_t*)name.c_str());
	if (return_value == 0)
		return def;
	String r;
	r.resize(return_value);
	_wgetenv_s(&return_value, (wchar_t*)r.data(), return_value, (wchar_t*)name.c_str());
	r.resize(return_value - 1);
	return r;
#else
	char *v = ::getenv(convert_utf16_to_utf8(name).c_str());
	return v != nullptr ? String(convert_utf8_to_utf16(v, strlen(v))) : def;
#endif
}

inline void setenv(const String &name, const String &value)
{
#ifdef _WIN32
	_wputenv_s((wchar_t*)name.c_str(), (wchar_t*)value.c_str());
#else
	::setenv(convert_utf16_to_utf8(name).c_str(), convert_utf16_to_utf8(value).c_str(), 1);
#endif
}

static class Environ
{
public:
	class Var
	{
		String name;
	public:
		Var(const String &name) : name(name) {}
		void operator&=(const String &s)
		{
			setenv(name, getenv(name) & s);
		}
		operator String() const
		{
			String r = getenv(name, u"\0"_S);
			if (r == u'\0')
				throw KeyError(name);
			return r;
		}
	};

	Var operator[](const String &name) const
	{
		return Var(name);
	}

	void set(const String &name, const String &value)
	{
		setenv(name, value);
	}
#undef environ
} environ;

inline int _(const String &s)
{
#ifdef _WIN32
	return _wsystem((wchar_t*)s.c_str());
#else
	return system(convert_utf16_to_utf8(s).c_str());
#endif
}

inline int _(const Array<String> &args)
{
	//return _(args.map([](const String &a){return u'"'_C & a & u'"'_C;}).join(u' '_C));
	String a0 = args[0];
    if (in(u' '_C, a0))
        a0 = u'"'_C & a0 & u'"'_C;
    return _((a0 + args[range_ei(1)].map([](const String &a){return u'"'_C & a & u'"'_C;})).join(u' '_C));}
}

namespace _9
{
template <typename Ty> inline int _(const Ty &cmd) {return os::_(cmd);}
}
namespace _9os = _9;
