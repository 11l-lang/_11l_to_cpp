﻿#include <initializer_list>
#include <vector>

template <typename Type, int N> String to_string(const Tvec<Type, N> &v)
{
	String r;
	for (int i = 0; i < N; i++) {
		r &= String(v[i]);
		if (i < N-1)
			r &= u", ";
	}
	return r;
}

template <typename ValType> Int String::index(const ValType &v) const
{
	Int r = findi(v);
	if (r == -1) throw ValueError(v);
	return r;
}

// [https://stackoverflow.com/a/48458312 <- google:‘c++ is tuple’]
template <typename> struct is_tuple_or_vec : std::false_type {};
template <typename ...T> struct is_tuple_or_vec<std::tuple<T...>> : std::true_type {};
template <typename Ty, int N> struct is_tuple_or_vec<Tvec<Ty, N>> : std::true_type {};
template <typename Type, size_t i, typename = std::enable_if_t< is_tuple_or_vec<Type>::value>> std::tuple_element_t<(i < std::tuple_size<Type>::value ? i : 0), Type> tuple_element_f();
template <typename Type, size_t i, typename = std::enable_if_t<!is_tuple_or_vec<Type>::value>> void tuple_element_f();

template <typename Type> class Array : public std::vector<Type>
{
	Array slice(Int begin, Int end, Int step = 1) const
	{
		Array r;
		if (step > 0) {
			r.reserve(max(Int(0), (end - begin + step - 1) / step));
			for (Int i = begin; i < end; i += step)
				r.push_back(std::vector<Type>::at(i));
		}
		else
			for (Int i = begin; i > end; i += step)
				r.push_back(std::vector<Type>::at(i));
		return r;
	}

	Array slice_e(Int begin, Int end, Int step = 1) const
	{
		Array r;
		if (step > 0) {
			r.reserve(max(Int(0), (end - begin + step) / step));
			for (Int i = begin; i <= end; i += step)
				r.push_back(std::vector<Type>::at(i));
		}
		else
			for (Int i = begin; i >= end; i += step)
				r.push_back(std::vector<Type>::at(i));
		return r;
	}

public:
	Array() {}
	Array(std::initializer_list<Type> il) : std::vector<Type>(il) {}
	explicit Array(const String &s)
	{
		std::vector<Type>::reserve(s.len());
		for (auto c : s)
			append(c);
	}

	using std::vector<Type>::begin,
	      std::vector<Type>::end,
	      std::vector<Type>::clear,
	      std::vector<Type>::reserve;

	void drop() {clear();}

	template <typename Func> auto map(Func &&func) const -> Array<decltype(func(std::declval<Type>()))>
	{
		Array<decltype(func(std::declval<Type>()))> r;
		r.reserve(len());
		for (auto &&el : *this)
			r.push_back(func(el));
		return r;
	}

	template <typename Func, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
	                                                                  std::declval<decltype(tuple_element_f<Type, 1>())>()))> auto map(Func &&func) const
	{
		Array<decltype(func(std::declval<std::tuple_element_t<0, Type>>(), std::declval<std::tuple_element_t<1, Type>>()))> r;
		r.reserve(len());
		for (auto &&el : *this)
			r.push_back(func(_get<0>(el), _get<1>(el)));
		return r;
	}

	template <typename Func, typename Dummy = int, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
	                                                                                        std::declval<decltype(tuple_element_f<Type, 1>())>(),
	                                                                                        std::declval<decltype(tuple_element_f<Type, 2>())>()))> auto map(Func &&func) const
	{
		Array<decltype(func(std::declval<std::tuple_element_t<0, Type>>(), std::declval<std::tuple_element_t<1, Type>>(), std::declval<std::tuple_element_t<2, Type>>()))> r;
		r.reserve(len());
		for (auto &&el : *this)
			r.push_back(func(_get<0>(el), _get<1>(el), _get<2>(el)));
		return r;
	}

	String join(const String &str) const
	{
		String r;
		auto it = begin();
		if (it != end()) {
			r &= *it;
			for (++it; it != end(); ++it) {
				r &= str;
				r &= *it;
			}
		}
		return r;
	}

	template <typename Func, typename = decltype(std::declval<Func>()(std::declval<Type>()))> Array filter(Func &&func) const
	{
		Array r;
		for (auto &&el : *this)
			if (func(el))
				r.push_back(el);
		return r;
	}

	template <typename Func, typename Dummy = int, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
	                                                                                        std::declval<decltype(tuple_element_f<Type, 1>())>()))> Array filter(Func &&func) const
	{
		Array r;
		for (auto &&el : *this)
			if (func(_get<0>(el), _get<1>(el)))
				r.push_back(el);
		return r;
	}

	template <typename Func, typename Dummy = int, typename Dummy2 = int, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
	                                                                                                               std::declval<decltype(tuple_element_f<Type, 1>())>(),
	                                                                                                               std::declval<decltype(tuple_element_f<Type, 2>())>()))> Array filter(Func &&func) const
	{
		Array r;
		for (auto &&el : *this)
			if (func(_get<0>(el), _get<1>(el), _get<2>(el)))
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

	Int len() const {return (Int)std::vector<Type>::size();}

	friend Array operator*(Array a, Int n)
	{
		if (n < 1) // mimic Python's behavior in which [1] * 0 = [] and [1] * -1 = []
			a.clear();
		else {
			Int len = a.len();
			a.reserve(len * n);
			for (Int i = 1; i < n; i++)
				for (Int j = 0; j < len; j++)
					a.append(a[j]);
		}
		return std::move(a);
	}
	friend Array operator*(Int n, Array a)
	{
		return std::move(a) * n;
	}
/*	friend Array operator*(const Array &a, Int n)
	{
		Array r;
		if (n >= 1) { // mimic Python's behavior in which [1] * 0 = [] and [1] * -1 = []
			Int len = a.len();
			r.reserve(len * n);
			for (Int i = 0; i < n; i++)
				for (Int j = 0; j < len; j++)
					r.append(a[j]);
		}
		return r;
	}
	friend Array operator*(Int n, const Array &a)
	{
		return a * n;
	}*/

	void operator*=(Int n)
	{
		if (n < 1)
			clear();
		else {
			Int l = len();
			reserve(l * n);
			for (Int i = 1; i < n; i++)
				for (Int j = 0; j < l; j++)
					append(std::vector<Type>::data()[j]);
		}
	}

	Array operator[](const Range<Int, true,  true > range) const {return slice_e(max(range.b    , Int(0)), min(range.e, len()));}
	Array operator[](const Range<Int, true,  false> range) const {return slice  (max(range.b    , Int(0)), min(range.e, len()));}
	Array operator[](const Range<Int, false, true > range) const {return slice_e(max(range.b + 1, Int(0)), min(range.e, len()));}
	Array operator[](const Range<Int, false, false> range) const {return slice  (max(range.b + 1, Int(0)), min(range.e, len()));}
	Array operator[](const RangeEI<Int>             range) const {return slice  (max(range.b    , Int(0)),              len() );}
	Array operator[](const RangeWithStep<Int, true,  true > range) const {return slice_e(max(range.b    , Int(0)), min(range.e, len()), range.step);}
	Array operator[](const RangeWithStep<Int, true,  false> range) const {return slice  (max(range.b    , Int(0)), min(range.e, len()), range.step);}
	Array operator[](const RangeWithStep<Int, false, true > range) const {return slice_e(max(range.b + 1, Int(0)), min(range.e, len()), range.step);}
	Array operator[](const RangeWithStep<Int, false, false> range) const {return slice  (max(range.b + 1, Int(0)), min(range.e, len()), range.step);}
	Array operator[](const RangeEIWithStep<Int>             range) const {return slice  (max(range.b    , Int(0)), range.step > 0 ? len() : -1, range.step);}

	Array operator[](const range_e_llen    range) const {return (*this)[range_el(        range.b, len() + range.e)];}
	Array operator[](const range_elen_elen range) const {return (*this)[range_ee(len() + range.b, len() + range.e)];}
	Array operator[](const range_elen_llen range) const {return (*this)[range_el(len() + range.b, len() + range.e)];}
	Array operator[](const range_elen_i    range) const {return (*this)[range_ei(len() + range.b)];}
	Array operator[](const range_elen_i_wstep range) const {return (*this)[range_ei(len() + range.b).step(range.s)];}
	Array operator[](const range_elen_e_wstep range) const {return (*this)[range_ee(len() + range.b, range.e).step(range.s)];}

	decltype(std::declval<const std::vector<Type>>().at(0)) operator[](Int i) const // decltype is needed for Array<bool> support
	{
#ifdef PYTHON_NEGATIVE_INDEXING
		if (i < 0)
			i += len();
#endif
		if (in(i, range_el(Int(0), len())))
			return std::vector<Type>::operator[](i);
		throw IndexError(i);
	}

	decltype(std::declval<std::vector<Type>>().at(0)) operator[](Int i) // decltype is needed for Array<bool> support
	{
#ifdef PYTHON_NEGATIVE_INDEXING
		if (i < 0)
			i += len();
#endif
		if (in(i, range_el(Int(0), len())))
			return std::vector<Type>::operator[](i);
		throw IndexError(i);
	}

	const Type &at_plus_len(Int i) const
	{
#ifndef PYTHON_NEGATIVE_INDEXING
		i += len();
#endif
		return operator[](i);
	}

	Type &at_plus_len(Int i)
	{
#ifndef PYTHON_NEGATIVE_INDEXING
		i += len();
#endif
		return operator[](i);
	}

	const Type &set(Int i, const Type &v) // return `const Type&` for [https://www.rosettacode.org/wiki/Perlin_noise#Python]:‘p[256+i] = p[i] = permutation[i]’
	{
#ifdef PYTHON_NEGATIVE_INDEXING
		if (i < 0)
			i += len();
#endif
		if (in(i, range_el(Int(0), len())))
			return std::vector<Type>::operator[](i) = v;
		else
			throw IndexError(i);
	}

	const Type &set_plus_len(Int i, const Type &v)
	{
#ifndef PYTHON_NEGATIVE_INDEXING
		i += len();
#endif
		return set(i, v);
	}

	decltype(std::declval<const std::vector<Type>>().at(0)) at_ni(Int ii) const // decltype is needed for Array<bool> support
	{
		Int i = ii;
		if (i < 0)
			i += len();
		if (in(i, range_el(Int(0), len())))
			return std::vector<Type>::operator[](i);
		throw IndexError(ii);
	}

	decltype(std::declval<std::vector<Type>>().at(0)) at_ni(Int ii) // decltype is needed for Array<bool> support
	{
		Int i = ii;
		if (i < 0)
			i += len();
		if (in(i, range_el(Int(0), len())))
			return std::vector<Type>::operator[](i);
		throw IndexError(ii);
	}

	void append(Type &&v) {std::vector<Type>::push_back(std::move(v));}
	void append(const Type &v) {std::vector<Type>::push_back(v);}

	void operator_append(Type &&v) {append(std::move(v));}
	void operator_append(const Type &v) {append(v);}
	template <typename Ty> void operator_append(const Ty &obj)
	{
		extend(obj);
	}

	template <typename Ty, bool include_beginning, bool include_ending> void extend(const Range<Ty, include_beginning, include_ending> &range)
	{
		reserve(std::vector<Type>::size() + range.size());
		for (auto i : range)
			append(i);
	}
//	template <typename Ty, bool include_beginning, bool include_ending> void operator+=(const Range<Ty, include_beginning, include_ending> &range)
//	{
//		extend(range);
//	}

	template <typename Ty> void extend(const Ty &iterable)
	{
		reserve(std::vector<Type>::size() + iterable.len());
		for (auto &&el : iterable)
			append(el);
	}

	void insert(Int index, const Type &v)
	{
		std::vector<Type>::insert(begin() + index, v);
	}

	Int index(const Type &v, Int start, Int end) const
	{
		for (Int i=start; i<end; i++)
			if (std::vector<Type>::data()[i] == v) return i;
		throw ValueError(v);
	}
	Int index(const Type &v, Int start = 0) const
	{
		return index(v, start, len());
	}

	Nullable<Int> find(const Type &val, Int start = 0) const
	{
		auto it = std::find(begin() + start, end(), val);
		if (it != end())
			return Int(it - begin());
		return nullptr;
	}

	Nullable<Int> find(const decltype(make_tuple(std::declval<Type>(), std::declval<Type>())) &t, Int start = 0) const
	{
		for (auto it = begin() + start; it != end(); ++it) {
			if (*it == _get<0>(t)) return Int(it - begin());
			if (*it == _get<1>(t)) return Int(it - begin());
		}
		return nullptr;
	}

	template <typename Ty> Nullable<Int> find(const Tuple<Ty, Ty> &t, Int start = 0) const
	{
		return find(make_tuple((Type)_get<0>(t), (Type)_get<1>(t)), start);
	}
	template <typename Ty> Nullable<Int> find(const Tvec<Ty, 2> &t, Int start = 0) const
	{
		return find(make_tuple((Type)_get<0>(t), (Type)_get<1>(t)), start);
	}

	Int count(const Type &val) const
	{
		return (Int)std::count(begin(), end(), val);
	}

	void sort()
	{
		std::sort(begin(), end());
	}

	template <typename Key> void sort(Key &&key, bool reverse = false)
	{
		if (!reverse)
			std::sort(begin(), end(), [key](const Type &a, const Type &b) {return key(a) < key(b);});
		else
			std::sort(begin(), end(), [key](const Type &a, const Type &b) {return key(b) < key(a);});
	}

	void sort(nullptr_t, bool reverse)
	{
		if (!reverse)
			std::sort(begin(), end());
		else
			std::sort(begin(), end(), std::greater<>()); // [https://stackoverflow.com/a/37757410/2692494 <- google:‘std sort reverse’]
	}

	void sort_range(const Range<Int, true,  true> range) {std::sort(begin() + range.b, begin() + range.e + 1);}
	void sort_range(const Range<Int, true, false> range) {std::sort(begin() + range.b, begin() + range.e);}
	void sort_range(const RangeEI<Int>            range) {std::sort(begin() + range.b, end());}

	void sort_range(const Range<Int, true, false> range, nullptr_t, bool reverse)
	{
		if (!reverse)
			std::sort(begin() + range.b, begin() + range.e);
		else
			std::sort(begin() + range.b, begin() + range.e, std::greater<>());
	}

	void reverse()
	{
		std::reverse(begin(), end());
	}

	void reverse_range(const Range<Int, true,  true> range) {std::reverse(begin() + range.b, begin() + range.e + 1);}
	void reverse_range(const Range<Int, true, false> range) {std::reverse(begin() + range.b, begin() + range.e);}
	void reverse_range(const RangeEI<Int>            range) {std::reverse(begin() + range.b, end());}
	void reverse_range(const range_elen_i            range) {reverse_range(range_ei(len() + range.b));}

	bool next_permutation()
	{
		return std::next_permutation(begin(), end());
	}

	bool is_sorted() const
	{
		return std::is_sorted(begin(), end());
	}

	const Type &last() const
	{
		if (std::vector<Type>::empty())
			throw IndexError(0);
		return std::vector<Type>::operator[](len()-1);
	}

	Type &last()
	{
		if (std::vector<Type>::empty())
			throw IndexError(0);
		return std::vector<Type>::operator[](len()-1);
	}

	Type pop()
	{
		if (std::vector<Type>::empty())
			throw IndexError(0);
		Type r(std::move(std::vector<Type>::operator[](len()-1)));
		std::vector<Type>::pop_back();
		return r;
	}

	Type pop(Int i)
	{
#ifdef PYTHON_NEGATIVE_INDEXING
		if (i < 0)
			i += len();
#endif
		if (!in(i, range_el(Int(0), len())))
			throw IndexError(i);
		Type r(std::move((*this)[i]));
		std::vector<Type>::erase(begin() + i);
		return r;
	}

	Type pop_fast(Int i)
	{
#ifdef PYTHON_NEGATIVE_INDEXING
		if (i < 0)
			i += len();
#endif
		if (!in(i, range_el(Int(0), len())))
			throw IndexError(i);
		Type r(std::move((*this)[i]));
		(*this)[i] = std::move(std::vector<Type>::back());
		std::vector<Type>::pop_back();
		return r;
	}

	Type pop_plus_len(Int i)
	{
#ifndef PYTHON_NEGATIVE_INDEXING
		i += len();
#endif
		return pop(i);
	}

	void remove(const Type &val)
	{
		auto it = std::find(begin(), end(), val);
		if (it == end())
			throw ValueError(val);
		std::vector<Type>::erase(it);
	}

	void del(const Range<Int, true, false> range)
	{
		std::vector<Type>::erase(begin() + range.b, begin() + range.e);
	}
	void del(const RangeEI<Int> range)
	{
		std::vector<Type>::erase(begin() + range.b, end());
	}

	String decode(const String &encoding = u"utf-8"_S) const
	{
		static_assert(std::is_same_v<Type, Byte>, "`decode()` can be called only on `Array<Byte>`");

		if (encoding == u"ascii") {
			String r;
			r.reserve(len());
			for (auto el : *this) {
				if (!(el < 128))
					throw AssertionError();
				r &= Char(el);
			}
			return r;
		}
		else if (encoding == u"utf-8") {
			String convert_utf8_string_to_String(const char *s, size_t len);
			return convert_utf8_string_to_String((char*)std::vector<Type>::data(), std::vector<Type>::size());
		}
		else
			throw AssertionError();
	}

	String hex() const
	{
		static_assert(std::is_same_v<Type, Byte>, "`hex()` can be called only on `Array<Byte>`");

		String r;
		r.reserve(len() * 2);
		for (auto el : *this) {
			r &= Char(hex_to_char(el >> 4));
			r &= Char(hex_to_char(el & 0xF));
		}
		return r;
	}

	template <typename Ty> bool operator==(const Array<Ty> &arr) const
	{
		if (len() != arr.len()) return false;
		for (Int i=0, n=len(); i<n; i++)
			if (!(std::vector<Type>::operator[](i) == arr.operator[](i)))
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
		r.extend(a);
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
		r.extend(a);
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
	if (ptrdiff_t(range.len()) > 0)
		r.reserve(range.len());
	for (auto i : range)
		r.push_back(i);
	return r;
}

template <typename Type, bool include_beginning, bool include_ending> Array<Type> create_array(const RangeWithStep<Type, include_beginning, include_ending> &range)
{
	Array<Type> r;
	if (ptrdiff_t(range.len()) > 0)
		r.reserve(range.len());
	for (auto i : range)
		r.push_back(i);
	return r;
}

Array<Char> create_array(const String &s)
{
	Array<Char> r;
	r.reserve(s.len());
	for (auto c : s)
		r.push_back(c);
	return r;
}

template <typename Type> Array<Type> create_array(const Array<Type> &arr)
{
	return arr;
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

template <typename Iterable, typename Func> auto sum_map(const Iterable &iterable, Func &&func)// -> decltype(func(std::declval<decltype(*std::begin(iterable))>()))
{
	decltype(func(std::declval<decltype(*std::begin(iterable))>())) r = 0;
	for (auto &&el : iterable)
		r += func(el);
	return r;
}

template <typename Type, typename Func, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
                                                                                 std::declval<decltype(tuple_element_f<Type, 1>())>()))> auto sum_map(const Array<Type> &arr, Func &&func)
{
	decltype(func(std::declval<std::tuple_element_t<0, Type>>(), std::declval<std::tuple_element_t<1, Type>>())) r = 0;
	for (auto &&el : arr)
		r += func(_get<0>(el), _get<1>(el));
	return r;
}

