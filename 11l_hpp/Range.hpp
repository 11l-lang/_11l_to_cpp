template <typename Type> class Array;
template <typename Type, bool include_beginning, bool include_ending> class RangeWithStep;

template <typename Type, bool include_beginning, bool include_ending> class Range
{
public:
	Type b, e;

	Range(const Type &begin, const Type &end) : b(begin), e(end) {}

	class Iterator
	{
		Type value;
	public:
		explicit Iterator(Type value) : value(value) {}
		bool operator==(Iterator i) {return value == i.value;}
		bool operator!=(Iterator i) {return value != i.value;}
		void operator++() {++value;}
		Type operator*() {return value;}
	};
	Iterator begin() const {return Iterator(b + (int)!include_beginning);}
	Iterator end()   const {return Iterator(max(e - (int)!include_ending + 1, b + (int)!include_beginning));}

	Type size() const
	{
		if (include_beginning && include_ending)
			return e - b + 1;
		if (include_beginning && !include_ending)
			return e - b;
		if (!include_beginning && include_ending)
			return e - b;
		if (!include_beginning && !include_ending)
			return e - b - 1;
	}

	Type len() const
	{
		return size();
	}

	template <typename Func> auto map(Func &&func) const -> Array<decltype(func(std::declval<Type>()))>
	{
		Array<decltype(func(std::declval<Type>()))> r;
		r.reserve(e - !include_ending - (b + !include_beginning) + 1);
		for (Type i = b + !include_beginning; i <= e - !include_ending; i++)
			r.push_back(func(i));
		return r;
	}

	template <typename Func> Array<Type> filter(Func &&func) const
	{
		Array<Type> r;
		for (auto &&el : *this)
			if (func(el))
				r.push_back(el);
		return r;
	}

	template <typename Func> Type reduce(Func &&func) const
	{
		auto it = begin();
		if (it == end())
			throw AssertionError();
		Type r = *it;
		for (++it; it != end(); ++it)
			r = func(r, *it);
		return r;
	}

	template <typename Ty, typename Func> Ty reduce(const Ty &initial, Func &&func) const
	{
		Ty r = initial;
		for (auto &&el : *this)
			r = func(r, el);
		return r;
	}

	RangeWithStep<Type, include_beginning, include_ending> step(Type step)
	{
		return RangeWithStep<Type, include_beginning, include_ending>(b, e, step);
	}
};

template <typename Type, bool include_beginning, bool include_ending> class RangeWithStep
{
public:
	Type b, e, step;

	RangeWithStep(const Type &begin, const Type &end, const Type &step) : b(begin), e(end), step(step) {}

	class Iterator
	{
		Type value, step;
	public:
		Iterator(Type value, Type step) : value(value), step(step) {}
		bool operator!=(Iterator i) {return !include_ending ? (step < 0 ? value >  i.value : value <  i.value) // I know this is hack, but it is faster than precise computation of end() {for step > 0: `end = include_ending ? e + step - (e - b) %  step : e-1 + step - (e-1 - b) %  step`,
			                                                : (step < 0 ? value >= i.value : value <= i.value);} //                                                                        for step < 0: `end = include_ending ? e + step + (b - e) % -step : e+1 + step + (b - e-1) % -step`}
		void operator++() {value += step;}
		Type operator*() {return value;}
	};
	Iterator begin() const {return Iterator(b + !include_beginning, step);}
	Iterator end()   const {return Iterator(e                     , step);}

	Type size() const
	{
		return (Range<Type, include_beginning, include_ending>(b, e).size() + step - 1) / step;
	}
	Type len() const
	{
		return size();
	}

	template <typename Func> auto map(Func &&func) -> Array<decltype(func(std::declval<Type>()))>
	{
		Array<decltype(func(std::declval<Type>()))> r;
		for (Type i = b + !include_beginning; i <= e - !include_ending; i+=step)
			r.push_back(func(i));
		return r;
	}
};

template <typename Type> class RangeEIWithStep
{
public:
	Type b, step;

	RangeEIWithStep(const Type &begin, const Type &step) : b(begin), step(step) {}
};

template <typename Type> class RangeEI
{
public:
	Type b;

	explicit RangeEI(const Type &begin) : b(begin) {}

	class Iterator
	{
		Type value;
	public:
		explicit Iterator(Type value) : value(value) {}
		bool operator==(Iterator i) {return value == i.value;}
		bool operator!=(Iterator i) {return value != i.value;}
		void operator++() {++value;}
		Type operator*() {return value;}
	};
	Iterator begin() const {return Iterator(b);}
	Iterator end  () const {return Iterator(std::numeric_limits<Type>::max());}

	RangeEIWithStep<Type> step(Type step)
	{
		return RangeEIWithStep<Type>(b, step);
	}
};

