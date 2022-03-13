namespace detail {
const char16_t *skip_whitespace(const char16_t *s, const char16_t *end)
{
	while (s < end && (*s == ' ' || *s == '\n' || *s == '\r' || *s == '\t'))
		s++;
	return s;
}

template <int N> bool read_keyword(const char16_t *&s, const char16_t *end, const char16_t(&keyword)[N])
{
	if (end - s >= N-1 && memcmp(s, keyword, (N-1)*sizeof(char16_t)) == 0) {
		s += N-1;
		return true;
	}
	return false;
}

const char16_t *from_json(const char16_t *s, const char16_t *end, Element &el)
{
	assert(el.value_type == ValueType::N);

	s = skip_whitespace(s, end);
	assert(s < end);

	if (*s == '{') {
		el.value.object = new Object;
		el.value_type = ValueType::OBJECT;
		s++;
		s = skip_whitespace(s, end);
		assert(s < end);
		if (*s != '}')
			while (true) {
				String key;
				s = read_string(s, end, key);
				s = skip_whitespace(s, end);
				assert(s < end && *s == ':');
				s++;
				s = from_json(s, end, el.value.object->members[key]);
				s = skip_whitespace(s, end);
				assert(s < end);
				if (*s == ',') {
					s++;
					s = skip_whitespace(s, end);
				}
				else { // {
					assert(*s == '}');
					break;
				}
			}
		return s + 1;
	}

	if (*s == '[') {
		el.value.array = new Array;
		el.value_type = ValueType::ARRAY;
		s++;
		s = skip_whitespace(s, end);
		assert(s < end);
		if (*s != ']')
			while (true) {
				el.value.array->elements.push_back(Element());
				s = from_json(s, end, el.value.array->elements.back());
				s = skip_whitespace(s, end);
				assert(s < end);
				if (*s == ',') {
					s++;
				}
				else { // [
					assert(*s == ']');
					break;
				}
			}
		return s + 1;
	}

	if (*s == '"') {
		el.value.string = new String;
		el.value_type = ValueType::STRING;
		return read_string(s, end, *el.value.string);
	}

	if (*s == '-' || Char(*s).is_digit()) {
		return read_number(s, end, el);
	}

	if (read_keyword(s, end, u"null")) {
		el.value_type = ValueType::N; // this is unnecessary
		return s;
	}

	el.value.boolean = (*s == 't');
	if (read_keyword(s, end, u"true") || read_keyword(s, end, u"false")) {
		el.value_type = ValueType::BOOLEAN;
		return s;
	}

	assert(false);
	return s; // to suppress warning C4715
}
}

void from_json(const String &json, Element &el)
{
	const char16_t *end = &(*json.end()).code;
	const char16_t *s = detail::from_json(&(*json.begin()).code, end, el);
	s = detail::skip_whitespace(s, end);
	assert(s == end);
}

String to_json(const Element &el, const String &indent, int level = 0)
{
	String r;

	switch (el.value_type)
	{
	case ValueType::OBJECT:
		r = u'{'_C;
		if (!el.value.object->members.empty()) {
			r &= u'\n'_C;
			for (auto it = el.value.object->members.begin(),
			         end = el.value.object->members.end(); it != end;) {
				auto &&[key, element] = *it;
				r &= indent*(level + 1) & detail::string(key) & u": " & to_json(element, indent, level + 1);
				++it;
				if (it != end)
					r &= u','_C;
				r &= u'\n'_C;
			}
			r &= indent*level;
		}
		r &= u'}'_C;
		break;

	case ValueType::ARRAY:
		r = u'['_C;
		if (!el.value.array->elements.empty()) {
			r &= u'\n'_C;
			for (auto it = el.value.array->elements.begin(),
			         end = el.value.array->elements.end(); it != end;) {
				r &= indent*(level + 1) & to_json(*it, indent, level + 1);
				++it;
				if (it != end)
					r &= u','_C;
				r &= u'\n'_C;
			}
			r &= indent*level;
		}
		r &= u']'_C;
		break;

	case ValueType::STRING:
		r = detail::string(*el.value.string);
		break;

	case ValueType::NUMBER_INT:
		r = String(el.value.number_int);
		break;

	case ValueType::NUMBER_FLOAT:
		r = String(el.value.number_float);
		break;

	case ValueType::BOOLEAN:
		r = el.value.boolean ? u"true"_S : u"false"_S;
		break;

	case ValueType::N:
		r = u"null";
		break;

	default:
		assert(false);
	}

	return r;
}

String to_json(const Element &el, Int indent = 4)
{
	return to_json(el, indent * Char(u' '));
}

String to_json(const Element &el, Char indent)
{
	return to_json(el, String(indent));
}
