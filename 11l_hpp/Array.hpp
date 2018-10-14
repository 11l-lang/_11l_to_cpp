#include <initializer_list>
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
			r.push_back(at(i));
		return r;
	}

public:
	Array() {}
    Array(std::initializer_list<Type> il) : vector(il) {}

	template <typename Func> auto map(Func &&func) -> Array<decltype(func(Type()))>
	{
		Array<decltype(func(Type()))> r;
		for (auto el : *this)
			r.push_back(func(el));
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

	template <typename Func> Array filter(Func &&func)
	{
		Array r;
		for (auto el : *this)
			if (func(el))
				r.push_back(el);
		return r;
	}

	int len() const {return size();}

	friend Array &&operator*(Array &&a, int n)
	{
		int len = a.len();
		if (n < 1) // mimic Python's behavior in which [1] * 0 = [] and [1] * -1 = []
			a.clear();
		else {
			a.reserve(len * n);
			for (int i = 1; i < n; i++)
				for (int j = 0; j < len; j++)
					a.append(a[j]);
		}
		return std::move(a);
	}

	Array operator[](const Range<int, true,  true > &range) const {return slice(max(range.b    , 0), min(range.e + 1, len()));}
	Array operator[](const Range<int, true,  false> &range) const {return slice(max(range.b    , 0), min(range.e    , len()));}
	Array operator[](const Range<int, false, true > &range) const {return slice(max(range.b + 1, 0), min(range.e + 1, len()));}
	Array operator[](const Range<int, false, false> &range) const {return slice(max(range.b + 1, 0), min(range.e    , len()));}
	Array operator[](const RangeEI<int>             &range) const {return slice(max(range.b    , 0),                  len() );}
	Array operator[](const RangeWithStep<int, true,  true > &range) const {return slice(max(range.b    , 0), min(range.e + 1, len()), range.step);}
	Array operator[](const RangeWithStep<int, true,  false> &range) const {return slice(max(range.b    , 0), min(range.e    , len()), range.step);}
	Array operator[](const RangeWithStep<int, false, true > &range) const {return slice(max(range.b + 1, 0), min(range.e + 1, len()), range.step);}
	Array operator[](const RangeWithStep<int, false, false> &range) const {return slice(max(range.b + 1, 0), min(range.e    , len()), range.step);}
	Array operator[](const RangeEIWithStep<int>             &range) const {return slice(max(range.b    , 0),                  len() , range.step);}

	const Type &operator[](int i) const
	{
		if (in(i, range_el(0, len())))
			return at(i);
		throw IndexError(i);
	}

	Type &operator[](int i)
	{
		if (in(i, range_el(0, len())))
			return at(i);
		throw IndexError(i);
	}

	const Type &at_plus_len(int i) const
	{
		return (*this)[len() + i];
	}

	void set(int i, const Type &v)
	{
		if (in(i, range_el(0, len())))
			at(i) = v;
		else
			throw IndexError(i);
	}

	void set_plus_len(int i, const Type &v)
	{
		set(len() + i, v);
	}

	void append(const Type &v) {push_back(v);}

	void append(const Array<Type> &arr)
	{
		reserve(size() + arr.size());
		for (auto el : arr)
			push_back(el);
	}

	int index(const Type &v) const
	{
		for (int i=0, l=len(); i<l; i++)
			if (data()[i] == v) return i;
		throw ValueError(v);
	}

	Type &last()
	{
		if (empty())
			throw IndexError(0);
		return at(len()-1);
	}

	Type pop()
	{
		if (empty())
			throw IndexError(0);
		Type r(std::move(at(len()-1)));
		resize(len()-1);
		return r;
	}
};

template <typename Type> Array<Type> create_array(std::initializer_list<Type> il)
{
    return Array<Type>(il);
}

template <typename Type> bool in(const Type &val, const Array<Type> &arr)
{
	return std::find(arr.begin(), arr.end(), val) != arr.end();
}

template <typename Type> Type sum(const Array<Type> &arr)
{
	Type r = 0;
	for (auto v : arr)
		r += v;
	return r;
}

inline Array<String> String::split(const String &delim)
{
	Array<String> arr;
	arr.reserve(count(delim) + 1);
	const char16_t *str = data(), *begin = str, *end = data() + len() - delim.len() + 1;
	while (str < end)
		if (memcmp(str, delim.data(), delim.len()*sizeof(char16_t)) == 0)
		{
			arr.append(String(begin, str-begin));
			str += delim.len();
			begin = str;
		}
		else
			str++;

	arr.append(String(begin, str-begin));
	return arr;
}
