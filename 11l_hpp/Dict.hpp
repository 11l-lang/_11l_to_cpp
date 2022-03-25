#include <map>

class KeyError
{
public:
	String key;

	KeyError(Char c) : key(c) {}
	KeyError(const String &key) : key(key) {}
	template <typename KeyTy> KeyError(const KeyTy &key) : key(key) {}
};

// Mimic boost::assign::map_list_of behavior
template <typename KeyType, typename ValueType> class DictInitializer
{
	std::map<KeyType, ValueType> dict;
	template <typename /*KeyType*/, typename /*ValueType*/> friend class DefaultDict; // fix of error in GCC: declaration of template parameter ‘KeyType’ shadows template parameter

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
template <typename KeyType, typename ValueType> auto dict_of(KeyType &&key, ValueType &&value)
{
	return DictInitializer<std::remove_reference_t<KeyType>, std::remove_reference_t<ValueType>>(std::forward<KeyType>(key), std::forward<ValueType>(value));
}

template <typename KeyType, typename ValueType> class DefaultDict : public std::map<KeyType, ValueType>
{
	using std::map<KeyType, ValueType>::size;

	class ConstValuePtr
	{
		const ValueType *value;

	public:
		ConstValuePtr(const ValueType *value) : value(value) {}

		bool operator==(nullptr_t) const {return value == nullptr;}
		bool operator!=(nullptr_t) const {return value != nullptr;}

		const ValueType &operator*() const {if (value == nullptr) throw NullPointerException(); return *value;}
		const ValueType &operator*()       {if (value == nullptr) throw NullPointerException(); return *value;}
		const ValueType *operator->() const {if (value == nullptr) throw NullPointerException(); return value;}
		const ValueType *operator->()       {if (value == nullptr) throw NullPointerException(); return value;}
	};
	class ValuePtr
	{
		ValueType *value;

	public:
		ValuePtr(ValueType *value) : value(value) {}

		bool operator==(nullptr_t) const {return value == nullptr;}
		bool operator!=(nullptr_t) const {return value != nullptr;}

		const ValueType &operator*() const {if (value == nullptr) throw NullPointerException(); return *value;}
		      ValueType &operator*()       {if (value == nullptr) throw NullPointerException(); return *value;}
		const ValueType *operator->() const {if (value == nullptr) throw NullPointerException(); return value;}
		      ValueType *operator->()       {if (value == nullptr) throw NullPointerException(); return value;}
	};

public:
	DefaultDict() {}
	DefaultDict(DictInitializer<KeyType, ValueType> &&di) : std::map<KeyType, ValueType>(std::forward<std::map<KeyType, ValueType>>(di.dict)) {}

	Int len() const {return (Int)size();}

	const ValueType &set(const KeyType &key, const ValueType &value)
	{
		//insert(std::make_pair(key, value)); //`insert()` does not assign value if it already exists in the map
		//std::map<KeyType, ValueType>::operator[](key) = value; // this does not work with ValueType = Char (as `Char` has no default constructor)
		std::map<KeyType, ValueType>::insert_or_assign(key, value);
		return value;
	}

	const ValueType &set(const KeyType &key, ValueType &&value)
	{
		//std::map<KeyType, ValueType>::operator[](key) = std::forward<ValueType>(value);
		std::map<KeyType, ValueType>::insert_or_assign(key, std::forward<ValueType>(value));
		return value;
	}

	void set_default(const KeyType &key, const ValueType &value)
	{
		insert(std::make_pair(key, value));
	}

	void update(const DefaultDict &other)
	{
		for (auto &&el : other)
			set(el.first, el.second);
	}

	ConstValuePtr find(const KeyType &key) const
	{
		auto r = std::map<KeyType, ValueType>::find(key);
		if (r == std::map<KeyType, ValueType>::end()) return ConstValuePtr(nullptr);
		return ConstValuePtr(&r->second);
	}

	ValuePtr find(const KeyType &key)
	{
		auto r = std::map<KeyType, ValueType>::find(key);
		if (r == std::map<KeyType, ValueType>::end()) return ValuePtr(nullptr);
		return ValuePtr(&r->second);
	}

	const ValueType &get(const KeyType &key, const ValueType &default_value) const
	{
		auto r = std::map<KeyType, ValueType>::find(key);
		if (r == std::map<KeyType, ValueType>::end()) return default_value;
		return r->second;
	}

	const ValueType &operator[](const KeyType &key) const
	{
		auto r = std::map<KeyType, ValueType>::find(key);
		if (r == std::map<KeyType, ValueType>::end()) throw KeyError(String(key));
		return r->second;
	}
	ValueType &operator[](const KeyType &key)
	{
		return std::map<KeyType, ValueType>::operator[](key);
	}

	ValueType pop(const KeyType &key)
	{
		auto r = std::map<KeyType, ValueType>::find(key);
		if (r == std::map<KeyType, ValueType>::end()) throw KeyError(String(key));
		ValueType el = std::move(r->second);
		std::map<KeyType, ValueType>::erase(r);
		return std::move(el);
	}

	Array<KeyType> keys() const
	{
		Array<KeyType> r;
		r.reserve(size());
		for (auto &&kv : *this)
			r.push_back(kv.first);
		return r;
	}

	Array<ValueType> values() const
	{
		Array<ValueType> r;
		r.reserve(size());
		for (auto &&kv : *this)
			r.push_back(copy(kv.second));
		return r;//std::move(r);
	}

	auto items() const
	{
		Array<decltype(make_tuple(std::declval<KeyType>(), std::declval<ValueType>()))> r;
		r.reserve(size());
		for (auto &&el : *this)
			r.push_back(make_tuple(el.first, el.second));
		return r;
	}

	template <typename Func> auto map(Func &&func) const
	{
		Array<decltype(func(std::declval<KeyType>(), std::declval<ValueType>()))> r;
		r.reserve(len());
		for (auto &&el : *this)
			r.push_back(func(el.first, el.second));
		return r;
	}

	template <typename Func, typename = decltype(std::declval<Func>()(std::declval<decltype(make_tuple(std::declval<KeyType>(), std::declval<ValueType>()))>()))> auto filter(Func &&func) const
	{
		Array<decltype(make_tuple(std::declval<KeyType>(), std::declval<ValueType>()))> r;
		for (auto &&el : *this)
			if (func(make_tuple(el.first, el.second)))
				r.push_back(make_tuple(el.first, el.second));
		return r;
	}

	template <typename Func, typename Dummy = int, typename = decltype(std::declval<Func>()(std::declval<KeyType>(),
	                                                                                        std::declval<ValueType>()))> auto filter(Func &&func) const
	{
		Array<decltype(make_tuple(std::declval<KeyType>(), std::declval<ValueType>()))> r;
		for (auto &&el : *this)
			if (func(el.first, el.second))
				r.push_back(make_tuple(el.first, el.second));
		return r;
	}
};

template <typename KeyType, typename ValueType> class Dict : public DefaultDict<KeyType, ValueType>
{
public:
	Dict() {}
	Dict(DictInitializer<KeyType, ValueType> &&di) : DefaultDict<KeyType, ValueType>(std::forward<DictInitializer<KeyType, ValueType>>(di)) {}
	Dict(DefaultDict<KeyType, ValueType> &&d) : DefaultDict<KeyType, ValueType>(std::forward<DefaultDict<KeyType, ValueType>>(d)) {}

	ValueType &operator[](const KeyType &key)
	{
		auto r = std::map<KeyType, ValueType>::find(key);
		if (r == std::map<KeyType, ValueType>::end()) throw KeyError(String(key));
		return r->second;
	}

	const ValueType &operator[](const KeyType &key) const
	{
		auto r = std::map<KeyType, ValueType>::find(key);
		if (r == std::map<KeyType, ValueType>::end()) throw KeyError(String(key));
		return r->second;
	}

	Dict copy() const
	{
		Dict d = *this;
		return d;
	}
};

template <typename KeyType, typename ValueType> Dict<KeyType, ValueType> create_dict(DictInitializer<KeyType, ValueType> &di)
{
	return Dict<KeyType, ValueType>(std::move(di));
}

template <typename KeyType, typename ValueType> Dict<KeyType, ValueType> create_dict(DictInitializer<KeyType, ValueType> &&di)
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

template <typename Iterable, typename Func,
    typename = decltype(std::declval<Func>()(std::declval<decltype(*std::begin(std::declval<Iterable>()))>()))> auto create_dict(const Iterable &iterable, Func &&func)
{
	using Type = decltype(func(std::declval<decltype(*std::begin(iterable))>()));
	Dict<std::tuple_element_t<0, Type>, std::tuple_element_t<1, Type>> r;
	for (auto &&el : iterable) {
		auto &&e = func(el);
		r.set(_get<0>(e), _get<1>(e));
	}
	return r;
}

template <typename Iterable, typename Func, typename Dummy = int, typename = decltype(
    std::declval<Func>()(std::declval<decltype(tuple_element_f<std::remove_const_t<std::remove_reference_t<decltype(*std::begin(std::declval<Iterable>()))>>, 0>())>(),
                         std::declval<decltype(tuple_element_f<std::remove_const_t<std::remove_reference_t<decltype(*std::begin(std::declval<Iterable>()))>>, 1>())>()))> auto create_dict(const Iterable &iterable, Func &&func)
{
	using Ty = std::remove_const_t<std::remove_reference_t<decltype(*std::begin(iterable))>>;
	using Type = decltype(func(std::declval<std::tuple_element_t<0, Ty>>(), std::declval<std::tuple_element_t<1, Ty>>()));
	Dict<std::tuple_element_t<0, Type>, std::tuple_element_t<1, Type>> r;
	for (auto &&el : iterable) {
		auto &&e = func(_get<0>(el), _get<1>(el));
		r.set(_get<0>(e), _get<1>(e));
	}
	return r;
}

template <typename KeyType, typename ValueType> Dict<KeyType, ValueType> create_dict(DefaultDict<KeyType, ValueType> &&d)
{
	return Dict<KeyType, ValueType>(std::forward<DefaultDict<KeyType, ValueType>>(d));
}

template <typename KeyType, typename ValueType> inline bool in(const KeyType &key, const DefaultDict<KeyType, ValueType> &dict)
{
	return dict.std::map<KeyType, ValueType>::find(key) != dict.end();
}

template <typename KeyType> inline DefaultDict<KeyType, Int> Counter(const Array<KeyType> &arr)
{
	DefaultDict<KeyType, Int> d;
	for (auto &&el : arr)
		d[el]++;
	return d;
}

inline DefaultDict<Char, Int> Counter(const String &str)
{
	DefaultDict<Char, Int> d;
	for (Char c : str)
		d[c]++;
	return d;
}
