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
	}

	void serialize(int64_t &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::NUMBER_INT);
			val = element->value.number_int;
		}
	}

	void serialize(int &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::NUMBER_INT);
			assert(element->value.number_int >= std::numeric_limits<int>::min() && element->value.number_int <= std::numeric_limits<int>::max());
			val = int(element->value.number_int);
		}
	}

	void serialize(double &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::NUMBER_FLOAT || element->value_type == ValueType::NUMBER_INT);
			val = element->value_type == ValueType::NUMBER_FLOAT ? element->value.number_float : element->value.number_int;
		}
	}

	void serialize(float &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::NUMBER_FLOAT || element->value_type == ValueType::NUMBER_INT);
			val = element->value_type == ValueType::NUMBER_FLOAT ? float(element->value.number_float) : element->value.number_int;
		}
	}

	void serialize(bool &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::BOOLEAN);
			val = element->value.boolean;
		}
	}

	//template <typename Ty> void serialize(const String &key, Ty &val)
	template <typename Ty> void operator()(const String &key, Ty &val)
	{
		if (from_element_to_object) {
			assert(element->value_type == ValueType::OBJECT);
			auto p = element->value.object->members.find(key);
			assert(p != nullptr);
			ldf::Serializer ser(&*p, true, move_strings);
			ser.serialize(val);
		}
	}
};
