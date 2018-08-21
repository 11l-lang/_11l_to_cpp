template <typename Ty> class Array;
template <typename Type, bool include_beginning, bool include_ending> class RangeWithStep;

template <typename Type, bool include_beginning, bool include_ending> class Range
{
public:
	Type begin, end;

	Range(const Type &begin, const Type &end) : begin(begin), end(end) {}

	Type size() const
	{
		if (include_beginning && include_ending)
			return end - begin + 1;
		if (include_beginning && !include_ending)
			return end - begin;
		if (!include_beginning && include_ending)
			return end - begin;
		if (!include_beginning && !include_ending)
			return end - begin - 1;
	}

	template <typename Func> auto map(Func &&func) -> Array<decltype(func(Type()))>
	{
		Array<decltype(func(Type()))> r;
		for (Type i = begin + !include_beginning; i <= end - !include_ending; i++)
			r.push_back(func(i));
		return r;
	}

	RangeWithStep<Type, include_beginning, include_ending> step(Type step)
	{
		return RangeWithStep<Type, include_beginning, include_ending>(begin, end, step);
	}
};

template <typename Type, bool include_beginning, bool include_ending> class RangeWithStep
{
public:
	Type begin, end, step;

	RangeWithStep(const Type &begin, const Type &end, const Type &step) : begin(begin), end(end), step(step) {}

	template <typename Func> auto map(Func &&func) -> Array<decltype(func(Type()))>
	{
		Array<decltype(func(Type()))> r;
		for (Type i = begin + !include_beginning; i <= end - !include_ending; i+=step)
			r.push_back(func(i));
		return r;
	}
};

template <typename Type> class RangeEI
{
public:
	Type begin;

	explicit RangeEI(const Type &begin) : begin(begin) {}
};

template <typename Ty> auto range_ee(const Ty &begin, const Ty &end) {return Range<Ty, true,  true >(begin, end);} // equal-equal range (`a..b`)
template <typename Ty> auto range_el(const Ty &begin, const Ty &end) {return Range<Ty, true,  false>(begin, end);} // equal-less  range (`a.<b`)
template <typename Ty> auto range_le(const Ty &begin, const Ty &end) {return Range<Ty, false, true >(begin, end);} // less-equal  range (`a<.b`)
template <typename Ty> auto range_ll(const Ty &begin, const Ty &end) {return Range<Ty, false, false>(begin, end);} // less-less   range (`a<.<b`)
template <typename Ty> auto range_ei(const Ty &begin               ) {return RangeEI<Ty>            (begin     );} // equal-infinity range (`a..`)

template <typename Ty> bool in(const Ty &val, const Range<Ty, true,  true > &range) {return val >= range.begin && val <= range.end;}
template <typename Ty> bool in(const Ty &val, const Range<Ty, true,  false> &range) {return val >= range.begin && val <  range.end;}
template <typename Ty> bool in(const Ty &val, const Range<Ty, false, true > &range) {return val >  range.begin && val <= range.end;}
template <typename Ty> bool in(const Ty &val, const Range<Ty, false, false> &range) {return val >  range.begin && val <  range.end;}
template <> bool in(const int &val, const Range<int, true,  true > &range) {return unsigned(val - range.begin) <= unsigned(range.end - range.begin);}
template <> bool in(const int &val, const Range<int, true,  false> &range) {return unsigned(val - range.begin) <  unsigned(range.size());}
template <> bool in(const int &val, const Range<int, false, true > &range) {return unsigned(val - range.begin - 1) < unsigned(range.end - range.begin);}
template <> bool in(const int &val, const Range<int, false, false> &range) {return unsigned(val - range.begin - 1) < unsigned(range.size());}

template <typename Ty> bool in(const Ty &val, const RangeEI<Ty> &range) {return val >= range.begin;}
