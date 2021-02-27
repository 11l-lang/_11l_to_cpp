class Serializer
{
public:
	Element *element;
	bool from_element_to_object;
	bool move_strings;

	Serializer(Element *element, bool from_element_to_object, bool move_strings) : element(element), from_element_to_object(from_element_to_object), move_strings(move_strings) {}

	template <typename Ty> void serialize(Ty &obj)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::OBJECT);
			obj.serialize(*this);
		}
		else {
			element->value_type = ValueType::OBJECT;
			element->value.object = new Object;
			obj.serialize(*this);
		}
	}

	template <typename KeyType, typename ValueTy> void serialize(DefaultDict<KeyType, ValueTy> &dict)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::OBJECT);
			assert(dict.empty());
			//dict.reserve(element->value.object->members.size());
			for (auto &[k, el] : element->value.object->members) {
				ValueTy &dv = dict[k];
				ldf::Serializer ser(&el, true, move_strings);
				ser.serialize(dv);
			}
		}
		else {
			element->value_type = ValueType::OBJECT;
			element->value.object = new Object;
			//element->value.object->members.reserve(dict.size());
			for (auto &[k, v] : dict) {
				Element &el = element->value.object->members[k];
				ldf::Serializer ser(&el, false, move_strings);
				ser.serialize(v);
			}
		}
	}
	template <typename KeyType, typename ValueTy> void serialize(Dict<KeyType, ValueTy> &dict)
	{
		serialize(static_cast<DefaultDict<KeyType, ValueTy>&>(dict));
	}

	template <typename Ty> void serialize(::Array<Ty> &arr)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::ARRAY);
			assert(arr.empty());
			arr.reserve(element->value.array->elements.size());
			for (auto &el : element->value.array->elements) {
				arr.push_back(Ty());
				ldf::Serializer ser(&el, true, move_strings);
				ser.serialize(arr.back());
			}
		}
		else {
			element->value_type = ValueType::ARRAY;
			element->value.array = new Array;
			element->value.array->elements.reserve(arr.size());
			for (auto &el : arr) {
				element->value.array->elements.push_back(Element());
				ldf::Serializer ser(&element->value.array->elements.back(), false, move_strings);
				ser.serialize(el);
			}
		}
	}

	void serialize(String &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::STRING);
			if (move_strings)
				val = std::move(*element->value.string);
			else
				val = *element->value.string;
		}
		else {
			element->value_type = ValueType::STRING;
			if (move_strings)
				element->value.string = new String(std::move(val));
			else
				element->value.string = new String(val);
		}
	}

	void serialize(Int64 &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::NUMBER_INT);
			val = element->value.number_int;
		}
		else {
			element->value_type = ValueType::NUMBER_INT;
			element->value.number_int = val;
		}
	}

	void serialize(int &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::NUMBER_INT);
			assert(element->value.number_int >= std::numeric_limits<int>::min() && element->value.number_int <= std::numeric_limits<int>::max());
			val = int(element->value.number_int);
		}
		else {
			element->value_type = ValueType::NUMBER_INT;
			element->value.number_int = val;
		}
	}

	void serialize(double &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::NUMBER_FLOAT || element->value_type == ValueType::NUMBER_INT);
			val = element->value_type == ValueType::NUMBER_FLOAT ? element->value.number_float : element->value.number_int;
		}
		else {
			element->value_type = ValueType::NUMBER_FLOAT;
			element->value.number_float = val;
		}
	}

	void serialize(float &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::NUMBER_FLOAT || element->value_type == ValueType::NUMBER_INT);
			val = element->value_type == ValueType::NUMBER_FLOAT ? float(element->value.number_float) : element->value.number_int;
		}
		else {
			element->value_type = ValueType::NUMBER_FLOAT;
			element->value.number_float = val;
		}
	}

	void serialize(bool &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::BOOLEAN);
			val = element->value.boolean;
		}
		else {
			element->value_type = ValueType::BOOLEAN;
			element->value.boolean = val;
		}
	}

	//template <typename Ty> void serialize(const String &key, Ty &val)
	template <typename Ty> void operator()(const String &key, Ty &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::OBJECT);
			auto p = element->value.object->members.find(key);
			//assert(p != nullptr);
			//ldf::Serializer ser(&*p, true, move_strings);
			assert(p != element->value.object->members.end());
			ldf::Serializer ser(const_cast<Element*>(&p->second), true, move_strings);
			ser.serialize(val);
		}
		else {
			assert(element->value_type == ValueType::OBJECT);
			ldf::Serializer ser(&element->value.object->members[key], false, move_strings);
			ser.serialize(val);
		}
	}
};
