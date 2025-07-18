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
 * It should be used in conjunction with SWMR writer (see swmr_write example)
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
    auto old_dims = std::vector<size_t>{0ul};
    size_t max_dim = 10;

    size_t fail_count = 0;
    while (true) {
        // refresh is needed for SWMR read
        dataset.refresh();

        dims = dataset.getDimensions();
        // if dimensions changed, it means new data was written to a file
        if (dims[0] != old_dims[0]) {
            std::vector<size_t> slice{dims[0] - old_dims[0]};
            auto values = dataset.select(old_dims, slice).read<std::vector<int>>();
            for (const auto& v: values) {
                std::cout << v << " ";
            }
            std::cout << std::flush;
            old_dims = dims;
            fail_count = 0;
        } else {
            fail_count++;
        }

        // there is no way to know that the writer has stopped
        // we know that our example writer writes exactly 10 values
        if (dims[0] >= max_dim) {
            break;
        }

        // our example writer should add a value every 100 ms
        // longer delay means something went wrong
        if (fail_count >= 10) {
            throw std::runtime_error("SWMR reader timed out.");
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std::cout << std::endl;
    return 0;
}
