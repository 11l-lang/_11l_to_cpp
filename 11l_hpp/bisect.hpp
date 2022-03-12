// [https://github.com/python/cpython/blob/main/Lib/bisect.py <- google:‘bisect cpython github’]

namespace bisect
{
template <typename Type> Int right(const Array<Type> &a, const Type &x, Int lo, Int hi)
{
	if (lo < 0)
		throw ValueError(u"lo must be non-negative"_S);

	while (lo < hi) {
		Int mid = (lo + hi) / 2;
		if (x < a[mid])
			hi = mid;
		else
			lo = mid + 1;
	}

	return lo;
}
template <typename Type> Int right(const Array<Type> &a, const Type &x, Int lo = 0)
{
	return right(a, x, lo, a.len());
}

template <typename Type> Int left(const Array<Type> &a, const Type &x, Int lo, Int hi)
{
	if (lo < 0)
		throw ValueError(u"lo must be non-negative"_S);

	while (lo < hi) {
		Int mid = (lo + hi) / 2;
		if (a[mid] < x)
			lo = mid + 1;
		else
			hi = mid;
	}

	return lo;
}
template <typename Type> Int left(const Array<Type> &a, const Type &x, Int lo = 0)
{
	return left(a, x, lo, a.len());
}
}
