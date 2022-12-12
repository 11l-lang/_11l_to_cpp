template <typename T1> int popcount(const T1 &x)
{
    return bin(x).count(u'1'_C);
}

BigInt sqrtb(const BigInt &x)
{
    assert(x >= 0);
    BigInt i = 1;
    while (i * i <= x)
        i *= 2;
    BigInt y = 0;
    while (i > 0) {
        if (square((y + i)) <= x)
            y += i;
        i = idiv(i, 2);
    }
    return y;
}

template <typename T1> auto is_square(const T1 &x)
{
    if (x < 0)
        return false;
    auto y = to_int(sqrt(x));
    return y * y == x;
}

auto is_prime(const int x)
{
    if (x <= 1)
        return false;
    else if (x <= 3)
        return true;
    else if (mod(x, 2) == 0)
        return false;
    else {
        for (auto i : range_ee(3, to_int(sqrt(x))).step(2))
            if (mod(x, i) == 0)
                return false;
        return true;
    }
}

template <typename T1> auto list_primality(const T1 &n)
{
    auto result = create_array({true}) * (n + 1);
    _set<0>(result, _set<1>(result, false));
    for (auto i : range_ee(0, to_int(sqrt(n))))
        if (result[i])
            for (auto j : range_el(i * i, result.len()).step(i))
                result.set(j, false);
    return result;
}

template <typename T1> auto list_primes(const T1 &n)
{
    return enumerate(list_primality(n)).filter([](const auto &i, const auto &isprime){return isprime;}).map([](const auto &i, const auto &isprime){return i;});
}

auto list_smallest_prime_factors(const int n)
{
    auto result = create_array({0}) * (n + 1);
    auto limit = to_int(sqrt(n));
    for (auto i : range_el(2, result.len()))
        if (result[i] == 0) {
            result.set(i, i);
            if (i <= limit)
                for (auto j : range_ee(i * i, n).step(i))
                    if (result[j] == 0)
                        result.set(j, i);
        }
    return result;
}

template <typename T1> auto list_totients(const T1 &n)
{
    auto result = create_array(range_ee(0, n));
    for (auto i : range_el(2, result.len()))
        if (result[i] == i)
            for (auto j : range_el(i, result.len()).step(i))
                result[j] -= idiv(result[j], i);
    return result;
}

auto binomial(const BigInt &n, const BigInt &k)
{
    assert(in(k, range_ee(BigInt(0), n)));
    return idiv(factorial(n), (factorial(k) * factorial(n - k)));
}

int reciprocal_mod(int x, const int m)
{
    assert(in(x, range_el(0, m)));

    int y = x;
    x = m;
    int a = 0;
    int b = 1;
    while (y != 0) {
        assign_from_tuple(a, b, make_tuple(b, a - idiv(x, y) * b));
        assign_from_tuple(x, y, make_tuple(y, mod(x, y)));
    }
    if (x == 1)
        return a >= 0 ? a : a + m;
    else
        throw ValueError(u"Reciprocal does not exist"_S);
}
