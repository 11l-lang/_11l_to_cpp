#include <string>

class String : public std::u16string
{
public:
	String() {}
	String(char16_t c) : basic_string(1, c) {}
	String(const char16_t *s) : basic_string(s) {}
	String(const char16_t *s, size_t sz) : basic_string(s, sz) {}

	int len() const { return size(); } // return `int` (not `size_t`) to avoid warning C4018: '<': signed/unsigned mismatch
};

String operator ""_S(const char16_t *s, size_t sz)
{
	return String(s, sz);
}
