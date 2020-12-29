#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

template <typename T1, typename T2, typename T3> auto randomMatrix(const T1 &n, const T2 &upperBound, const T3 &seed)
{
    randomns::seed(seed);
    Array<Array<double>> m;
    for (auto r : range_el(0, n)) {
        Array<double> sm;
        for (auto c : range_el(0, n))
            sm.append(upperBound * randomns::_());
        m.append(sm);
    }
    return m;
}

template <typename T1> auto wrappedPath(const T1 &path)
{
    return path[range_ei(1)] + create_array({_get<0>(path)});
}

template <typename T1, typename T2> auto pathLength(const T1 &cities, const T2 &path)
{
    auto pairs = zip(path, wrappedPath(path));
    return sum_map(pairs, [&cities](const auto &r, const auto &c){return cities[r][c];});
}

template <typename T2, typename T3> auto updatePher(Array<Array<double>> &pher, const T2 &path, const T3 &boost)
{
    auto pairs = zip(path, wrappedPath(path));
    for (auto &&[r, c] : pairs)
        pher[r].set(c, pher[r][c] + boost);
}

template <typename T2, typename T3> auto evaporatePher(Array<Array<double>> &pher, const T2 &maxIter, const T3 &boost)
{
    auto decr = boost / to_float(maxIter);
    for (auto r : range_el(0, pher.len()))
        for (auto c : range_el(0, pher[r].len()))
            if (pher[r][c] > decr)
                pher[r].set(c, pher[r][c] - decr);
            else
                pher[r].set(c, 0.0);
}

template <typename T1, typename T2, typename T3, typename T4> auto doSumWeight(const T1 &cities, const T2 &pher, const T3 &used, const T4 &current)
{
    auto runningTotal = 0.0;
    for (auto city : range_el(0, cities.len()))
        if (!in(city, used))
            runningTotal = (runningTotal + cities[current][city] * (1.0 + pher[current][city]));
    return runningTotal;
}

template <typename T1, typename T2, typename T3, typename T4, typename T5> auto findSumWeight(const T1 &cities, const T2 &pher, const T3 &used, const T4 &current, const T5 &soughtTotal)
{
    auto runningTotal = 0.0;
    auto next = 0;
    for (auto city : range_el(0, cities.len())) {
        if (runningTotal >= soughtTotal)
            break;
        if (!in(city, used)) {
            runningTotal = (runningTotal + cities[current][city] * (1.0 + pher[current][city]));
            next = city;
        }
    }
    return next;
}

template <typename T1, typename T2> auto genPath(const T1 &cities, const T2 &pher)
{
    auto current = randomns::_(range_ee(0, cities.len() - 1));
    auto path = create_array({current});
    auto used = create_dict(dict_of(current, 1));
    while (used.len() < cities.len()) {
        auto sumWeight = doSumWeight(cities, pher, used, current);
        auto rndValue = randomns::_() * sumWeight;
        current = findSumWeight(cities, pher, used, current, rndValue);
        path.append(current);
        used.set(current, 1);
    }
    return path;
}

template <typename T1, typename T2, typename T3, typename T4> auto bestPath(const T1 &cities, const T2 &seed, const T3 &maxIter, const T4 &boost)
{
    auto pher = randomMatrix(cities.len(), 0, 0);
    randomns::seed(seed);
    auto bestLen = 0.0;
    Array<int> bestPath;
    for (auto iter : range_el(0, maxIter)) {
        auto path = genPath(cities, pher);
        auto pathLen = pathLength(cities, path);
        if (pathLen > bestLen) {
            updatePher(pher, path, boost);
            bestLen = pathLen;
            bestPath = path;
        }
        evaporatePher(pher, maxIter, boost);
    }
    return bestPath;
}

int main()
{
    auto seed = 1;
    auto boost = 5;
    auto iter = 1000;
    auto numCities = 200;
    auto maxDistance = 100;
    auto cityDistanceSeed = 1;
    print(u"starting"_S);
    auto cities = randomMatrix(numCities, maxDistance, cityDistanceSeed);
    auto path = bestPath(cities, seed, iter, boost);
    print(path);
    print(u"len = "_S & String(pathLength(cities, path)));
}