template <typename Type, typename Func, typename Dummy = int, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
                                                                                                       std::declval<decltype(tuple_element_f<Type, 1>())>(),
                                                                                                       std::declval<decltype(tuple_element_f<Type, 2>())>()))> auto sum_map(const Array<Type> &arr, Func &&func)
{
	decltype(func(std::declval<std::tuple_element_t<0, Type>>(), std::declval<std::tuple_element_t<1, Type>>(), std::declval<std::tuple_element_t<2, Type>>())) r = 0;
	for (auto &&el : arr)
		r += func(_get<0>(el), _get<1>(el), _get<2>(el));
	return r;
}

template <typename Iterable> auto product(const Iterable &iterable)
{
	std::remove_const_t<std::remove_reference_t<decltype(*std::begin(iterable))>> r = 1;
	for (auto v : iterable)
		r *= v;
	return r;
}

template <typename Type> auto enumerate(const Array<Type> &arr, Int start = 0)
{
	Array<decltype(make_tuple(Int(), std::declval<Type>()))> r;
	r.reserve(arr.size());
	Int i = start;
	for (auto &&v : arr)
		r.append(make_tuple(i++, v));
	return r;
}

template <typename Func> Array<Char> String::filter(Func &&func) const
{
	Array<Char> r;
	for (Char c : *this)
		if (func(c))
			r.append(c);
	return r;
}

