#include <map>

class KeyError
{
public:
	String key;

	KeyError(const String &key) : key(key) {}
};

// Mimic boost::assign::map_list_of behavior
template <typename KeyType, typename ValueType> class DictInitializer
{
	std::map<KeyType, ValueType> dict;
	template <typename KeyType, typename ValueType> friend class Dict;

public:
	DictInitializer(const KeyType &key, const ValueType &value)
	{
		dict.insert(std::make_pair(key, value));
	}
	DictInitializer(KeyType &&key, ValueType &&value)
	{
		dict.insert(std::make_pair(std::forward<KeyType>(key), std::forward<ValueType>(value)));
	}

	DictInitializer &operator()(const KeyType &key, const ValueType &value)
	{
		dict.insert(std::make_pair(key, value));
		return *this;
	}
	DictInitializer &operator()(KeyType &&key, ValueType &&value)
	{
		dict.insert(std::make_pair(std::forward<KeyType>(key), std::forward<ValueType>(value)));
		return *this;
	}
};

template <typename KeyType, typename ValueType> DictInitializer<KeyType, ValueType> dict_of(const KeyType &key, const ValueType &value)
{
	return DictInitializer<KeyType, ValueType>(key, value);
}
template <typename KeyType, typename ValueType> DictInitializer<KeyType, ValueType> dict_of(KeyType &&key, ValueType &&value)
{
	return DictInitializer<KeyType, ValueType>(std::forward<KeyType>(key), std::forward<ValueType>(value));
}

template <typename KeyType, typename ValueType> class Dict : public std::map<KeyType, ValueType>
{
public:
	Dict() {}
	Dict(DictInitializer<KeyType, ValueType> &&di) : map(std::forward<std::map<KeyType, ValueType>>(di.dict)) {}

	ValueType &operator[](const KeyType &key)
	{
		auto r = insert(std::make_pair(key, ValueType()));
		if (r.second) throw KeyError(String(key));
		return r.first->second;
	}

	const ValueType &operator[](const KeyType &key) const
	{
		auto r = find(key);
		if (r == end()) throw KeyError(String(key));
		return r->second;
	}

	void set(const KeyType &key, const ValueType &value)
	{
		//insert(std::make_pair(key, value)); //`insert()` does not assign value if it already exists in the map
		map::operator[](key) = value;
	}
};

template <typename KeyType, typename ValueType> Dict<KeyType, ValueType> create_dict(DictInitializer<KeyType, ValueType> &di)
{
	return Dict<KeyType, ValueType>(std::move(di));
}
