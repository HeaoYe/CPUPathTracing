#include "spectrum/piecewise_linear_spectrum.hpp"
#include "util/csv.h"

PiecewiseLinearSpectrum PiecewiseLinearSpectrum::LoadCSV(
    const std::filesystem::path &filename,
    const std::string &header_name_lambda,
    float lambda_scale,
    const std::string &header_name_value
) {
    io::CSVReader<2> file(filename.string());
    file.read_header(io::ignore_extra_column, header_name_lambda, header_name_value);
    float lambda, value;
    std::vector<float> lambdas, values;
    while (file.read_row(lambda, value)) {
        lambdas.push_back(lambda * lambda_scale);
        values.push_back(value);
    }
    return PiecewiseLinearSpectrum { lambdas, values };
}