inline Array<String> String::split(const String &delim, Nullable<Int> limit, bool group_delimiters) const
{
	Int lim = limit == nullptr ? -1 : *limit - 1;
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
				arr.append(String(str, data() + len() - str));
				return arr;
			}
			begin = str;
		}
		else
			str++;
	//str += delim.len() - 1; // with this line of code there was an error in `u"[]\n===\n[]"_S.split(u"===\n"_S)` (second string in the resulting array has additional null character)
	str = data() + len();

	if (group_delimiters ? str != begin : !empty())
		arr.append(String(begin, str-begin));
	return arr;
}

template <typename ... Types> inline Array<String> String::split(const Tuple<Types...> &delim_tuple, Nullable<Int> limit, bool group_delimiters) const
{
	Int lim = limit == nullptr ? -1 : *limit - 1;
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

	if (group_delimiters ? str != begin : !empty())
		arr.append(String(begin, str-begin));
	return arr;
}

inline Array<String> String::split_py() const
{
	return split(make_tuple(u' '_C, u'\t'_C, u'\r'_C, u'\n'_C), nullptr, true);
}

Array<String> String::split_req(const String &delim, Int req) const
{
	Array<String> res = split(delim);
	if (res.len() != req)
		throw AssertionError();
	return res;
}

template <typename RangeOrTupleType> Array<String> String::split_req(const String &delim, const RangeOrTupleType &req) const
{
	Array<String> res = split(delim);
	if (!in(res.len(), req))
		throw AssertionError();
	return res;
}

