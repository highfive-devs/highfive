/*
 *  Copyright (c), 2025, HighFive Developers
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

// This example demonstrates using `std::mdspan`. An `std::mdspan` is a
// multi-dimensional view over contiguous memory, similar to `std::span` but
// for multi-dimensional arrays.

#include <string>
#include <vector>
#include <mdspan>
#include <cassert>

#include <highfive/highfive.hpp>

#include <highfive/mdspan.hpp>

int main(void) {
    using namespace HighFive;

    std::string file_name = "read_write_mdspan.h5";
    std::string dataset_name = "array";

    // Assume we have multi-dimensional data stored contiguously, e.g. in a
    // vector.
    constexpr size_t rows = 3;
    constexpr size_t cols = 4;
    std::vector<double> values_row_major(rows * cols);

    // Fill the data
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            values_row_major[i * cols + j] = double(i * cols + j);
        }
    }

    // Create a 2D mdspan view over the contiguous memory.
    auto view_row_major = std::mdspan(values_row_major.data(), std::extents{rows, cols});

    {
        File file(file_name, File::Truncate);
        auto dataset = file.createDataSet(dataset_name, view_row_major);
    }

    // Let's read from file.
    {
        File file(file_name, File::ReadOnly);
        auto dataset = file.getDataSet(dataset_name);

        // Assume that memory was allocated by some means, e.g.:
        auto dims = dataset.getDimensions();
        auto values_col_major = std::vector<double>(dataset.getElementCount());

        // Create a column-major mdspan view over the preallocated memory.
        auto m = std::layout_left::mapping{std::extents{dims[0], dims[1]}};
        auto view_col_major = std::mdspan(values_col_major.data(), m);

        // ... now we can read into the preallocated memory:
        dataset.read(view_col_major);

        // Check that the data was read correctly.
        for (size_t i = 0; i < rows; ++i) {
            for (size_t j = 0; j < cols; ++j) {
                if (view_col_major[i, j] != view_row_major[i, j]) {
                    std::cerr << "Error: data was not read correctly." << std::endl;
                    return 1;
                }
            }
        }
    }

    return 0;
}
