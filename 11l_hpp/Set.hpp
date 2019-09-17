#include <set>

template <typename KeyType> class Set : public std::set<KeyType>
{
public:
	Set() {}

	int len() const {return (int)size();}

	bool add(const KeyType &k)
	{
		auto r = insert(k);
		return r.second;
	}

	Set difference(const Set &other) const
	{
		Set r;
		for (auto &&el : *this)
			if (!in(el, other))
				r.add(el);
		return r;
	}

	template <typename Func> auto map(Func &&func) const
	{
		Array<decltype(func(std::declval<KeyType>()))> r;
		r.reserve(len());
		for (auto &&el : *this)
			r.push_back(func(el));
		return r;
	}
};

template <typename Type> Set<Type> create_set(const Array<Type> &arr)
{
	Set<Type> r;
	for (auto &&el : arr)
		r.add(el);
	return r;
}

template <typename Type> Array<Type> create_array(const Set<Type> &set)
{
	Array<Type> r;
	for (auto &&el : set)
		r.append(el);
	return r;
}

template <typename KeyType> inline bool in(const KeyType &key, const Set<KeyType> &set)
{
	return set.find(key) != set.end();
}
