namespace detail {
const char16_t *find_ending_pair_quote(const char16_t *s, const char16_t *end, const char16_t *start)
{
	assert(*s == u'‘'); // ’
	const char16_t *startq = s;
	int nesting_level = 0;
	while (true) {
		if (s == end)
			throw Error(u"unpaired left single quotation mark", startq - start);
		if (*s == u'‘')
			nesting_level++;
		else if (*s == u'’')
			if (--nesting_level == 0)
				return s;
		s++;
	}
}

String substr(const char16_t *s, const char16_t *e)
{
	return String(s, e - s);
}

const char16_t *from_str(const char16_t *s, const char16_t *end, const char16_t *sstart, Element &el, const wchar_t *stop_characters)
{
	assert(el.value_type == ValueType::N);

	const char16_t *start = s;
	if (*s == u'"') {
		el.value.string = new String;
		el.value_type = ValueType::STRING;
		return read_string(s, end, *el.value.string);
	}
	if (*s == u'‘') {
		const char16_t *endq = find_ending_pair_quote(s, end, sstart);
		s = endq + 1;
		while (s < end && *s == u'\'')
			s++;
		size_t eat_right = s - (endq + 1);

		if (substr(endq - eat_right, endq + 1) != u'’'_C * Int(eat_right+1))
			throw Error(u"wrong ending of the raw string", endq - sstart);

		el.value.string = new String(substr(start + 1, endq - eat_right));
		el.value_type = ValueType::STRING;
		return s;
	}
	if (*s == u'\'') {
		s++;
		while (s < end && *s == u'\'')
			s++;
		size_t eat_left = s - start;

		if (String(s, eat_left+1) != u'‘'_C * Int(eat_left+1))
			throw Error(u"wrong beginning of the raw string", start - sstart);

		const char16_t *startq = s,
		               *endq = find_ending_pair_quote(s, end, sstart);
		s = endq + 1;
		while (s < end && *s == u'\'')
			s++;
		size_t eat_right = s - (endq + 1);

		if (substr(endq - eat_right, endq + 1) != u'’'_C * Int(eat_right+1))
			throw Error(u"wrong ending of the raw string", endq - sstart);

		el.value.string = new String(substr(startq + eat_left + 1, endq - eat_right));
		el.value_type = ValueType::STRING;
		return s;
	}
	if (Char(*s).is_digit() || (*s == '-' && Char(s[1]).is_digit())) {
		if (Char(*s).is_digit() && (s[1] == u'B' || s[1] == u'В')) {
			if (!(*s == '0' || *s == '1'))
				throw Error(u"wrong value", s - sstart);
			el.value.boolean = (*s == '1');
			el.value_type = ValueType::BOOLEAN;
			return s + 2;
		}
		return read_number(s, end, el);
	}

	while (s < end) {
		if (wcschr(stop_characters, *s) || *s == '\r' || *s == '\n')
			break;
		s++;
	}
	const char16_t *t = s - 1;
	while (t > start && (*t == ' ' || *t == '\t'))
		t--;
	String ss = substr(start, t + 1);
	if (ss == u"N" || ss == u"Н") {
		el.value_type = ValueType::N; // this is unnecessary
		return s;
	}
	el.value.string = new String(std::move(ss));
	el.value_type = ValueType::STRING;
	return s;
}
}