inline String String::ltrim(const Array<Char> &arr, Nullable<Int> limit) const
{
	const char16_t *s = data();
	Int i = 0;
	for (Int l=limit == nullptr ? len() : min(len(), *limit); i<l; i++)
		if (!in(s[i], arr)) break;
	return String(s + i, len() - i);
}

inline String String::rtrim(const Array<Char> &arr, Nullable<Int> limit) const
{
	const char16_t *s = data();
	Int l = len()-1;
	for (Int ll=limit == nullptr ? 0 : max(Int(0), len()-*limit); l>=ll; l--)
		if (!in(s[l], arr)) break;
	return String(s, l+1);
}

template <typename Type> Array<Type> sorted(const Array<Type> &arr)
{
	Array<Type> r(arr);
	std::sort(r.begin(), r.end());
	return r;
}

template <typename Type, typename Key> Array<Type> sorted(const Array<Type> &arr, Key &&key, bool reverse = false)
{
	Array<Type> r(arr);
	if (!reverse)
		std::sort(r.begin(), r.end(), [key](const Type &a, const Type &b) {return key(a) < key(b);});
	else
		std::sort(r.begin(), r.end(), [key](const Type &a, const Type &b) {return key(b) < key(a);});
	return r;
}

template <typename Type> Array<Type> sorted(const Array<Type> &arr, nullptr_t, bool reverse)
{
	Array<Type> r(arr);
	if (!reverse)
		std::sort(r.begin(), r.end());
	else
		std::sort(r.begin(), r.end(), std::greater<>()); // [https://stackoverflow.com/a/37757410/2692494 <- google:‘std sort reverse’]
	return r;
}

