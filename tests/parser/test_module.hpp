template <typename T1 = decltype(0)> auto _(const T1 &a = 0)
{
    print(a);
}

template <typename T1 = decltype(0)> auto f(const T1 &inner_name = 0)
{
    return inner_name;
}
