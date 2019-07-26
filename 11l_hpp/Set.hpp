#include <set>

template <typename KeyType> class Set : public std::set<KeyType>
{
public:
	Set() {}

	bool add(const KeyType &k)
	{
		auto r = insert(k);
		return r.second;
	}
};

template <typename KeyType> inline bool in(const KeyType &key, const Set<KeyType> &set)
{
	return set.find(key) != set.end();
}
