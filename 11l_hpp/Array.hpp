﻿#include <initializer_list>
#include <vector>

class ValueError
{
public:
	String value;

	template <typename ValueTy> ValueError(const ValueTy &value) : value(String(value)) {}
};

template <typename Type> class Array : public std::vector<Type>
{
	Array slice(int begin, int end, int step = 1) const
	{
		Array r;
		for (int i = begin; i < end; i += step)
			r.push_back(std::vector<Type>::at(i));
		return r;
	}

public:
	Array() {}
	Array(std::initializer_list<Type> il) : std::vector<Type>(il) {}
	Array(const String &s)
	{
		std::vector<Type>::reserve(s.len());
		for (auto c : s)
			append(c);
	}

	using std::vector<Type>::begin,
	      std::vector<Type>::end,
	      std::vector<Type>::clear,
	      std::vector<Type>::reserve;

	template <typename Func> auto map(Func &&func) const -> Array<decltype(func(std::declval<Type>()))>
	{
		Array<decltype(func(std::declval<Type>()))> r;
		r.reserve(len());
		for (auto &&el : *this)
			r.push_back(func(el));
		return r;
	}

	template <typename Func> auto map2(Func &&func) const
	{
		Array<decltype(func(std::declval<std::tuple_element_t<0, Type>>(), std::declval<std::tuple_element_t<1, Type>>()))> r;
		r.reserve(len());
		for (auto &&el : *this)
			r.push_back(func(std::get<0>(el), std::get<1>(el)));
		return r;
	}

	String join(const String &str)
	{
		String r;
		auto it = begin();
		if (it != end()) {
			r += *it;
			for (++it; it != end(); ++it) {
				r += str;
				r += *it;
			}
		}
		return r;
	}

	template <typename Func> Array filter(Func &&func) const
	{
		Array r;
		for (auto &&el : *this)
			if (func(el))
				r.push_back(el);
		return r;
	}

	template <typename Func> Array filter2(Func &&func) const
	{
		Array r;
		for (auto &&el : *this)
			if (func(std::get<0>(el), std::get<1>(el)))
				r.push_back(el);
		return r;
	}

	template <typename Func> Type reduce(Func &&func) const
	{
		Type r = (*this)[0];
		for (auto it = begin() + 1; it != end(); ++it)
			r = func(r, *it);
		return r;
	}

	template <typename Func> Type reduce(const Type &initial, Func &&func) const
	{
		Type r = initial;
		for (auto &&el : *this)
			r = func(r, el);
		return r;
	}

	int len() const {return (int)std::vector<Type>::size();}

	friend Array &&operator*(Array &&a, int n)
	{
		if (n < 1) // mimic Python's behavior in which [1] * 0 = [] and [1] * -1 = []
			a.clear();
		else {
			int len = a.len();
			a.reserve(len * n);
			for (int i = 1; i < n; i++)
				for (int j = 0; j < len; j++)
					a.append(a[j]);
		}
		return std::move(a);
	}
	friend Array &&operator*(int n, Array &&a)
	{
		return std::move(a) * n;
	}

	void operator*=(int n)
	{
		if (n < 1)
			clear();
		else {
			int l = len();
			reserve(l * n);
			for (int i = 1; i < n; i++)
				for (int j = 0; j < l; j++)
					append(std::vector<Type>::data()[j]);
		}
	}

	Array operator[](const Range<int, true,  true > range) const {return slice(max(range.b    , 0), min(range.e + 1, len()));}
	Array operator[](const Range<int, true,  false> range) const {return slice(max(range.b    , 0), min(range.e    , len()));}
	Array operator[](const Range<int, false, true > range) const {return slice(max(range.b + 1, 0), min(range.e + 1, len()));}
	Array operator[](const Range<int, false, false> range) const {return slice(max(range.b + 1, 0), min(range.e    , len()));}
	Array operator[](const RangeEI<int>             range) const {return slice(max(range.b    , 0),                  len() );}
	Array operator[](const RangeWithStep<int, true,  true > range) const {return slice(max(range.b    , 0), min(range.e + 1, len()), range.step);}
	Array operator[](const RangeWithStep<int, true,  false> range) const {return slice(max(range.b    , 0), min(range.e    , len()), range.step);}
	Array operator[](const RangeWithStep<int, false, true > range) const {return slice(max(range.b + 1, 0), min(range.e + 1, len()), range.step);}
	Array operator[](const RangeWithStep<int, false, false> range) const {return slice(max(range.b + 1, 0), min(range.e    , len()), range.step);}
	Array operator[](const RangeEIWithStep<int>             range) const {return slice(max(range.b    , 0),                  len() , range.step);}

	Array operator[](const range_e_llen    range) const {return (*this)[range_el(        range.b, len() + range.e)];}
	Array operator[](const range_elen_elen range) const {return (*this)[range_ee(len() + range.b, len() + range.e)];}
	Array operator[](const range_elen_llen range) const {return (*this)[range_el(len() + range.b, len() + range.e)];}
	Array operator[](const range_elen_i    range) const {return (*this)[range_ei(len() + range.b)];}

	const Type &operator[](int i) const
	{
		if (in(i, range_el(0, len())))
			return std::vector<Type>::at(i);
		throw IndexError(i);
	}

	Type &operator[](int i)
	{
		if (in(i, range_el(0, len())))
			return std::vector<Type>::at(i);
		throw IndexError(i);
	}

	const Type &at_plus_len(int i) const
	{
		return (*this)[len() + i];
	}

	Type &at_plus_len(int i)
	{
		return (*this)[len() + i];
	}

	void set(int i, const Type &v)
	{
		if (in(i, range_el(0, len())))
			std::vector<Type>::at(i) = v;
		else
			throw IndexError(i);
	}

	void set_plus_len(int i, const Type &v)
	{
		set(len() + i, v);
	}

	void append(Type &&v) {std::vector<Type>::push_back(std::move(v));}
	void append(const Type &v) {std::vector<Type>::push_back(v);}

	void append(const Array<Type> &arr)
	{
		reserve(std::vector<Type>::size() + arr.size());
		for (auto el : arr)
			append(el);
	}

	template <typename Ty, bool include_beginning, bool include_ending> void append(const Range<Ty, include_beginning, include_ending> &range)
	{
		reserve(size() + range.size());
		for (auto i : range)
			append(i);
	}
//	template <typename Ty, bool include_beginning, bool include_ending> void operator+=(const Range<Ty, include_beginning, include_ending> &range)
//	{
//		append(range);
//	}

	template <typename Ty> void extend(const Ty &iterable)
	{
		reserve(std::vector<Type>::size() + iterable.len());
		for (auto el : iterable)
			append(el);
	}

	void insert(int index, const Type &v)
	{
		std::vector<Type>::insert(begin() + index, v);
	}

	int index(const Type &v) const
	{
		for (int i=0, l=len(); i<l; i++)
			if (std::vector<Type>::data()[i] == v) return i;
		throw ValueError(v);
	}

	Nullable<int> find(const Type &val, int start = 0) const
	{
		auto it = std::find(begin() + start, end(), val);
		if (it != end())
			return int(it - begin());
		return nullptr;
	}

	Nullable<int> find(const Tuple<Type, Type> &t, int start = 0) const
	{
		for (auto it = begin() + start; it != end(); ++it) {
			if (*it == std::get<0>(t)) return int(it - begin());
			if (*it == std::get<1>(t)) return int(it - begin());
		}
		return nullptr;
	}

	template <typename Ty> Nullable<int> find(const Tuple<Ty, Ty> &t, int start = 0) const
	{
		return find(make_tuple((Type)std::get<0>(t), (Type)std::get<1>(t)), start);
	}

	int count(const Type &val) const
	{
		return (int)std::count(begin(), end(), val);
	}

	void sort()
	{
		std::sort(begin(), end());
	}

	void reverse()
	{
		std::reverse(begin(), end());
	}

	const Type &last() const
	{
		if (std::vector<Type>::empty())
			throw IndexError(0);
		return std::vector<Type>::at(len()-1);
	}

	Type &last()
	{
		if (std::vector<Type>::empty())
			throw IndexError(0);
		return std::vector<Type>::at(len()-1);
	}

	Type pop()
	{
		if (std::vector<Type>::empty())
			throw IndexError(0);
		Type r(std::move(std::vector<Type>::at(len()-1)));
		std::vector<Type>::pop_back();
		return r;
	}

	Type pop(int i)
	{
		Type r(std::move((*this)[i]));
		std::vector<Type>::erase(begin() + i);
		return r;
	}

	void remove(const Type &val)
	{
		auto it = std::find(begin(), end(), val);
		if (it == end())
			throw ValueError(val);
		std::vector<Type>::erase(it);
	}

	template <typename Ty> bool operator==(const Array<Ty> &arr) const
	{
		if (len() != arr.len()) return false;
		for (int i=0, n=len(); i<n; i++)
			if (std::vector<Type>::at(i) != arr.at(i))
				return false;
		return true;
	}
	template <typename Ty> bool operator!=(const Array<Ty> &arr) const
	{
		return !(*this == arr);
	}

	Array operator+(const Array &a) const
	{
		Array r(*this);
		r.append(a);
		return r;
	}

	Array operator+(const Type &t) const
	{
		Array r(*this);
		r.append(t);
		return r;
	}

	friend Array operator+(const Type &t, const Array &a)
	{
		Array r;
		r.append(t);
		r.append(a);
		return r;
	}
};