struct range_e_llen // `a.<(len)-b`
{
	int b, e;
	range_e_llen(int b, int e) : b(b), e(e) {}
};
struct range_elen_elen // `(len)-a..(len)-b`
{
	int b, e;
	range_elen_elen(int b, int e) : b(b), e(e) {}
};
struct range_elen_llen // `(len)-a.<(len)-b`
{
	int b, e;
	range_elen_llen(int b, int e) : b(b), e(e) {}
};
struct range_elen_i_wstep // `((len)-a..).step(s)`
{
	int b, s;
	range_elen_i_wstep(int b, int s) : b(b), s(s) {}
};
struct range_elen_i // `(len)-a..`
{
	int b;
	range_elen_i(int b) : b(b) {}
	range_elen_i_wstep step(int s) {return range_elen_i_wstep(b, s);}
};

template <typename Ty> inline auto range_ee(const Ty &begin, const Ty &end) {return Range<Ty, true,  true >(begin, end);} // equal-equal range (`a..b`)
template <typename Ty> inline auto range_el(const Ty &begin, const Ty &end) {return Range<Ty, true,  false>(begin, end);} // equal-less  range (`a.<b`)
template <typename Ty> inline auto range_ep(const Ty &begin, const Ty &len) {return range_el(begin, begin + len);}        // equal-plus  range (`a.+b`)
template <typename Ty> inline auto range_le(const Ty &begin, const Ty &end) {return Range<Ty, false, true >(begin, end);} // less-equal  range (`a<.b`)
template <typename Ty> inline auto range_ll(const Ty &begin, const Ty &end) {return Range<Ty, false, false>(begin, end);} // less-less   range (`a<.<b`)
template <typename Ty> inline auto range_ei(const Ty &begin               ) {return RangeEI<Ty>            (begin     );} // equal-infinity range (`a..`)

template <typename Ty> inline bool in(const Ty &val, const Range<Ty, true,  true > &range) {return val >= range.b && val <= range.e;}
template <typename Ty> inline bool in(const Ty &val, const Range<Ty, true,  false> &range) {return val >= range.b && val <  range.e;}
template <typename Ty> inline bool in(const Ty &val, const Range<Ty, false, true > &range) {return val >  range.b && val <= range.e;}
template <typename Ty> inline bool in(const Ty &val, const Range<Ty, false, false> &range) {return val >  range.b && val <  range.e;}
template <> inline bool in(const int &val, const Range<int, true,  true > &range) {return unsigned(val - range.b) <= unsigned(range.e - range.b);}
template <> inline bool in(const int &val, const Range<int, true,  false> &range) {return unsigned(val - range.b) <  unsigned(range.size());}
template <> inline bool in(const int &val, const Range<int, false, true > &range) {return unsigned(val - range.b - 1) < unsigned(range.e - range.b);}
template <> inline bool in(const int &val, const Range<int, false, false> &range) {return unsigned(val - range.b - 1) < unsigned(range.size());}

template <typename Ty> inline bool in(const Ty &val, const RangeEI<Ty> &range) {return val >= range.b;}

template <typename Type, int n> bool in(const Type &val, const Tvec<Type, n> &v)
{
	for (int i=0; i<n; i++)
		if (v[i] == val) return true;
	return false;
}

// Based on [http://artlang.net/post/c++11-obkhod-elementov-kortezhe-std-tuple/ <- google:‘c++ tuple’]
namespace _detail_
{
	template <typename Type, bool include_beginning, bool include_ending> bool in_or_equals(const Type &val, const Range<Type, include_beginning, include_ending> &range)
	{
		return in(val, range);
	}
//	template <typename Type> bool in_or_equals(const Type &val, const Type &val2)
//	{
//		return val == val2;
//	}
	template <typename Type, typename Type2> bool in_or_equals(const Type &val, const Type2 &val2) // for comparison between Char and String
	{
		return val == val2;
	}

	template <int index, typename ValueType, typename ... Types> struct iterate_tuple
	{
		static bool next(const ValueType &val, const Tuple<Types...> &t)
		{
			return in_or_equals(val, std::get<index>(t)) || iterate_tuple<index - 1, ValueType, Types...>::next(val, t);
		}
	};

	template <typename ValueType, typename ... Types> struct iterate_tuple<0, ValueType, Types...>
	{
		static bool next(const ValueType &val, const Tuple<Types...> &t)
		{
			return in_or_equals(val, std::get<0>(t));
		}
	};
}

template <typename ValueType, typename ... Types> inline bool in(const ValueType &val, const Tuple<Types...> &ranges)
{
	return _detail_::iterate_tuple<std::tuple_size<Tuple<Types...>>::value - 1, ValueType, Types...>::next(val, ranges);
}
