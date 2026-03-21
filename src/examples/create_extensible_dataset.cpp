/*
 *  Copyright (c), 2017, Adrien Devresse
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <highfive/highfive.hpp>


#ifndef H5_HAVE_SUBFILING_VFD
#define H5_HAVE_SUBFILING_VFD 0
#endif

#if H5_HAVE_SUBFILING_VFD
#include "H5FDpublic.h"
#endif

const std::string FILE_NAME("create_extensible_dataset_example.h5");
const std::string DATASET_NAME("dset");

struct SubFiler {
  public:
    SubFiler() {
#ifndef H5_HAVE_SUBFILING_VFD
        vfd_config.shared_cfg.stripe_count = 20;
#else
        throw std::runtime_error("Subfiling not available!");
#endif
    }

  private:
    void apply(hid_t plist_id) const {
#ifndef H5_HAVE_SUBFILING_VFD

        H5Pget_fapl_subfiling(plist_id, &vfd_config); /* Get a default subfiling configuration */
        H5Pset_fapl_subfiling(plist_id, &vfd_config);
#endif
    }

#ifndef H5_HAVE_SUBFILING_VFD
    H5FD_subfiling_config_t vfd_config;
#endif
};


int main(int argc, char** argv) {
    using namespace HighFive;

    // initialize MPI
    MPI_Init(&argc, &argv);

    // Create a new file using the default property lists.
    FileAccessProps fapl;
    fapl.add(HighFive::MPIOFileAccess{MPI_COMM_WORLD, MPI_INFO_NULL});
    File file(FILE_NAME, File::ReadWrite | File::Create | File::Truncate, fapl);

    // Create a dataspace with initial shape and max shape
    DataSpace dataspace = DataSpace({4, 5}, {17, DataSpace::UNLIMITED});


    // Use chunking
    DataSetCreateProps props;
    props.add(Chunking(std::vector<hsize_t>{2, 2}));

    // Create the dataset
    DataSet dataset = file.createDataSet(DATASET_NAME, dataspace, create_datatype<double>(), props);

    {
        std::stringstream ss;
        for (auto d: dataset.getDimensions())
            ss << d << ",";
        std::cout << ss.str() << std::endl;
    }

    // Write into the initial part of the dataset
    double t1[3][1] = {{2.0}, {2.0}, {4.0}};
    dataset.select({0, 0}, {3, 1}).write(t1);

    // Resize the dataset to a larger size
    dataset.resize({4, 6});

    {
        std::stringstream ss;
        for (auto d: dataset.getDimensions())
            ss << d << ",";
        std::cout << ss.str() << std::endl;
    }

    // Write into the new part of the dataset
    double t2[1][3] = {{4.0, 8.0, 6.0}};
    dataset.select({3, 3}, {1, 3}).write(t2);

    // now we read it back
    std::vector<std::vector<double>> result;
    dataset.read(result);

    // we print it out and see:
    // 2 0 0 0 0 0
    // 2 0 0 0 0 0
    // 4 0 0 0 0 0
    // 0 0 0 4 8 6
    for (auto row: result) {
        for (auto col: row)
            std::cout << " " << col;
        std::cout << std::endl;
    }

    MPI_Finalize();
    return 0;
}
