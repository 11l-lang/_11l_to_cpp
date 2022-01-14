template <typename T1> auto is_prime(const T1 &x)
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

auto binomial(const BigInt &n, const BigInt &k)
{
    assert(in(k, range_ee(BigInt(0), n)));
    return idiv(factorial(n), (factorial(k) * factorial(n - k)));
}
