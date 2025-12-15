/*
 *  Copyright (c), 2025, Antony Chan
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <highfive/highfive.hpp>

namespace {
template <size_t Rank>
struct RegularHyperSlabNoMalloc
    : public HighFive::HyperSlabInterface<RegularHyperSlabNoMalloc<Rank>> {
    RegularHyperSlabNoMalloc(const std::array<hsize_t, Rank>& offset_,
                             const std::array<hsize_t, Rank>& count_)
        : offset(offset_)
        , count(count_) {}

    HighFive::DataSpace apply(const HighFive::DataSpace& space_) const {
        auto space = space_.clone();
        const auto error_code = H5Sselect_hyperslab(
            space.getId(), H5S_SELECT_SET, offset.data(), nullptr, count.data(), nullptr);

        if (error_code < 0) {
            HighFive::HDF5ErrMapper::ToException<HighFive::DataSpaceException>(
                "Unable to select hyperslab");
        }
        return space;
    }

    std::array<hsize_t, Rank> offset{};
    std::array<hsize_t, Rank> count{};
};
}  // namespace

int main(void) {
    using namespace HighFive;

    // Create a new file using the default property lists.
    File file{"select_partial_example_no_alloc.h5", File::Truncate};

    // Example values in a 2x5 array. This might cause one allocation.
    const std::vector<std::array<double, 5>> values{
        std::array<double, 5>{1.0, 2.0, 4.0, 8.0, 16.0},
        std::array<double, 5>{32.0, 64.0, 128.0, 256.0, 512.0}};

    // Might cause allocations in HDF5 and for std::string if "dset" is too long for
    // short-string optimization.
    DataSet dataset = file.createDataSet<double>("dset", DataSpace::From(values));

    // -------------------------------------------
    // From here on: no allocations are permitted.
    dataset.write(values);

    // Select the 2x2 values after an offset of 0x2.
    std::array<std::array<double, 2>, 2> result;

    // Specify the selection without any memory allocations.
    dataset.select(RegularHyperSlabNoMalloc<2>{{0, 2}, {2, 2}}).read_raw(result.front().data());

    // Print out the 4 values.
    for (auto i: result) {
        for (auto j: i) {
            std::cout << ' ' << j;
        }
        std::cout << '\n';
    }

    return 0;
}
