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
 * This is the SWMR writer.
 * It should be used in conjunction with SWMR reader (see read_swmr example)
 */
int main(void) {
    using namespace HighFive;

    // Create a new file
    // For SWMR we need to force the latest header, which is passed in AccessProps
    FileAccessProps fapl;
    fapl.add(FileVersionBounds(H5F_LIBVER_LATEST, H5F_LIBVER_LATEST));
    File file(file_name, File::Truncate, fapl);

    // To make sense for SWMR, the dataset should be extendable, and hence - chunkable
    DataSetCreateProps props;
    props.add(Chunking({1}));
    auto dataset =
        file.createDataSet<int>(dataset_name, DataSpace({0ul}, {DataSpace::UNLIMITED}), props);

    // start the SWMR write
    // you are not allowed to create new data headers (i.e. DataSets, Groups, and Attributes) after
    // that, you should also make sure all the Groups and Attributes are closed before calling this
    // function see https://docs.hdfgroup.org/archive/support/HDF5/Tutor/swmr.html for details
    file.startSWMR();

    // If you want to open an already-existing file for SWMR write, use
    // File file(file_name, File::WriteSWMR);
    // auto dataset = file.getDataSet(dataset_name);

    std::cout << "Started the SWMR write" << std::endl;

    // Let's write to file.
    for (int i = 0; i < 100; i++) {
        // resize the dataset to fit the new element
        dataset.resize({static_cast<size_t>(i + 1)});
        // select the dataset slice and write the number to it
        dataset.select({static_cast<size_t>(i)}, {1ul}).write(i);
        // in SWMR mode you need to explicitly flush the DataSet
        dataset.flush();

        // give time for the reader to react
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    return 0;
}
