> [!NOTE]
> HighFive was orignally developed and maintained at
> https://github.com/BlueBrain/HighFive. To continue maintenance of HighFive as
> an independent open-source code without support from BBP or EPFL, some (one)
> of the developers decided to create this repository.

# HighFive - HDF5 header-only C++ Library

[![Tests](https://github.com/highfive-devs/highfive/actions/workflows/ci.yml/badge.svg)](https://github.com/highfive-devs/highfive/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/highfive-devs/highfive/graph/badge.svg?token=IGQLPCOGTJ)](https://codecov.io/gh/highfive-devs/highfive)
[![Doxygen -> gh-pages](https://github.com/highfive-devs/highfive/actions/workflows/gh-pages.yml/badge.svg?branch=main)](https://highfive-devs.github.io/highfive)
[![Zenodo](https://zenodo.org/badge/47755262.svg)](https://zenodo.org/doi/10.5281/zenodo.10679422)

HighFive is a modern, user-friendly, header-only, C++14 interface for libhdf5.
```c++
// Open (or create) a file:
HighFive::File file(filename, HighFive::File::Truncate);

// ... write the data to disk
std::vector<int> data(50, 1);
auto dataset = file.createDataSet("grp/data", data);

// ... and read it back, automatically allocating memory:
data = dataset.read<std::vector<int>>();
```

It integrates nicely with other CMake projects through CMake targets:
```cmake
find_package(HighFive REQUIRED)
target_link_libraries(foo HighFive::HighFive)
```

### Design
- User-friendly, C++14 interface.
- Automatic type mapping (serialization).
- Zero/low overhead, when possible.
- RAII for opening/closing files, groups, datasets, etc.
- Compatible with the HDF5 C API.
- A second API layer `H5Easy` provides a oneliner API for common, simple
  usecases.

### Feature support
- create/read/write files, datasets, attributes, groups, dataspaces.
- automatic memory management / ref counting
- automatic conversion (serialization) of STL, Boost, Eigen and XTensor containers from/to any dataset
- automatic conversion of `std::string` to/from variable- or fixed-length string dataset
- simplified APIs for common selections and full support of (irregular) HyperSlabs
- parallel HDF5 using MPI
- Singe Writer, Multiple Reader (SMWR) mode
- Advanced types: Compound, Enum, Arrays of Fixed-length strings, References
- half-precision (16-bit) floating-point datasets
- etc... (see [ChangeLog](./CHANGELOG.md))

### Known flaws
- HighFive is not thread-safe. At best it has the same limitations as the HDF5 library. However, HighFive objects modify their members without protecting these writes. Users have reported that HighFive is not thread-safe even when using the threadsafe HDF5 library, e.g., https://github.com/BlueBrain/HighFive/discussions/675.
- Eigen support in core HighFive was broken until v3.0. See https://github.com/BlueBrain/HighFive/issues/532. H5Easy was not
  affected.
- The support of fixed length strings isn't ideal.


## Examples
Here's an expanded version of the example provided above.
```c++
#include <highfive/highfive.hpp>

using namespace HighFive;

std::string filename = "/tmp/new_file.h5";

{
    // We create an empty HDF55 file, by truncating an existing
    // file if required:
    File file(filename, File::Truncate);

    std::vector<int> data(50, 1);
    file.createDataSet("grp/data", data);
}

{
    // We open the file as read-only:
    File file(filename, File::ReadOnly);
    auto dataset = file.getDataSet("grp/data");

    // Read back, with allocating:
    auto data = dataset.read<std::vector<int>>();

    // Because `data` has the correct size, this will
    // not cause `data` to be reallocated:
    dataset.read(data);
}
```

**Note:** As of 2.8.0, one can use `highfive/highfive.hpp` to include
everything HighFive. Prior to 2.8.0 one would include `highfive/H5File.hpp`.

**Note:** For advanced usecases the dataset can be created without immediately
writing to it. This is common in MPI-IO related patterns, or when growing a
dataset over the course of a simulation.

### H5Easy
For simple, common usecases the [highfive/H5Easy.hpp](include/highfive/H5Easy.hpp)
interface provides single line solution. Here's the example from the
introduction again:
```cpp
#include <highfive/H5Easy.hpp>

int main() {
    H5Easy::File file("example.h5", H5Easy::File::Overwrite);

    int A = 42;
    H5Easy::dump(file, "/path/to/A", A);

    A = H5Easy::load<int>(file, "/path/to/A");
}
```
where `A` in this example can be replaced by other scalar variables or any of
the supported containers, i.e. STL, Boost, XTensor, Eigen and (Easy-only)
OpenCV.
See [easy_load_dump.cpp](src/examples/easy_load_dump.cpp) for more details.

**Note:** Classes such as `H5Easy::File` are just short for the regular
`HighFive` classes (in this case `HighFive::File`). They can thus be used
interchangeably.

### And Many More Examples!
We strive to have one example per usecase or feature of HighFive,
see [src/examples/](https://github.com/highfive-devs/highfive/blob/master/src/examples/)
for more examples.


## CMake integration
There's two common paths of integrating HighFive into a CMake based project.
The first is to "vendor" HighFive, the second is to install HighFive as a
normal C++ library. Since HighFive makes choices about how to integrate HDF5,
sometimes following the third Bailout Approach is needed.

Regular HDF5 CMake variables can be used. Interesting variables include:

* `HDF5_USE_STATIC_LIBRARIES` to link statically against the HDF5 library.
* `HDF5_PREFER_PARALLEL` to prefer pHDF5.
* `HDF5_IS_PARALLEL` to check if HDF5 is parallel.

Please consult `tests/cmake_integration` for examples of how to write libraries
or applications using HighFive.

### Vendoring HighFive

In this approach the HighFive sources are included in a subdirectory of the
project (typically as a git submodule), for example in `third_party/HighFive`.

The projects `CMakeLists.txt` add the following lines
```cmake
add_subdirectory(third_party/HighFive)
target_link_libraries(foo HighFive::HighFive)
```

**Note:** `add_subdirectory(third_party/HighFive)` will search and "link" HDF5
but wont search or link any optional dependencies such as Boost.

**Note:** The two targets `HighFive` and `HighFive::HighFive` are aliases. The
former is older and works with v2, while the latter was introduced in v3,
because CMake targets work more nicely if they contain `::`.

### Regular Installation of HighFive

Alternatively, HighFive can be install and "found" like regular software.
The project's `CMakeLists.txt` should add the following:
```cmake
find_package(HighFive REQUIRED)
target_link_libraries(foo HighFive::HighFive)
```

**Note:** `find_package(HighFive)` will search for HDF5. "Linking" to
`HighFive` includes linking with HDF5.

### Bailout Approach

To prevent HighFive from searching or "linking" to HDF5 the project's
`CMakeLists.txt` should contain the following:

```cmake
# Prevent HighFive CMake code from searching for HDF5:
set(HIGHFIVE_FIND_HDF5 Off)

# Then "find" HighFive as usual:
find_package(HighFive REQUIRED)
# alternatively, when vendoring:
# add_subdirectory(third_party/HighFive)

# Finally, use the target `HighFive::Include` which
# doesn't add a dependency on HDF5.
target_link_libraries(foo HighFive::Include)

# Proceed to find and link HDF5 as required.
```

### Optional Dependencies
HighFive does not attempt to find or "link" to any optional dependencies, such
as Boost, Eigen, etc. Any project using HighFive with any of the optional
dependencies must include the respective header:
```
#include <highfive/boost.hpp>
#include <highfive/eigen.hpp>
```
and add the required CMake code to find and link against the dependencies. For
Boost the required lines might be
```
find_package(Boost REQUIRED)
target_link_libraries(foo PUBLIC Boost::headers)
```

HighFive integrates with the following libraries:
- boost (optional)
- eigen3 (optional)
- xtensor (optional)
- half (optional)

#### XTensor Header Location
XTensor reorganized their headers in version 0.26. HighFive attempts to guess
where the headers can be found. The guessing can be overridded by setting
`HIGHFIVE_XTENSOR_HEADER_VERSION` to: `1` for finding `xtensor.hpp` in
`<xtensor/xtensor.hpp>` and `2` for `<xtensor/containers/xtensor.hpp>`.

## Versioning & Code Stability
We use semantic versioning, all API breaking changes are considered bug, please
report them as such.

We've recently released v3.0.0, see
[Migration Guide](https://highfive-devs.github.io/highfive/md__2home_2runner_2work_2_high_five_2_high_five_2doc_2migration__guide.html).
Please let us know if there's any missing or incorrect information in the
Migration Guide, it'll help others make the switch more easily.

We're unlikely to backport bug fixes or features to v2.x.y. Therefore, if
you're affected by a bug, please update to v3.

Since HighFive is reasonably mature, development typically happens in short
bursts. Hence, waiting for "enough features" to accumulate on the main branch
doesn't make sense for us; and we'll release shortly after any meaningful
changes were made; but at most once a week. It seems better to risk three new
versions within three weeks than to have useful features linger on main for
half a year.

## Questions?

Please first check if your question/issue has been answered/reported at
[BlueBrain/HighFive](https://github.com/BlueBrain/HighFive).

Do you have questions on how to use HighFive? Would you like to share an interesting example or
discuss HighFive features? Head over to the [Discussions](https://github.com/highfive-devs/highfive/discussions)
forum and join the community.

For bugs and issues please use [Issues](https://github.com/highfive-devs/highfive/issues).

## Funding & Acknowledgment

HighFive releases are uploaded to Zenodo. If you wish to cite HighFive in a
scientific publication you can use the DOIs for the
[Zenodo records](https://zenodo.org/doi/10.5281/zenodo.10679422).

### Blue Brain Project Era: 2015 - 2024

HighFive was created and maintained as part of the BBP from 2015 until Dec 2024
(when BBP closed) at [BlueBrain/HighFive](https://github.com/BlueBrain/HighFive).

Please consult its README for funding information by the Blue Brain Project or EPFL.

### Post Blue Brain Project: 2025 - present

One of the main contributors to
[BlueBrain/HighFive](https://github.com/BlueBrain/HighFive) wanted to keep the
project alive past the end of BBP. This repository was created to provide a
seemless continuation of HighFive; and prevent fracturing or capturing of the
project.

This repository is not supported by the Blue Brain Project or EPFL.

## License & Copyright

Boost Software License 1.0
Copyright © 2015-2024 Blue Brain Project/EPFL