template <typename Type> Array<Type> create_array(std::initializer_list<Type> il)
{
	return Array<Type>(il);
}

Array<char> create_array(std::initializer_list<bool> il) // avoid using std::vector<bool>
{
	Array<char> r;
	r.reserve(il.size());
	for (bool b : il)
		r.push_back(b);
	return r;
}

template <typename Type, bool include_beginning, bool include_ending> Array<Type> create_array(const Range<Type, include_beginning, include_ending> &range)
{
	Array<Type> r;
	r.reserve(range.size());
	for (auto i : range)
		r.push_back(i);
	return r;
}

template <typename Ty, typename Type> bool in(const Ty &val, const Array<Type> &arr)
{
	return std::find(arr.begin(), arr.end(), val) != arr.end();
}

template <typename Iterable> auto sum(const Iterable &iterable)
{
	std::remove_const_t<std::remove_reference_t<decltype(*std::begin(iterable))>> r = 0; // >[https://stackoverflow.com/questions/15887144/stdremove-const-with-const-references <- google:‘c++ remove_const’]:‘In order to strip `const` away, you first have to apply `std::remove_reference`, ~‘then’ apply `std::remove_const`’
	//auto r = decltype(*std::begin(iterable))(0); // this also works
	for (auto v : iterable)
		r += v;
	return r;
}

