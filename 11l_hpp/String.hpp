#include <string>

class String : public std::u16string
{
public:
	String() {}
	String(char16_t c) : basic_string(1, c) {}
	String(const char16_t *s) : basic_string(s) {}
	String(const char16_t *s, size_t sz) : basic_string(s, sz) {}

	int len() const { return size(); } // return `int` (not `size_t`) to avoid warning C4018: '<': signed/unsigned mismatch

	friend String &&operator*(String &&s, int n)
	{
		int s_len = s.length();
		if (n < 1) // mimic Python's behavior in which 's' * 0 = '' and 's' * -1 = ''
			s.clear();
		else {
			s.reserve(s_len * n);
			for (int i = 1; i < n; i++)
				s.append(s.c_str(), s_len);
		}
		return std::move(s);
	}
	friend String &&operator*(int n, String &&s)
	{
		return std::move(s) * n;
	}
};

String operator ""_S(const char16_t *s, size_t sz)
{
	return String(s, sz);
}
