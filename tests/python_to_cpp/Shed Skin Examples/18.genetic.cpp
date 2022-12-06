#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

auto infiniteNeg = -1e302;

class Individual
{
public:
    int ngenes;
    Array<Byte> genome;
    decltype(::infiniteNeg) fitness = ::infiniteNeg;
    template <typename T1 = decltype(0)> Individual(const T1 &ngenes = 0) :
        ngenes(ngenes)
    {
        genome = range_el(0, ngenes).map([](const auto &i){return Byte(randomns::_() < 0.5);});
    }
    template <typename T1 = decltype(0), typename T2 = decltype(0)> auto bin2dec(const T1 &inf = 0, T2 sup = 0)
    {
        if (sup == 0)
            sup = ngenes - 1;
        auto result = 0;
        for (auto i : range_ee(inf, sup))
            if (genome[i])
                result += 1 << (i - inf);
        return result;
    }
    auto computeFitness()
    {
        fitness = fitnessFun(computeValuesGenome());
    }
    auto __repr__()
    {
        return (genome.map([](const auto &gene){return String(to_int(gene));})).join(u""_S);
    }

    template <typename T1> auto fitnessFun(const T1 &x)
    {
        return x + abs(sin(32 * x));
    }
    template <typename T1 = decltype(0), typename T2 = decltype(math::pi)> auto computeValuesGenome(const T1 &xMin = 0, const T2 &xMax = math::pi)
    {
        auto scaleFactor = (xMax - xMin) / (1 << ngenes);
        return bin2dec() * scaleFactor;
    }
};

class SGA
{
public:
    Array<Individual> population;
    Individual bestIndividual;
    decltype(200) popSize = 200;
    decltype(16) genomeSize = 16;
    decltype(16) generationsMax = 16;
    decltype(0.75) crossingOverProb = 0.75;
    decltype(0.75) selectivePressure = 0.75;
    decltype(0.005) geneMutationProb = 0.005;

    SGA()
    {
    }

    auto generateRandomPop()
    {
        population = range_el(0, popSize).map([this](const auto &i){return Individual(genomeSize);});
    }

    auto computeFitnessPop()
    {
        for (auto &&individual : population)
            individual.computeFitness();
    }

    auto mutatePop()
    {
        auto nmutations = to_int(round(popSize * genomeSize * geneMutationProb));
        for (auto i : range_el(0, nmutations)) {
            auto individual = randomns::choice(population);
            auto gene = randomns::_(range_ee(0, genomeSize - 1));
            individual.genome.set(gene, !individual.genome[gene]);
        }
    }

    auto tounamentSelectionPop()
    {
        Array<Individual> pop2;
        for (auto i : range_el(0, popSize)) {
            auto individual1 = randomns::choice(population);
            auto individual2 = randomns::choice(population);
            if (randomns::_() < selectivePressure) {
                if (individual1.fitness > individual2.fitness)
                    pop2.append(individual1);
                else
                    pop2.append(individual2);
            }
            else
                if (individual1.fitness > individual2.fitness)
                    pop2.append(individual2);
                else
                    pop2.append(individual1);
        }
        return pop2;
    }

    auto crossingOverPop()
    {
        auto nCrossingOver = to_int(round(popSize * crossingOverProb));
        for (auto i : range_el(0, nCrossingOver)) {
            auto ind1 = randomns::choice(population);
            auto ind2 = randomns::choice(population);
            auto crossPosition = randomns::_(range_ee(0, genomeSize - 1));
            for (auto j : range_ee(0, crossPosition))
                swap(ind1.genome[j], ind2.genome[j]);
        }
    }

    auto showGeneration_bestIndFind()
    {
        auto fitnessTot = 0.0;
        auto bestIndividualGeneration = _get<0>(population);
        for (auto &&individual : population) {
            fitnessTot += individual.fitness;
            if (individual.fitness > bestIndividualGeneration.fitness)
                bestIndividualGeneration = individual;
        }
        if (bestIndividual.fitness < bestIndividualGeneration.fitness)
            bestIndividual = copy(bestIndividualGeneration);
    }

    auto run()
    {
        generateRandomPop();
        bestIndividual = Individual(genomeSize);
        for (auto generation : range_ee(1, generationsMax)) {
            if (mod(generation, 300) == 0)
                print(u"generation "_S & String(generation));
            computeFitnessPop();
            showGeneration_bestIndFind();
            population = tounamentSelectionPop();
            mutatePop();
            crossingOverPop();
        }
    }
};

int main()
{
    auto sga = SGA();
    sga.generationsMax = 3000;
    sga.genomeSize = 20;
    sga.popSize = 30;
    sga.geneMutationProb = 0.01;
    sga.run();
}
