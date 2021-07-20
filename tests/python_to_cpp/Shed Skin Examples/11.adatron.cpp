#include "C:\!!BITBUCKET\11l-lang\_11l_to_cpp\11l.hpp"

auto CYTOSOLIC = 0;
auto EXTRACELLULAR = 1;
auto NUCLEAR = 2;
auto MITOCHONDRIAL = 3;
auto BLIND = 4;
auto D = 5.0;
auto LENGTH = 50;
auto AMINOACIDS = u"ACDEFGHIKLMNPQRSTVWY"_S;

class Protein
{
public:
    String name;
    String mass;
    String isoelectric_point;
    String size;
    String sequence;
    int type_id;
    Dict<Char, double> local_composition;
    Dict<Char, double> global_composition;

    template <typename T1, typename T2, typename T3, typename T4, typename T5, typename T6> Protein(const T1 &name, const T2 &mass, const T3 &isoelectric_point, const T4 &size, const T5 &sequence, const T6 &type_id) :
        name(name),
        mass(mass),
        isoelectric_point(isoelectric_point),
        size(size),
        sequence(sequence),
        type_id(type_id)
    {
        extract_composition();
    }

    auto extract_composition()
    {
        local_composition = create_dict((::AMINOACIDS.map([](const auto &x){return make_tuple(x, 0.0);})));
        for (auto counter : range_el(0, ::LENGTH))
            local_composition[sequence[counter]] += 1.0 / ::LENGTH;
        global_composition = create_dict((::AMINOACIDS.map([](const auto &x){return make_tuple(x, 0.0);})));
        for (auto &&aminoacid : sequence)
            global_composition[aminoacid] += 1.0 / sequence.len();
    }

    auto create_vector()
    {
        Array<double> vector;
        auto last_value = 0.0;
        for (auto &&[key, value] : sorted(local_composition.items())) {
            vector.append(value);
            last_value = value;
        }
        for (auto &&key : sorted(global_composition.keys()))
            vector.append(last_value);
        return vector;
    }
};
Array<Protein> PROTEINS;

template <typename T1, typename T2> auto load_file(const T1 &filename, const T2 &type_id)
{
    auto protfile = File(filename);
    for (auto &&line : protfile.read_lines(true)) {
        if (line.starts_with(u"name"_S))
            continue;
        auto [name, mass, isoelectric_point, size, sequence] = bind_array<5>(line.trim(make_tuple(u" "_S, u"\t"_S, u"\r"_S, u"\n"_S)).split(u"\t"_S));
        auto protein = Protein(name, mass, isoelectric_point, size, sequence, type_id);
        ::PROTEINS.append(protein);
    }
    protfile.close();
}

auto create_tables()
{
    u"Create the feature and label tables."_S;
    Array<Array<double>> feature_table;
    Array<Array<int>> label_table;
    for (auto &&protein : ::PROTEINS)
        feature_table.append(protein.create_vector());

    for (auto &&protein : ::PROTEINS) {
        if (protein.type_id == ::BLIND)
            continue;
        auto labels = create_array({-1}) * 4;
        labels[protein.type_id] *= -1;
        label_table.append(labels);
    }
    return make_tuple(feature_table, label_table);
}

template <typename T1> auto create_kernel_table(const T1 &feature_table)
{
    Array<Array<double>> kernel_table;
    for (auto &&row : feature_table) {
        Array<double> kernel_row;
        for (auto &&candidate : feature_table) {
            auto difference = 0.0;
            for (auto counter : range_el(0, row.len()))
                difference += square((row[counter] - candidate[counter]));
            kernel_row.append(exp(-::D * difference));
        }
        kernel_table.append(kernel_row);
    }
    return kernel_table;
}