void from_eldf(const String &eldf, Element &el)
{
	if (eldf.empty())
		throw Error(u"empty ELDF is not allowed", 0);

	assert(el.value_type == ValueType::N);

	const char16_t *end = &(*eldf.end()).code,
	               *s = &(*eldf.begin()).code,
	               *start = s;

	bool expected_an_indented_block = false;
	::Array<int> indentation_levels;
	class ElementRef
	{
	public:
		//ElementRef() : element_ref(&element) {}
		//Element element;
		//Element *element_ref;
		//Element *operator->() {return element_ref;}
		bool is_array;
		ElementRef(bool is_array) : is_array(is_array), is_ref(false)
		{
			//if (is_array) value.array = &array; else value.object = &object;
		}
		ElementRef(Object *object) : is_array(false), is_ref(true)
		{
			value.object = object;
		}
		ElementRef(Array *array) : is_array(true), is_ref(true)
		{
			value.array = array;
		}

		Array  *array () {assert( is_array); return is_ref ? value.array  : &array_ ;}
		Object *object() {assert(!is_array); return is_ref ? value.object : &object_;}

		void move_to_element(Element &el)
		{
			assert(el.value_type == ValueType::N);

			if (is_ref) {
				assert(false);
				el.value = std::move(value);
			}
			else if (is_array) {
				el.value.array = new Array(std::move(array_));
				el.value_type = ValueType::ARRAY;
			}
			else {
				el.value.object = new Object(std::move(object_));
				el.value_type = ValueType::OBJECT;
			}
		}

	private:
		bool is_ref;
		Value value;
		Object object_;
		Array array_;
	};
	std::list<ElementRef> obj_stack; // ::Array<ElementRef> can not be used here

	if (*s == '[') {
		assert(eldf.len() >= 2);
		if (s[1] == ']') {
			assert(eldf.rtrim(make_tuple(u' '_C, u'\t'_C, u'\r'_C, u'\n'_C)).len() == 2);
			el.value.array = new Array;
			el.value_type = ValueType::ARRAY;
			return;
		}
		obj_stack.push_back(ElementRef(true));
		//obj_stack.back()->value.array = new Array;
		//obj_stack.back()->value_type = ValueType::ARRAY;
		expected_an_indented_block = true;
		s++;
	}
	else {
		if (*s == '{' && s[1] == '}') {
			assert(eldf.rtrim(make_tuple(u' '_C, u'\t'_C, u'\r'_C, u'\n'_C)).len() == 2);
			el.value.object = new Object;
			el.value_type = ValueType::OBJECT;
			return;
		}
		obj_stack.push_back(ElementRef(false));
		//obj_stack.back()->value.object = new Object;
		//obj_stack.back()->value_type = ValueType::OBJECT;
	}

	while (s < end) {
		int indentation_level = 0;
		while (s < end) {
			if (*s == ' ')
				indentation_level++;
			else
				break;
			s++;
		}

		if (s == end) // end of source
			break;

		if (*s == '\r' || *s == '\n') { // lines with only whitespace do not affect the indentation
			s++;
			while (s < end && (*s == '\r' || *s == '\n'))
				s++;
			continue;
		}

		if (*s == '.' && s[1] == ' ') {
			indentation_level++;
			s++;
			while (s < end) {
				if (*s == ' ')
					indentation_level++;
				else
					break;
				s++;
			}
			//assert(obj_stack.back()->value_type == ValueType::ARRAY || (obj_stack.back()->value_type == ValueType::OBJECT && (*(std::prev(obj_stack.end(), 2)))->value_type == ValueType::ARRAY));
			//if (obj_stack.back()->value_type == ValueType::OBJECT)
			//	obj_stack.pop_back();
			//obj_stack.back()->value.array->elements.append(Element());
			//Element &new_dict = obj_stack.back()->value.array->elements.last();
			assert(obj_stack.back().is_array || std::prev(obj_stack.end(), 2)->is_array);
			if (!obj_stack.back().is_array)
				obj_stack.pop_back();
			obj_stack.back().array()->elements.append(Element());
			Element &new_dict = obj_stack.back().array()->elements.last();
			new_dict.value.object = new Object;
			new_dict.value_type = ValueType::OBJECT;
			obj_stack.push_back(ElementRef(new_dict.value.object));
		}

		int prev_indentation_level = !indentation_levels.empty() ? indentation_levels.last() : 0;

		if (*s == ';') {
			s++;
			while (s < end && (*s == ' ' || *s == '\t')) // skip spaces after `;`
				s++;
			if (s == end) // end of source
				break;
			indentation_level = prev_indentation_level;
		}

		if (expected_an_indented_block)
			if (!(indentation_level > prev_indentation_level))
				throw Error(u"expected an indented block", s - start);

		if (indentation_level == prev_indentation_level)
			;
		else if (indentation_level > prev_indentation_level) {
			if (!expected_an_indented_block)
				throw Error(u"unexpected indent", s - start);
			expected_an_indented_block = false;
			indentation_levels.append(indentation_level);
		}
		else { // [
			if (*s == ']') {
				assert(obj_stack.back().is_array || std::prev(obj_stack.end(), 2)->is_array);
				s++;
				indentation_levels.pop();
				if (obj_stack.size() > 1)
					if (obj_stack.back().is_array || (obj_stack.size() == 2 && obj_stack.front().is_array))
						obj_stack.pop_back();
					else {
						obj_stack.pop_back();
						obj_stack.pop_back();
					}
				continue;
			}

			while (true) {
				indentation_levels.pop();
				obj_stack.pop_back();
				int level = !indentation_levels.empty() ? indentation_levels.last() : 0;
				if (level == indentation_level)
					break;
				if (level < indentation_level)
					throw Error(u"unindent does not match any outer indentation level", s - start);
			}
		}

		if (*s == '[') { // nested inline list
			assert(obj_stack.back().is_array);
			obj_stack.back().array()->elements.append(Element());
			Element &new_list = obj_stack.back().array()->elements.last();
			new_list.value.array = new Array;
			new_list.value_type = ValueType::ARRAY;
			s++;
			while (true) {
				if (s == end)
					throw Error(u"unexpected end of source", s - start);
				if (*s == ']') {
					s++;
					break; // [
				}
				new_list.value.array->elements.append(Element());
				s = detail::from_str(s, end, start, new_list.value.array->elements.back(), L",]");
				if (s < end)
					if (*s == ',') {
						s++;
						while (s < end && (*s == ' ' || *s == '\t')) // skip spaces after `,`
							s++; // [[
					}
					else if (*s != ']')
						throw Error(u"expected `]`", s - start);
			}
			continue;
		}

		if (*s == '{' && s[1] == '}') {
			s += 2;
			continue;
		}

		const char16_t *key_start = s;
		Element key;
		s = detail::from_str(s, end, start, key, L"={"); // }

		while (s < end && (*s == ' ' || *s == '\t')) // skip spaces after key
			s++;

		if (s < end && *s == '=') {
			if (key.value_type != ValueType::STRING)
				throw Error(u"only string keys are supported so far", key_start - start);

			s++;
			while (s < end && (*s == ' ' || *s == '\t')) // skip spaces after `=`
				s++;
			if (*s == '[' && (s[1] == '\r' || s[1] == '\n')) { // ]
				s += 2;
				expected_an_indented_block = true;
				assert(!obj_stack.back().is_array);
				Element &element = obj_stack.back().object()->members[*key.value.string];
				element.value.array = new Array;
				element.value_type = ValueType::ARRAY;
				obj_stack.push_back(ElementRef(element.value.array));
			}
			else {
				if (obj_stack.back().is_array)
					throw Error(u"key/value pairs are allowed only inside dictionaries not lists", key_start - start);
				Element &element = obj_stack.back().object()->members[*key.value.string];
				if (*s == '[' && s[1] == ']') {
					s += 2;
					element.value.array = new Array;
					element.value_type = ValueType::ARRAY;
				}
				else
					s = detail::from_str(s, end, start, element, L";");
			}
		}
		else {
			if (obj_stack.back().is_array)
				obj_stack.back().array()->elements.append(std::move(key));
			else {
				if (key.value_type != ValueType::STRING)
					throw Error(u"only string keys are supported so far", key_start - start);

				if (s < end && *s == '{') {
					s++;
					if (!(s < end && *s == '}')) // {
						throw Error(u"expected `}`", s - start);
					s++;
					Element &element = obj_stack.back().object()->members[*key.value.string];
					element.value.object = new Object;
					element.value_type = ValueType::OBJECT;
				}
				else {
					expected_an_indented_block = true;
					Element &new_dict = obj_stack.back().object()->members[*key.value.string];
					new_dict.value.object = new Object;
					new_dict.value_type = ValueType::OBJECT;
					obj_stack.push_back(ElementRef(new_dict.value.object));
				}
			}
		}
	}

	if (expected_an_indented_block)
		throw Error(u"expected an indented block", s - start);

	while (!indentation_levels.empty()) {
		indentation_levels.pop();
		obj_stack.pop_back();
	}

	assert(obj_stack.size() == 1);
	obj_stack.back().move_to_element(el);
}