String sorted(const String &s)
{
	String r(s);
	std::sort(r.std::u16string::begin(), r.std::u16string::end());
	return r;
}

String sorted(const String &s, nullptr_t, bool reverse)
{
	String r(s);
	if (!reverse)
		std::sort(r.std::u16string::begin(), r.std::u16string::end());
	else
		std::sort(r.std::u16string::begin(), r.std::u16string::end(), std::greater<>());
	return r;
}

// [https://stackoverflow.com/questions/32752739/python-how-does-the-functools-cmp-to-key-function-works <- google:‘c++ cmp_to_key’]
template <typename CmpFn> auto cmp_to_key(CmpFn mycmp)
{
	return [mycmp](const auto &obj) {
		class K
		{
			CmpFn cf;
			const TYPE_OF(obj) *o;

		public:
			K(CmpFn cf, const TYPE_OF(obj) *o) : cf(cf), o(o) {}

			bool operator<(const K &k) const {return cf(*o, *k.o) < 0;}
		} k(mycmp, &obj);

		return k;
	};
}

template <typename Type> inline Array<Type> reversed(const Array<Type> &arr)
{
	Array<Type> r(arr);
	std::reverse(r.begin(), r.end());
	return r;
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

template <typename Type> Type max_with_default(const Array<Type> &arr, const Type &default_)
{
	if (arr.empty())
		return default_;
	Type r = arr[0];
	for (auto i : arr)
		if (i > r) r = i;
	return r;
}

template <typename Iterable, typename Func> auto max_map(const Iterable &iterable, Func &&func)
{
	auto r = func(*std::begin(iterable));
	for (auto &&el : iterable) {
		auto i = func(el);
		if (i > r) r = i;
	}
	return r;
}

template <typename Type, typename Func, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
                                                                                 std::declval<decltype(tuple_element_f<Type, 1>())>()))> auto max_map(const Array<Type> &arr, Func &&func)
{
	auto r = func(_get<0>(*std::begin(arr)), _get<1>(*std::begin(arr)));
	for (auto &&el : arr) {
		auto i = func(_get<0>(el), _get<1>(el));
		if (i > r) r = i;
	}
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

template <typename Type> Type min_with_default(const Array<Type> &arr, const Type &default_)
{
	if (arr.empty())
		return default_;
	Type r = arr[0];
	for (auto i : arr)
		if (i < r) r = i;
	return r;
}

template <typename Iterable, typename Func> auto min_map(const Iterable &iterable, Func &&func)
{
	auto r = func(*std::begin(iterable));
	for (auto &&el : iterable) {
		auto i = func(el);
		if (i < r) r = i;
	}
	return r;
}

template <typename Type, typename Func, typename = decltype(std::declval<Func>()(std::declval<decltype(tuple_element_f<Type, 0>())>(),
                                                                                 std::declval<decltype(tuple_element_f<Type, 1>())>()))> auto min_map(const Array<Type> &arr, Func &&func)
{
	auto r = func(_get<0>(*std::begin(arr)), _get<1>(*std::begin(arr)));
	for (auto &&el : arr) {
		auto i = func(_get<0>(el), _get<1>(el));
		if (i < r) r = i;
	}
	return r;
}

template <class T, int N> Tvec<T,N> min(const Tvec<T,N> &a, const Tvec<T,N> &b) {return a < b ? a : b;}
template <class T, int N> Tvec<T,N> max(const Tvec<T,N> &a, const Tvec<T,N> &b) {return a > b ? a : b;}

template <typename Iterable, typename Key> auto min_with_key(const Iterable &iterable, Key &&key)
{
	std::remove_const_t<std::remove_reference_t<decltype(*std::begin(iterable))>> minitem = *std::begin(iterable);
	decltype(key(std::declval<decltype(*std::begin(iterable))>())) minval;
	bool first_iteration = true;
	for (auto &&el : iterable)
		if (first_iteration) {
			first_iteration = false;
			minitem = el;
			minval = key(el);
		}
		else {
			auto val = key(el);
			if (val < minval) {
				minval = val;
				minitem = el;
			}
		}
	return minitem;
}

template <typename Iterable, typename Key> auto max_with_key(const Iterable &iterable, Key &&key)
{
	std::remove_const_t<std::remove_reference_t<decltype(*std::begin(iterable))>> maxitem = *std::begin(iterable);
	decltype(key(std::declval<decltype(*std::begin(iterable))>())) maxval;
	bool first_iteration = true;
	for (auto &&el : iterable)
		if (first_iteration) {
			first_iteration = false;
			maxitem = el;
			maxval = key(el);
		}
		else {
			auto val = key(el);
			if (val > maxval) {
				maxval = val;
				maxitem = el;
			}
		}
	return maxitem;
}

// [https://stackoverflow.com/a/51077813/2692494 <- google:‘structured binding vector’]
template <class T, size_t N> struct array_binder {
	//const Array<T> &arr; // this does not work with `auto [a, b] = bind_array<2>(create_array({1, 2}));` because destructor of Array is called before `get()`
	T a[N];
	array_binder(const Array<T> &arr) { for (size_t i=0; i<N; i++) a[i] = arr[(Int)i]; }
	template <size_t I> const T &get() const { return a[I]; }
	template <size_t I>       T &get()       { return a[I]; }
};

namespace std {
	template <class T, size_t N> struct tuple_size<array_binder<T, N>> : std::integral_constant<size_t, N> { };
	template <size_t I, size_t N, class T> struct tuple_element<I, array_binder<T, N>> { using type = T; };
}

template <size_t N, class T> auto bind_array(const Array<T> &vec) {
	return array_binder<T, N>(vec);
}

inline Int int_from_bytes(const Array<Byte> &bytes)
{
	Int r = 0;
	if (bytes.len() > sizeof(Int))
		throw AssertionError();
	memcpy(&r, bytes.data(), bytes.len());
	return r;
}

inline Int int_from_bytes_be(const Array<Byte> &bytes)
{
	Int r = 0;
	if (bytes.len() > sizeof(Int))
		throw AssertionError();
	memcpy(&r, bytes.data(), bytes.len());
	for (int i=0; i<bytes.len(); i++)
		((char*)&r)[i] = bytes.data()[bytes.len() - 1 - i];
	return r;
}

template <typename Ty, bool include_beginning, bool include_ending> inline Int int_from_bytes(const Array<Byte> &bytes, const Range<Ty, include_beginning, include_ending> &range)
{
	Int r = 0;
	if (range.len() > sizeof(Int))
		throw AssertionError();
	memcpy(&r, bytes.data() + range.b, range.len());
	return r;
}

template <typename IntT> inline IntT int_t_from_bytes(const Array<Byte> &bytes)
{
	IntT r;
	if (bytes.len() != sizeof(IntT))
		throw AssertionError();
	memcpy(&r, bytes.data(), sizeof(IntT));
	return r;
}

template <typename IntT> inline IntT int_t_from_bytes_be(const Array<Byte> &bytes)
{
	IntT r;
	if (bytes.len() != sizeof(IntT))
		throw AssertionError();
	for (int i=0; i<sizeof(IntT); i++)
		((char*)&r)[i] = bytes.data()[sizeof(IntT) - 1 - i];
	return r;
}

template <typename IntT, typename Ty, bool include_beginning, bool include_ending> inline IntT int_t_from_bytes(const Array<Byte> &bytes, const Range<Ty, include_beginning, include_ending> &range)
{
	IntT r;
	if (range.len() != sizeof(IntT))
		throw AssertionError();
	memcpy(&r, bytes.data() + range.b, sizeof(IntT));
	return r;
}

template <typename IntT, typename Ty, bool include_beginning, bool include_ending> inline IntT int_t_from_bytes_be(const Array<Byte> &bytes, const Range<Ty, include_beginning, include_ending> &range)
{
	IntT r;
	if (range.len() != sizeof(IntT))
		throw AssertionError();
	for (int i=0; i<sizeof(IntT); i++)
		((char*)&r)[i] = bytes.data()[range.b + sizeof(IntT) - 1 - i];
	return r;
}

template <typename Ty> Array<Byte> bytes_from_int(const Ty &i)
{
	Array<Byte> r;
	r.resize(sizeof(i));
	memcpy(r.data(), &i, sizeof(i));
	return r;
}

inline Array<Byte> operator ""_B(const char *s, size_t sz)
{
	Array<Byte> r;
	r.resize(sz);
	memcpy(r.data(), s, sz);
	return r;
}


template <typename Type, size_t fix_len> class ArrayFixLen
{
	std::array<Type, fix_len> elements;
public:
	ArrayFixLen() {}
	ArrayFixLen(std::initializer_list<Type> il) {
		if (il.size() != fix_len)
			throw AssertionError();
		std::copy(il.begin(), il.end(), elements.begin());
	}

	auto begin() const {return elements.begin();}
	auto begin()       {return elements.begin();}
	auto end  () const {return elements.end  ();}
	auto end  ()       {return elements.end  ();}

	Int len() const {return fix_len;}

	decltype(std::declval<const std::vector<Type>>().at(0)) operator[](Int i) const // decltype is needed for Array<bool> support
	{
#ifdef PYTHON_NEGATIVE_INDEXING
		if (i < 0)
			i += len();
#endif
		if (in(i, range_el(Int(0), len())))
			return elements[i];
		throw IndexError(i);
	}

	decltype(std::declval<std::vector<Type>>().at(0)) operator[](Int i) // decltype is needed for Array<bool> support
	{
#ifdef PYTHON_NEGATIVE_INDEXING
		if (i < 0)
			i += len();
#endif
		if (in(i, range_el(Int(0), len())))
			return elements[i];
		throw IndexError(i);
	}

	const Type &at_plus_len(Int i) const
	{
#ifndef PYTHON_NEGATIVE_INDEXING
		i += len();
#endif
		return operator[](i);
	}

	Type &at_plus_len(Int i)
	{
#ifndef PYTHON_NEGATIVE_INDEXING
		i += len();
#endif
		return operator[](i);
	}

	const Type &set(Int i, const Type &v) // return `const Type&` for [https://www.rosettacode.org/wiki/Perlin_noise#Python]:‘p[256+i] = p[i] = permutation[i]’
	{
#ifdef PYTHON_NEGATIVE_INDEXING
		if (i < 0)
			i += len();
#endif
		if (in(i, range_el(Int(0), len())))
			return elements[i] = v;
		else
			throw IndexError(i);
	}

	const Type &set_plus_len(Int i, const Type &v)
	{
#ifndef PYTHON_NEGATIVE_INDEXING
		i += len();
#endif
		return set(i, v);
	}
};

template <size_t len, typename Type> auto create_array_fix_len(std::initializer_list<Type> il)
{
	return ArrayFixLen<Type, len>(il);
}

template <size_t len, typename Type> auto create_array_fix_len_el(const Type &el)
{
	ArrayFixLen<Type, len> r;
	for (auto &e : r)
		e = el;
	return r;
}