template <typename T1, typename T2, typename T3, typename T4> auto train_adatron(const T1 &kernel_table, const T2 &label_table, const T3 &h, const T4 &c)
{
    auto tolerance = 0.5;
    auto alphas = range_el(0, _get<0>(label_table).len()).map([&kernel_table](const auto &_){return (create_array({0.0}) * kernel_table.len());});
    auto betas = range_el(0, _get<0>(label_table).len()).map([&kernel_table](const auto &_){return (create_array({0.0}) * kernel_table.len());});
    auto bias = create_array({0.0}) * _get<0>(label_table).len();
    auto labelalphas = create_array({0.0}) * kernel_table.len();
    auto max_differences = create_array({make_tuple(0.0, 0)}) * _get<0>(label_table).len();
    for (auto iteration : range_el(0, 10 * kernel_table.len())) {
        print(u"Starting iteration #...."_S.format(iteration));
        if (iteration == 20)
            return make_tuple(alphas, bias);
        for (auto klass : range_el(0, _get<0>(label_table).len())) {
            max_differences.set(klass, make_tuple(0.0, 0));
            for (auto elem : range_el(0, kernel_table.len()))
                labelalphas.set(elem, label_table[elem][klass] * alphas[klass][elem]);
            for (auto col_counter : range_el(0, kernel_table.len())) {
                auto prediction = 0.0;
                for (auto row_counter : range_el(0, kernel_table.len()))
                    prediction += kernel_table[col_counter][row_counter] * labelalphas[row_counter];
                auto g = 1.0 - ((prediction + bias[klass]) * label_table[col_counter][klass]);
                betas[klass].set(col_counter, min(max((alphas[klass][col_counter] + h * g), 0.0), c));
                auto difference = abs(alphas[klass][col_counter] - betas[klass][col_counter]);
                if (difference > _get<0>(max_differences[klass]))
                    max_differences.set(klass, make_tuple(difference, col_counter));
            }
            if (all_map(max_differences, [&tolerance](const auto &max_difference){return _get<0>(max_difference) < tolerance;}))
                return make_tuple(alphas, bias);
            else {
                alphas[klass].set(_get<1>(max_differences[klass]), betas[klass][_get<1>(max_differences[klass])]);
                auto element_sum = 0.0;
                for (auto element_counter : range_el(0, kernel_table.len()))
                    element_sum += label_table[element_counter][klass] * alphas[klass][element_counter] / 4.0;
                bias.set(klass, bias[klass] + element_sum);
            }
        }
    }
}

template <typename T1, typename T2, typename T3, typename T4> auto calculate_error(const T1 &alphas, const T2 &bias, const T3 &kernel_table, const T4 &label_table)
{
    auto prediction = 0.0;
    auto predictions = range_el(0, _get<0>(label_table).len()).map([&kernel_table](const auto &_){return (create_array({0.0}) * kernel_table.len());});
    for (auto klass : range_el(0, _get<0>(label_table).len()))
        for (auto col_counter : range_el(0, kernel_table.len())) {
            for (auto row_counter : range_el(0, kernel_table.len()))
                prediction += kernel_table[col_counter][row_counter] * label_table[row_counter][klass] * alphas[klass][row_counter];
            predictions[klass].set(col_counter, prediction + bias[klass]);
        }

    for (auto col_counter : range_el(0, kernel_table.len())) {
        Array<double> current_predictions;
        auto error = 0;
        for (auto row_counter : range_el(0, _get<0>(label_table).len()))
            current_predictions.append(predictions[row_counter][col_counter]);
        auto predicted_class = current_predictions.index(max(current_predictions));
        if (label_table[col_counter][predicted_class] < 0)
            error++;
        return 1.0 * error / kernel_table.len();
    }
}

int main()
{
    for (auto &&[filename, type_id] : create_array({make_tuple(u"testdata/c.txt"_S, CYTOSOLIC), make_tuple(u"testdata/e.txt"_S, EXTRACELLULAR), make_tuple(u"testdata/n.txt"_S, NUCLEAR), make_tuple(u"testdata/m.txt"_S, MITOCHONDRIAL)}))
        load_file(filename, type_id);
    print(u"Creating feature tables..."_S);
    auto [feature_table, label_table] = create_tables();
    print(u"Creating kernel table..."_S);
    auto kernel_table = create_kernel_table(feature_table);
    print(u"Training SVM..."_S);
    auto [alphas, bias] = train_adatron(kernel_table, label_table, 1.0, 3.0);
    print(calculate_error(alphas, bias, kernel_table, label_table));
}