template <typename Iterable, typename Func> auto sum_map(const Iterable &iterable, Func &&func)
{
	decltype(func(std::declval<decltype(*std::begin(iterable))>())) r = 0;
	for (auto &&el : iterable)
		r += func(el);
	return r;
}

template <typename Type> Type product(const Array<Type> &arr)
{
	Type r = 1;
	for (auto v : arr)
		r *= v;
	return r;
}

template <typename Type> auto enumerate(const Array<Type> &arr)
{
	Array<Tuple<int, Type>> r;
	int i = 0;
	for (auto &&v : arr)
		r.append(make_tuple(i++, v));
	return r;
}

inline Array<String> String::split(const String &delim, Nullable<int> limit, bool group_delimiters) const
{
	int lim = limit == nullptr ? -1 : *limit - 1;
	Array<String> arr;
	if (lim == 0) {
		arr.append(*this);
		return arr;
	}
	arr.reserve(limit == nullptr ? count(delim) + 1 : *limit);
	const char16_t *str = data(), *begin = str, *end = data() + len() - delim.len() + 1;
	while (str < end)
		if (memcmp(str, delim.data(), delim.len()*sizeof(char16_t)) == 0)
		{
			if (!group_delimiters || str != begin)
				arr.append(String(begin, str-begin));
			str += delim.len();
			if (--lim == 0) {
				arr.append(String(str, end-str));
				return arr;
			}
			begin = str;
		}
		else
			str++;

	if (!group_delimiters || str != begin)
		arr.append(String(begin, str-begin));
	return arr;
}

