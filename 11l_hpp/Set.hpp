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

	template <typename Func> auto map(Func &&func) const
	{
		Array<decltype(func(std::declval<KeyType>()))> r;
		r.reserve(len());
		for (auto &&el : *this)
			r.push_back(func(el));
		return r;
	}
};

template <typename KeyType> inline bool in(const KeyType &key, const Set<KeyType> &set)
{
	return set.find(key) != set.end();
}
