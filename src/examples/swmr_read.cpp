/*
 *  Copyright (c), 2025, George Sedov
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <highfive/highfive.hpp>

const std::string file_name("swmr_read_write.h5");
const std::string dataset_name("array");

/**
 * This is the SWMR reader.
 * It should be used in conjunction with SWMR writer (see write_swmr example)
 */
int main(void) {
    using namespace HighFive;

    // give time for the writer to create the file
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    // Open file for SWMR read
    File file(file_name, File::ReadSWMR);

    std::cout << "Started the SWMR read" << std::endl;

    auto dataset = file.getDataSet(dataset_name);
    auto dims = dataset.getDimensions();
    auto olddims = std::vector<size_t>{0ul};
    while (true) {
        // refresh is needed for SWMR read
        dataset.refresh();

        dims = dataset.getDimensions();
        // if dimensions changed, it means new data was written to a file
        if (dims[0] != olddims[0]) {
            std::vector<size_t> slice {dims[0] - olddims[0]};
            auto values = dataset.select(olddims, slice).read<std::vector<int>>();
            for (const auto& v : values) {
                std::cout << v << " ";
            }
            std::cout << std::flush;
            olddims = dims;
        }

        // there is no way to know that the writer has stopped
        // we know that our example writer writes exactly 100 values
        if (dims[0] >= 100) {
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << std::endl;
    return 0;
}