namespace detail {
String balance_pq_string(const String &s)
{
	int min_nesting_level = 0,
	    nesting_level = 0;
	for (Char ch : s)
		if (ch == u'‘')
			nesting_level++;
		else if (ch == u'’') {
			nesting_level--;
			min_nesting_level = min(min_nesting_level, nesting_level);
		}
	nesting_level -= min_nesting_level;
	return u'\''_C*-min_nesting_level & u'‘'_C*-min_nesting_level & u'‘'_C & s & u'’'_C & u'’'_C*nesting_level & u'\''_C*nesting_level;
}

String to_str(const String &vs, char16_t additional_prohibited_character = 0, char16_t additional_prohibited_character2 = 0)
{
	if (vs.len() < 100 && in(u'\n', vs))
		return string(vs);

	if (vs.empty() || in(vs[0], u" \t['") || vs.starts_with(u". ") || in(vs.last(), u" \t") || in(u'‘', vs) || in(u'’', vs) || in(u';', vs) || in(u'\n', vs) || vs == u'N'_C || vs == u'Н'_C // ]
		|| (additional_prohibited_character  != 0 && (in(additional_prohibited_character, vs) ||
		   (additional_prohibited_character2 != 0 && in(additional_prohibited_character2, vs)))) || vs[0].is_digit() || (vs[0] == u'-' && vs.len() > 1 && vs[1].is_digit()))
		return balance_pq_string(vs);

	return vs;
}

String to_str(const Element &el, char16_t additional_prohibited_character = 0, char16_t additional_prohibited_character2 = 0)
{
	if (el.value_type == ValueType::NUMBER_INT)
		return String(el.value.number_int);

	if (el.value_type == ValueType::NUMBER_FLOAT)
		return String(el.value.number_float);

	if (el.value_type == ValueType::BOOLEAN)
		return el.value.boolean ? u"1B" : u"0B";

	if (el.value_type == ValueType::N)
		return u"N";

	assert(el.value_type == ValueType::STRING);
	return to_str(*el.value.string, additional_prohibited_character, additional_prohibited_character2);
}
}

