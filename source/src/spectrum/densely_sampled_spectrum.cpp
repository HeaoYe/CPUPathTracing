#include "spectrum/densely_sampled_spectrum.hpp"
#include "util/csv.h"

DenselySampledSpectrum DenselySampledSpectrum::LoadCSV(
    const std::filesystem::path &filename,
    const std::string &header_name_lambda,
    const std::string &header_name_value
) {
    io::CSVReader<2> file(filename.string());
    file.read_header(io::ignore_extra_column, header_name_lambda, header_name_value);
    float value;
    std::vector<float> values;
    int lambda_min, lambda_max;
    bool first = true;
    while (file.read_row(lambda_max, value)) {
        if (first) {
            first = false;
            lambda_min = lambda_max;
        }
        values.push_back(value);
    }
    return DenselySampledSpectrum { values, lambda_min, lambda_max };
}
