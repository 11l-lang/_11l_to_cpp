namespace os
{
static const Char env_path_sep =
#ifdef _WIN32
u';'
#else
u':'
#endif
;

String getenv(const String &name, const String &def = String())
{
	size_t return_value;
	_wgetenv_s(&return_value, nullptr, 0, (wchar_t*)name.c_str());
	if (return_value == 0)
		return def;
	String r;
	r.resize(return_value);
	_wgetenv_s(&return_value, (wchar_t*)r.data(), return_value, (wchar_t*)name.c_str());
	r.resize(return_value - 1);
	return r;
}

void setenv(const String &name, const String &value)
{
	_wputenv_s((wchar_t*)name.c_str(), (wchar_t*)value.c_str());
}

class Environ
{
public:
	class Var
	{
		String name;
	public:
		Var(const String &name) : name(name) {}
		void operator+=(const String &s)
		{
			setenv(name, getenv(name) + s);
		}
	};

	Var operator[](const String &name) const
	{
		return Var(name);
	}
#undef environ
} environ;

int _(const String &s)
{
	return _wsystem((wchar_t*)s.c_str());
}
}
