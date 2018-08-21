template <typename Ty> class Array;
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
		bool operator!=(Iterator i) {return value != i.value;}
		void operator++() {value++;}
		Type operator*() {return value;}
	};
	Iterator begin() const {return Iterator(b + !include_beginning);}
	Iterator end()   const {return Iterator(e - !include_ending + 1);}

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

	template <typename Func> auto map(Func &&func) -> Array<decltype(func(Type()))>
	{
		Array<decltype(func(Type()))> r;
		for (Type i = b + !include_beginning; i <= e - !include_ending; i++)
			r.push_back(func(i));
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
		explicit Iterator(Type value, Type step) : value(value), step(step) {}
		bool operator!=(Iterator i) {return value < i.value;} // I know this is hack, but it is faster than precise computation of end()
		void operator++() {value += step;}
		Type operator*() {return value;}
	};
	Iterator begin() const {return Iterator(b + !include_beginning,  step);}
	Iterator end()   const {return Iterator(e - !include_ending + 1, step);}

	template <typename Func> auto map(Func &&func) -> Array<decltype(func(Type()))>
	{
		Array<decltype(func(Type()))> r;
		for (Type i = b + !include_beginning; i <= e - !include_ending; i+=step)
			r.push_back(func(i));
		return r;
	}
};

template <typename Type> class RangeEI
{
public:
	Type b;

	explicit RangeEI(const Type &begin) : b(begin) {}
};

template <typename Ty> auto range_ee(const Ty &begin, const Ty &end) {return Range<Ty, true,  true >(begin, end);} // equal-equal range (`a..b`)
template <typename Ty> auto range_el(const Ty &begin, const Ty &end) {return Range<Ty, true,  false>(begin, end);} // equal-less  range (`a.<b`)
template <typename Ty> auto range_le(const Ty &begin, const Ty &end) {return Range<Ty, false, true >(begin, end);} // less-equal  range (`a<.b`)
template <typename Ty> auto range_ll(const Ty &begin, const Ty &end) {return Range<Ty, false, false>(begin, end);} // less-less   range (`a<.<b`)
template <typename Ty> auto range_ei(const Ty &begin               ) {return RangeEI<Ty>            (begin     );} // equal-infinity range (`a..`)

template <typename Ty> bool in(const Ty &val, const Range<Ty, true,  true > &range) {return val >= range.b && val <= range.e;}
template <typename Ty> bool in(const Ty &val, const Range<Ty, true,  false> &range) {return val >= range.b && val <  range.e;}
template <typename Ty> bool in(const Ty &val, const Range<Ty, false, true > &range) {return val >  range.b && val <= range.e;}
template <typename Ty> bool in(const Ty &val, const Range<Ty, false, false> &range) {return val >  range.b && val <  range.e;}
template <> bool in(const int &val, const Range<int, true,  true > &range) {return unsigned(val - range.b) <= unsigned(range.e - range.b);}
template <> bool in(const int &val, const Range<int, true,  false> &range) {return unsigned(val - range.b) <  unsigned(range.size());}
template <> bool in(const int &val, const Range<int, false, true > &range) {return unsigned(val - range.b - 1) < unsigned(range.e - range.b);}
template <> bool in(const int &val, const Range<int, false, false> &range) {return unsigned(val - range.b - 1) < unsigned(range.size());}

template <typename Ty> bool in(const Ty &val, const RangeEI<Ty> &range) {return val >= range.b;}
