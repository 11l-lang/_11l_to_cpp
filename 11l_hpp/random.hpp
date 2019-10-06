namespace randomns // GCC: .../11l-lang/_11l_to_cpp/11l_hpp/random.hpp:1:11: error: ‘namespace random { }’ redeclared as different kind of symbol
{
static unsigned int seed_ = 0;

inline void seed(unsigned int s)
{
	seed_ = s;
}

inline unsigned int _random()
{
	return seed_ = 1664525 * seed_ + 1013904223;
}

inline double _(double max = 1.0)
{
	return _random()*(max/double(0x1'0000'0000));
}

inline float _(float max)
{
	return _random()*(max/float(0x1'0000'0000));
}

inline int _(int max)
{
	return ((unsigned long long)_random() * max) >> 32u;
}

inline int _(int min, int max)
{
	return _(max - min) + min;
}

inline double _(double min, double max)
{
	return _(max - min) + min;
}

inline float _(float min, float max)
{
	return _(max - min) + min;
}

inline int _(const Range<int, true, false> range) {return _(range.b, range.e);}
inline int _(const Range<int, true, true > range) {return _(range.b, range.e + 1);}

template <typename Type> inline void shuffle(Array<Type> &arr)
{
	for (int i=0,l=arr.len(); i<l; i++)
		std::swap(arr[_(l)], arr[_(l)]);
}

template <typename Type> inline auto choice(Array<Type> &arr)
{
	return arr[_(arr.len())];
}
}
