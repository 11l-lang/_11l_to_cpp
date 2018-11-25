namespace randomns // GCC: .../11l-lang/_11l_to_cpp/11l_hpp/random.hpp:1:11: error: ‘namespace random { }’ redeclared as different kind of symbol
{
static unsigned int seed = 0;

inline unsigned int _random()
{
	return seed = 1664525 * seed + 1013904223;
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
}
