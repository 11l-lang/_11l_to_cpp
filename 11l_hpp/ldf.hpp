namespace ldf // namespace for lightweight data formats (such as JSON and eldf)
{
class Object;
class Array;

union Value // inspired by [https://github.com/nlohmann/json/blob/develop/include/nlohmann/json.hpp]:‘union json_value’
{
	Object *object;
	Array  *array;
	String *string;
	int64_t number_int;
	double number_float;
	bool boolean;
};

enum class ValueType // inspired by [https://github.com/nlohmann/json/blob/develop/include/nlohmann/detail/value_t.hpp]
{
	N, // `NULL` cannot be used because of `#define NULL 0`
	OBJECT,
	ARRAY,
	STRING,
	NUMBER_INT,
	NUMBER_FLOAT,
	BOOLEAN
};

class Element // name `element` was taken from [https://json.org]
{
public:
	ValueType value_type = ValueType::N;
	Value value;

	Element() {}
	Element(Element &&other) : value_type(other.value_type), value(other.value)
	{
		other.value_type = ValueType::N;
	}
	~Element();
};

class Object
{
public:
	DefaultDict<String, Element> members;
};

class Array
{
public:
	::Array<Element> elements;
};

Element::~Element() // moved from class `Element` because of warning C4150: deletion of pointer to incomplete type 'Object'; no destructor called
{
	switch (value_type)
	{
	case ValueType::OBJECT: delete value.object; break;
	case ValueType::ARRAY:  delete value.array;  break;
	case ValueType::STRING: delete value.string; break;
	}
}

// Some common functions
namespace detail {
const char16_t *read_string(const char16_t *s, const char16_t *end, String &str)
{
	assert(s < end && *s == '"');
	s++;
	const char16_t *start = s;
	while (true) {
		assert(s < end);
		if (*s == '"') {
			str.assign(start, s - start);
			return s + 1;
		}
		// [-TODO: `\` support-]
		s++;
	}
}

String string(const String &s)
{
	return u'"'_C & s & u'"'_C; // [-TODO: `\` support-]
}

const char16_t *read_number(const char16_t *s, const char16_t *end, Element &el)
{
	const char16_t *start = s;

	if (*s == '-') {
		s++;
		assert(s < end);
	}
	while (s < end && Char(*s).is_digit())
		s++;

	if (s == end || (*s != '.' && *s != 'e' && *s != 'E')) {
		el.value_type = ValueType::NUMBER_INT;
		el.value.number_int = to_int64(String(start, s - start));
		return s;
	}
	s++;
	while (s < end && (Char(*s).is_digit() || *s == 'e' || *s == 'E')) {
		if ((*s == 'e' || *s == 'E') && s + 1 < end)
			if (s[1] == '-' || s[1] == '+')
				s++;
		s++;
	}
	el.value_type = ValueType::NUMBER_FLOAT;
	el.value.number_float = to_float(String(start, s - start));
	return s;
}
}

#include "ldf/json.hpp"
#include "ldf/serializer.hpp"
}

namespace json {
template <typename Ty> void to_object(const String &json, Ty &obj)
{
	ldf::Element el;
	ldf::from_json(json, el);
	ldf::Serializer ser(&el, true, true);
	ser.serialize(obj);
}
}