template <typename ... Types> inline Array<String> String::split(const Tuple<Types...> &delim_tuple, Nullable<int> limit, bool group_delimiters) const
{
	int lim = limit == nullptr ? -1 : *limit - 1;
	Array<String> arr;
	if (lim == 0) {
		arr.append(*this);
		return arr;
	}
	if (limit != nullptr)
		arr.reserve(*limit);
	const char16_t *str = data(), *begin = str, *end = data() + len();
	while (str < end)
		if (in(*str, delim_tuple))
		{
			if (!group_delimiters || str != begin)
				arr.append(String(begin, str-begin));
			str++;
			if (--lim == 0) {
				arr.append(String(str, end-str));
				return arr;
			}
			begin = str;
		}
		else
			str++;

	if (!group_delimiters || str != begin)
		arr.append(String(begin, str-begin));
	return arr;
}

inline String String::ltrim(const Array<Char> &arr, Nullable<int> limit) const
{
	const char16_t *s = data();
	int i = 0;
	for (int l=limit == nullptr ? len() : min(len(), *limit); i<l; i++)
		if (!in(s[i], arr)) break;
	return String(s + i, len() - i);
}

inline String String::rtrim(const Array<Char> &arr, Nullable<int> limit) const
{
	const char16_t *s = data();
	int l = len()-1;
	for (int ll=limit == nullptr ? 0 : max(0, len()-*limit); l>=ll; l--)
		if (!in(s[l], arr)) break;
	return String(s, l+1);
}

template <typename Type> Array<Type> sorted(const Array<Type> &arr)
{
	Array<Type> r(arr);
	std::sort(r.begin(), r.end());
	return r;
}

template <typename Type, typename Key> Array<Type> sorted(const Array<Type> &arr, Key key, bool reverse = false)
{
	Array<Type> r(arr);
	if (!reverse)
		std::sort(r.begin(), r.end(), [key](const Type &a, const Type &b) {return key(a) < key(b);});
	else
		std::sort(r.begin(), r.end(), [key](const Type &a, const Type &b) {return key(b) < key(a);});
	return r;
}

Array<Char> sorted(const String &s)
{
	Array<Char> arr;
	arr.assign(s.cbegin(), s.cend());
	std::sort(arr.begin(), arr.end());
	return arr;
}

template <typename Ty> bool operator>(const Tvec<Ty, 2> &v1, const Tvec<Ty, 2> &v2) {return std::make_tuple(v1[0], v1[1]) > std::make_tuple(v2[0], v2[1]);}
template <typename Ty> bool operator>(const Tvec<Ty, 3> &v1, const Tvec<Ty, 3> &v2) {return std::make_tuple(v1[0], v1[1], v1[2]) > std::make_tuple(v2[0], v2[1], v2[2]);}
template <typename Ty> bool operator>(const Tvec<Ty, 4> &v1, const Tvec<Ty, 4> &v2) {return std::make_tuple(v1[0], v1[1], v1[2], v1[3]) > std::make_tuple(v2[0], v2[1], v2[2], v2[3]);}

template <typename Type> Type max(const Array<Type> &arr)
{
	Type r = arr[0];
	for (auto i : arr)
		if (i > r) r = i;
	return r;
}

template <typename Ty> bool operator<(const Tvec<Ty, 2> &v1, const Tvec<Ty, 2> &v2) {return std::make_tuple(v1[0], v1[1]) < std::make_tuple(v2[0], v2[1]);}
template <typename Ty> bool operator<(const Tvec<Ty, 3> &v1, const Tvec<Ty, 3> &v2) {return std::make_tuple(v1[0], v1[1], v1[2]) < std::make_tuple(v2[0], v2[1], v2[2]);}
template <typename Ty> bool operator<(const Tvec<Ty, 4> &v1, const Tvec<Ty, 4> &v2) {return std::make_tuple(v1[0], v1[1], v1[2], v1[3]) < std::make_tuple(v2[0], v2[1], v2[2], v2[3]);}

template <typename Type> Type min(const Array<Type> &arr)
{
	Type r = arr[0];
	for (auto i : arr)
		if (i < r) r = i;
	return r;
}
