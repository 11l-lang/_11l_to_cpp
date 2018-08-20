#include <initializer_list>
#include <vector>

template <typename Ty> class Array : public std::vector<Ty>
{
	Array slice(int begin, int end) const
	{
		Array r;
		for (int i = begin; i < end; i++)
			r.push_back(at(i));
		return r;
	}

public:
	Array() {}
    Array(std::initializer_list<Ty> il) : vector(il) {}

	template <typename Func> auto map(Func &&func) -> Array<decltype(func(Ty()))>
	{
		Array<decltype(func(Ty()))> r;
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

	Array operator[](const Range<int, true,  true > &range) const {return slice(max(range.begin    , 0), min(range.end + 1, len()));}
	Array operator[](const Range<int, true,  false> &range) const {return slice(max(range.begin    , 0), min(range.end    , len()));}
	Array operator[](const Range<int, false, true > &range) const {return slice(max(range.begin + 1, 0), min(range.end + 1, len()));}
	Array operator[](const Range<int, false, false> &range) const {return slice(max(range.begin + 1, 0), min(range.end    , len()));}
	Array operator[](const RangeEI<int>             &range) const {return slice(max(range.begin    , 0),                    len() );}

	const Ty &operator[](int i) const {return at(i);}
};

template <class Ty> Array<Ty> create_array(std::initializer_list<Ty> il)
{
    return Array<Ty>(il);
}