String to_eldf(const Element &el, int indent = 4, int level = 0, bool toplevel = true)
{
	String r;
	if (el.value_type == ValueType::OBJECT) {
		if (el.value.object->members.empty() && toplevel)
			return u"{}\n";
		for (auto it = el.value.object->members.begin(),
		         end = el.value.object->members.end(); it != end;) {
			auto &&[key, element] = *it;
			++it;
			r &= indent * level * u' '_C & detail::to_str(key, u'=', u'{'); // }
			if (element.value_type == ValueType::OBJECT) {
				if (element.value.object->members.empty())
					r &= u" {}\n";
				else {
					r &= u'\n'_C & to_eldf(element, indent, level+1, false);
					if (element.value.object->members.size() > 2 && it != end)
						r &= u'\n'_C;
				}
			}
			else if (element.value_type == ValueType::ARRAY) {
				if (element.value.array->elements.empty())
					r &= u" = []\n";
				else
					r &= u" = [\n" & to_eldf(element, indent, level, false) & indent * level * u' '_C & u"]\n";
			}
			else // this is value
				r &= u" = " & detail::to_str(element) & u'\n'_C;
		}
	}
	else if (el.value_type == ValueType::ARRAY) {
		if (toplevel) {
			if (el.value.array->elements.empty())
				return u"[]\n";
			r &= u"[\n";
		}
		for (auto it = el.value.array->elements.begin(),
		         end = el.value.array->elements.end(); it != end;) {
			const Element &element = *it;
			++it;
			if (element.value_type == ValueType::ARRAY) {
				r &= indent * (level+1) * u' '_C & u'['_C;
				for (auto it = element.value.array->elements.begin(),
				         end = element.value.array->elements.end(); it != end;) {
					const Element &sub_element = *it;
					if (sub_element.value_type == ValueType::OBJECT
					 || sub_element.value_type == ValueType::ARRAY)
						throw Error(u"sorry, but this object can not be represented in ELDF", 0);
					r &= detail::to_str(sub_element, u',');
					++it;
					if (it != end)
						r &= u", ";
				}
				r &= u"]\n";
			}
			else if (element.value_type == ValueType::OBJECT) {
				if (element.value.object->members.empty())
					r &= indent * level * u' '_C & u'.'_C & (indent-1) * u' '_C & u"{}\n";
				else {
					r &= indent * level * u' '_C & u'.'_C & to_eldf(element, indent, level+1, false)[range_ei(indent * level + 1)];
					if (element.value.object->members.size() > 2 && it != end)
						r &= u'\n'_C;
				}
			}
			else
				r &= indent * (level+1) * u' '_C & detail::to_str(element, u'=', u'{') & u'\n'_C; // }
		}
		if (toplevel)
			r &= u"]\n";
	}
	else
		throw Error(u"sorry, but this object can not be represented in ELDF", 0);

	return r;
}
