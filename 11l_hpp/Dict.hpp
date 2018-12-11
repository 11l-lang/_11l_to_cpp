#include <map>

class KeyError
{
public:
	String key;

	KeyError(Char c) : key(c) {}
	KeyError(const String &key) : key(key) {}
};

// Mimic boost::assign::map_list_of behavior
template <typename KeyType, typename ValueType> class DictInitializer
{
	std::map<KeyType, ValueType> dict;
	template <typename /*KeyType*/, typename /*ValueType*/> friend class Dict; // fix of error in GCC: declaration of template parameter ‘KeyType’ shadows template parameter

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
	Dict(DictInitializer<KeyType, ValueType> &&di) : std::map<KeyType, ValueType>(std::forward<std::map<KeyType, ValueType>>(di.dict)) {}

	ValueType &operator[](const KeyType &key)
	{
		auto r = std::map<KeyType, ValueType>::insert(std::make_pair(key, ValueType()));
		if (r.second) throw KeyError(String(key));
		return r.first->second;
	}

	const ValueType &operator[](const KeyType &key) const
	{
		auto r = find(key);
		if (r == std::map<KeyType, ValueType>::end()) throw KeyError(String(key));
		return r->second;
	}

	void set(const KeyType &key, const ValueType &value)
	{
		//insert(std::make_pair(key, value)); //`insert()` does not assign value if it already exists in the map
		std::map<KeyType, ValueType>::operator[](key) = value;
	}
};

template <typename KeyType, typename ValueType> Dict<KeyType, ValueType> create_dict(DictInitializer<KeyType, ValueType> &di)
{
	return Dict<KeyType, ValueType>(std::move(di));
}

template <typename KeyType, typename ValueType> Dict<KeyType, ValueType> create_dict(const Array<Tuple<KeyType, ValueType>> &arr)
{
	Dict<KeyType, ValueType> r;
	for (auto &&el : arr)
		r.set(_get<0>(el), _get<1>(el));
	return r;
}

template <typename KeyType, typename ValueType> inline bool in(const KeyType &key, const Dict<KeyType, ValueType> &dict)
{
	return dict.find(key) != dict.end();
}

template <typename KeyType, typename ValueType> class DefaultDict : public std::map<KeyType, ValueType>
{
public:
	DefaultDict() {}

	void set(const KeyType &key, const ValueType &value)
	{
		std::map<KeyType, ValueType>::operator[](key) = value;
	}

	Array<ValueType> values() const
	{
		Array<ValueType> r;
		for (auto &&kv : *this)
			r.push_back(kv.second);
		return r;
	}
};

template <typename KeyType, typename ValueType> inline bool in(const KeyType &key, const DefaultDict<KeyType, ValueType> &dict)
{
	return dict.find(key) != dict.end();
}
