namespace ldf // namespace for lightweight data formats (such as JSON and ELDF)
{
class Object;
class Array;

union Value // inspired by [https://github.com/nlohmann/json/blob/develop/include/nlohmann/json.hpp]:‘union json_value’
{
	Object *object;
	Array  *array;
	String *string;
	Int64 number_int;
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

class Element // name `element` was taken from [https://www.json.org/json-en.html <- http://json.org]
{
public:
	ValueType value_type = ValueType::N;
	Value value;

	Element() {}
	Element(Element &&other) : value_type(other.value_type), value(other.value)
	{
		other.value_type = ValueType::N;
	}
	void operator=(Element &&other)
	{
		assert(value_type == ValueType::N);
		value_type = other.value_type;
		value = other.value;
		other.value_type = ValueType::N;
	}
	~Element();
};

class Object
{
public:
	//DefaultDict<String, Element> members;
	tsl::ordered_map<String, Element> members;
};

class Array
{
public:
	::Array<Element> elements;
};

Element::~Element() // moved from class `Element` because of warning C4150: deletion of pointer to incomplete type 'ldf::Object'; no destructor called
{
	switch (value_type)
	{
	case ValueType::OBJECT: delete value.object; break;
	case ValueType::ARRAY:  delete value.array;  break;
	case ValueType::STRING: delete value.string; break;
	}
}

class Error
{
public:
	String message;
	int pos;

	Error(const String &message, size_t pos) : message(message), pos(int(pos)) {}
};

// Some common functions
namespace detail {
const char16_t *read_string(const char16_t *s, const char16_t *end, String &str)
{
	assert(s < end && *s == '"');
	assert(str.empty());
	s++;
	while (true) {
		assert(s < end);
		if (*s == '"')
			return s + 1;

		if (*s == '\\') {
			s++;
			assert(s < end);
			switch (*s)
			{
			case '"' : str &= u'"'_C;  break;
			case '\\': str &= u'\\'_C; break;
			case '/' : str &= u'/'_C;  break;
			case 'b' : str &= u'\b'_C; break;
			case 'f' : str &= u'\f'_C; break;
			case 'n' : str &= u'\n'_C; break;
			case 'r' : str &= u'\r'_C; break;
			case 't' : str &= u'\t'_C; break;
			case 'u':
				s++;
				assert(s + 4 <= end);
				str &= Char((char16_t)to_int(String(s, 4), 16));
				s += 4;
				continue;
			default: assert(false);
			}
		}
		else
			str &= Char(*s);

		s++;
	}
}

String string(const String &s)
{
	String r;
	r.reserve(2 + s.length());
	r = u'"'_C;
	for (Char c : s)
		switch (c)
		{
		case u'"' : r &= u"\\\""; break;
		case u'\\': r &= u"\\\\"; break;
		case u'\b': r &= u"\\b";  break;
		case u'\f': r &= u"\\f";  break;
		case u'\n': r &= u"\\n";  break;
		case u'\r': r &= u"\\r";  break;
		case u'\t': r &= u"\\t";  break;
		default: r &= c;
		}
	r &= u'"'_C;
	return r;
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
	//s++; // this line was commented to fix `1e-21`
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
#include "ldf/eldf.hpp"
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

template <typename Ty, typename IndentType = int> String from_object(const Ty &obj, IndentType indent = 4)
{
	ldf::Element el;
	ldf::Serializer ser(&el, false, false);
	ser.serialize(const_cast<Ty&>(obj));
	return ldf::to_json(el, indent);
}

template <typename Ty, typename IndentType = int> String from_object(Ty &obj, IndentType indent = 4)
{
	ldf::Element el;
	ldf::Serializer ser(&el, false, false);
	ser.serialize(obj);
	return ldf::to_json(el, indent);
}

template <typename Ty, typename IndentType = int> String from_object(Ty &&obj, IndentType indent = 4) // for `from_object(std::move(obj))`
{
	ldf::Element el;
	ldf::Serializer ser(&el, false, true);
	ser.serialize(obj);
	return ldf::to_json(el, indent);
}
}

namespace eldf {
using Error = ldf::Error;

template <typename Ty> void to_object(const String &eldf, Ty &obj)
{
	ldf::Element el;
	ldf::from_eldf(eldf, el);
	ldf::Serializer ser(&el, true, true);
	ser.serialize(obj);
}

template <typename Ty> String from_object(const Ty &obj, int indent = 4)
{
	ldf::Element el;
	ldf::Serializer ser(&el, false, false);
	ser.serialize(const_cast<Ty&>(obj));
	return ldf::to_eldf(el, indent);
}

template <typename Ty> String from_object(Ty &obj, int indent = 4)
{
	ldf::Element el;
	ldf::Serializer ser(&el, false, false);
	ser.serialize(obj);
	return ldf::to_eldf(el, indent);
}

template <typename Ty> String from_object(Ty &&obj, int indent = 4) // for `from_object(std::move(obj))`
{
	ldf::Element el;
	ldf::Serializer ser(&el, false, true);
	ser.serialize(obj);
	return ldf::to_eldf(el, indent);
}

String from_json(const String &json_str)
{
	ldf::Element el;
	ldf::from_json(json_str, el);
	return ldf::to_eldf(el);
}

String to_json(const String &eldf_str)
{
	ldf::Element el;
	ldf::from_eldf(eldf_str, el);
	return ldf::to_json(el);
}

String reparse(const String &eldf_str)
{
	ldf::Element el;
	ldf::from_eldf(eldf_str, el);
	return ldf::to_eldf(el);
}

void test_parse(const String &eldf_str)
{
	ldf::Element el;
	ldf::from_eldf(eldf_str, el);
}
}
