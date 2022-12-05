#include <set>

template <typename KeyType> class Set : public std::set<KeyType>
{
#ifdef PUBLIC_SET_COPY_CONSTRUCTOR
public:
	const Set &operator=(const Set &s) { std::set<KeyType>::operator=(s); return *this; }
#endif
	Set(const Set &s) : std::set<KeyType>(s) {}

public:
	friend Set<KeyType> copy(const Set<KeyType> &s) {return s;}

	Set() {}
	Set(Set &&s) : std::set<KeyType>(std::forward<std::set<KeyType>>(s)) {}
	Set(std::initializer_list<KeyType> il)
	{
//		reserve(il.size());
		for (auto &&el : il)
			add(el);
	}
	void operator=(Set &&s)
	{
		std::set<KeyType>::operator=(std::forward<std::set<KeyType>>(s));
	}

	Int len() const {return (Int)std::set<KeyType>::size();}

	bool add(const KeyType &k)
	{
		auto r = std::set<KeyType>::insert(k);
		return r.second;
	}

	template <typename OtherType> void update(const OtherType &other)
	{
		std::set<KeyType>::insert(other.begin(), other.end());
	}

	KeyType pop()
	{
		if (std::set<KeyType>::empty())
			throw KeyError(String());
		KeyType r(std::move(*std::set<KeyType>::begin()));
		std::set<KeyType>::erase(std::set<KeyType>::begin());
		return r;
	}

	void remove(const KeyType &elem)
	{
		auto it = std::set<KeyType>::find(elem);
		if (it == std::set<KeyType>::end())
			throw KeyError(elem);
		std::set<KeyType>::erase(it);
	}

	void discard(const KeyType &elem)
	{
		auto it = std::set<KeyType>::find(elem);
		if (it != std::set<KeyType>::end())
			std::set<KeyType>::erase(it);
	}

	Set difference(const Set &other) const
	{
		Set r;
		for (auto &&el : *this)
			if (!in(el, other))
				r.add(el);
		return r;
	}

	Set intersection(const Set &other) const
	{
		Set result;
		std::set_intersection(std::set<KeyType>::begin(), std::set<KeyType>::end(), other.begin(), other.end(), std::inserter(result, result.end()));
		return result;
	}
	Set operator&(const Set &other) const
	{
		return intersection(other);
	}

	bool is_disjoint(const Set &set2) const // [https://stackoverflow.com/a/1964252/2692494 <- google:‘isdisjoint c++’]
	{
		if(empty() || set2.empty()) return true;

		const_iterator
			it1 = begin(),
			it1End = end(),
			it2 = set2.begin(),
			it2End = set2.end();

		if(*it1 > *set2.rbegin() || *it2 > *rbegin()) return true;

		while(it1 != it1End && it2 != it2End)
		{
			if(*it1 == *it2) return false;
			if(*it1 < *it2) { it1++; }
			else { it2++; }
		}

		return true;
	}

	Set set_union(const Set &other) const
	{
		Set result;
		std::set_union(std::set<KeyType>::begin(), std::set<KeyType>::end(), other.begin(), other.end(), std::inserter(result, result.end()));
		return result;
	}
	Set operator|(const Set &other) const
	{
		return set_union(other);
	}
	template <typename OtherType> void operator|=(const OtherType &other)
	{
		update(other);
	}

	Set symmetric_difference(const Set &other) const
	{
		Set result;
		std::set_symmetric_difference(std::set<KeyType>::begin(), std::set<KeyType>::end(), other.begin(), other.end(), std::inserter(result, result.end()));
		return result;
	}

	bool is_subset(const Set &other) const
	{
		return std::includes(other.begin(), other.end(), std::set<KeyType>::begin(), std::set<KeyType>::end());
	}
	bool operator<=(const Set &other) const { return is_subset(other); }
	bool operator< (const Set &other) const { return is_subset(other) && len() != other.len(); }
	bool operator>=(const Set &other) const { return other <= *this; }
	bool operator> (const Set &other) const { return other <  *this; }

	Nullable<KeyType> lower_bound(const KeyType &key) const
	{
		auto it = std::set<KeyType>::lower_bound(key);
		return it != std::set<KeyType>::end() ? Nullable<KeyType>(*it) : Nullable<KeyType>(nullptr);
	}

	Nullable<KeyType> upper_bound(const KeyType &key) const
	{
		auto it = std::set<KeyType>::upper_bound(key);
		return it != std::set<KeyType>::end() ? Nullable<KeyType>(*it) : Nullable<KeyType>(nullptr);
	}

	template <typename Func> auto map(Func &&func) const
	{
		Array<decltype(func(std::declval<KeyType>()))> r;
		r.reserve(len());
		for (auto &&el : *this)
			r.push_back(func(el));
		return r;
	}

	template <typename Func> auto filter(Func &&func) const
	{
		Array<KeyType> r;
		for (auto &&el : *this)
			if (func(el))
				r.push_back(el);
		return r;
	}

	friend Set operator-(const Set &s1, const Set &s2)
	{
		Set result;
		std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(result, result.end())); // [https://stackoverflow.com/a/284004 <- google:‘c++ set difference’]
		return result;
	}
};

template <typename Type> Set<Type> create_set(std::initializer_list<Type> il)
{
	return Set<Type>(il);
}

template <typename Type> Set<Type> create_set(const Array<Type> &arr)
{
	Set<Type> r;
	for (auto &&el : arr)
		r.add(el);
	return r;
}

template <typename Type, bool include_beginning, bool include_ending> Set<Type> create_set(const Range<Type, include_beginning, include_ending> &range)
{
	Set<Type> r;
	for (auto i : range)
		r.add(i);
	return r;
}

template <typename Type> Array<Type> create_array(const Set<Type> &set)
{
	Array<Type> r;
	r.reserve(set.size());
	for (auto &&el : set)
		r.append(el);
	return r;
}

template <typename Ty, typename KeyType> inline bool in(const Ty &key, const Set<KeyType> &set)
{
	return set.find(key) != set.end();
}

template <typename KeyType> KeyType min(const Set<KeyType> &s)
{
	KeyType r = *begin(s);
	for (auto i : s)
		if (i < r) r = i;
	return r;
}

template <typename KeyType> KeyType max(const Set<KeyType> &s)
{
	KeyType r = *begin(s);
	for (auto i : s)
		if (i > r) r = i;
	return r;
}
