﻿#include <set>

template <typename KeyType> class Set : public std::set<KeyType>
{
public:
	Set() {}
	Set(std::initializer_list<KeyType> il)
	{
//		reserve(il.size());
		for (auto &&el : il)
			add(el);
	}

	int len() const {return (int)std::set<KeyType>::size();}

	bool add(const KeyType &k)
	{
		auto r = std::set<KeyType>::insert(k);
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
	for (auto &&el : set)
		r.append(el);
	return r;
}

template <typename KeyType> inline bool in(const KeyType &key, const Set<KeyType> &set)
{
	return set.find(key) != set.end();
}
