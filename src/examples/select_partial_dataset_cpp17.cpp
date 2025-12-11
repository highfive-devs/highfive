/*
 *  Copyright (c), 2017, Adrien Devresse
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
struct RegularHyperSlabNoMalloc {
    constexpr size_t rank() const {
        return Rank;
    }

    /// Dimensions when all gaps are removed.
    constexpr std::array<size_t, Rank> packedDims() const {
        auto dims = std::array<size_t, Rank>{};

        for (size_t i = 0; i < Rank; ++i) {
            dims[i] = count[i] * (block ? (*block)[i] : 1);
        }

        return dims;
    }

    HighFive::DataSpace apply(const HighFive::DataSpace& space_) const {
        auto space = space_.clone();
        const auto error_code = H5Sselect_hyperslab(space.getId(),
                                                    H5S_SELECT_SET,
                                                    offset.data(),
                                                    stride ? stride->data() : nullptr,
                                                    count.data(),
                                                    block ? block->data() : nullptr);

        if (error_code < 0) {
            HighFive::HDF5ErrMapper::ToException<HighFive::DataSpaceException>(
                "Unable to select hyperslab");
        }
        return space;
    }

    std::array<hsize_t, Rank> offset{};
    std::array<hsize_t, Rank> count{};
    std::optional<std::array<hsize_t, Rank>> stride{std::nullopt};
    std::optional<std::array<hsize_t, Rank>> block{std::nullopt};
};
}  // namespace

int main(void) {
    using namespace HighFive;

    // Create a new file using the default property lists.
    //
    // Note: In C++14, using braces in constructor implies "explicit" keyword. Compiler logs will
    // warn about possible implicit type conversion that may involves transient memory allocations.
    File file{"select_partial_example_cpp17.h5", File::ReadWrite | File::Create | File::Truncate};

    // we have some example values in a 2D vector 2x5
    // Specifying the inner dimensions as std::array ensures that the two-dimensional data is
    // contiguous, so that the compiler will only invoke one single memory allocation. The
    // compiler also ensures the column counts are identical.
    //
    // Note: C++17 required.
    const std::vector values{std::array{1.0, 2.0, 4.0, 8.0, 16.0},
                             {32.0, 64.0, 128.0, 256.0, 512.0}};

    // let's create a dataset of this size
    DataSet dataset = file.createDataSet<double>("dset", DataSpace::From(values));
    // and write them
    dataset.write(values);


    // now we read back 2x2 values after an offset of 0x2
    // Notice that std::array lives in the stack space memory. No memory allocation required.
    std::array<std::array<double, 2>, 2> result;

    // Specify the selection without any memory allocations.
    dataset.select(RegularHyperSlabNoMalloc<2>{{0, 2}, {2, 2}}).read_raw(result.front().data());

    // we print out 4 values
    for (auto i: result) {
        for (auto j: i) {
            std::cout << ' ' << j;
        }
        std::cout << '\n';
    }

    return 0;  // successfully terminated
}
