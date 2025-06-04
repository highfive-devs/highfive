# Developer Guide
First clone the repository and remember the `--recursive`:
```bash
git clone --recursive git@github.com:BlueBrain/HighFive.git
```
The instructions to recover if you forgot are:
```bash
git submodule update --init --recursive
```

One remark on submodules: each HighFive commit expects that the submodules are
at a particular commit. The catch is that performing `git checkout` will not
update the submodules automatically. Hence, sometimes a `git submodule update
--recursive` might be needed to checkout the expected version of the
submodules.

## Compiling and Running the Tests
The instructions for compiling with examples and unit-tests are:

```bash
cmake -B build -DCMAKE_BUILD_TYPE={Debug,Release} .
cmake --build build --parallel
ctest --test-dir build
```

You might want to add:
* `-DHIGHFIVE_TEST_BOOST=On` or other optional dependencies on,
* `-DHIGHFIVE_MAX_ERRORS=3` to only show the first three errors.

Generic CMake reminders:
* `-DCMAKE_INSTALL_PREFIX` defines where HighFive will be installed,
* `-DCMAKE_PREFIX_PATH` defines where `*Config.cmake` files are found.

## Contributing
There's numerous HDF5 features that haven't been wrapped yet. HighFive is a
collaborative effort to slowly cover ever larger parts of the HDF5 library.
The process of contributing is to fork the repository and then create a PR.
Please ensure that any new API is appropriately documented and covered with
tests.

### Code formatting
The project is formatted using clang-format version 12.0.1 and CI will complain
if a commit isn't formatted accordingly. The `.clang-format` is at the root of
the git repository. Conveniently, `clang-format` is available via `pip`.

Formatting the entire code base can be done with:
```bash
    bin/format.sh
```
which will install the required version of clang-format in a venv called
`.clang-format-venv`.

To format only the changed files `git-clang-format` can be used.

## Releasing HighFive
Before releasing a new version perform the following:

* Update `CHANGELOG.md` and `AUTHORS.txt` as required.
* Update `CMakeLists.txt` and `include/highfive/H5Version.hpp`.
* Follow semantic versioning when deciding the next version number.
* Check that
  [HighFive-testing](https://github.com/BlueBrain/HighFive-testing/actions) ran
  recently.

At this point there should be a commit on master which will be the release
candidate. Don't tag it yet.

Next step is to update the [HighFive/spack](https://github.com/BlueBrain/spack)
recipe such that the proposed version points to the release candidate using the
SHA of that commit. The recipe will look something like this:

```python
    # ...

    version("2.8.0", commit="094400f22145bcdcd2726ce72888d9d1c21e7068")
    version("2.7.1", sha256="25b4c51a94d1e670dc93b9b73f51e79b65d8ff49bcd6e5d5582d5ecd2789a249")
    version("2.7.0", sha256="8e05672ddf81a59ce014b1d065bd9a8c5034dbd91a5c2578e805ef880afa5907")
    # ...
```

Push the changes to the BlueBrain spack repository. This will trigger building
all BBP dependencies of HighFive, i.e. another integration test. Don't actually
merge this commit yet.

Now that we know that the integration test ran, and all BBP software can be
built with the proposed version of HighFive, we can proceed and create the
release. Once this is done perform a final round of updates:

* Download the archive (`*.tar.gz`) and compute its SHA256.
* Update BlueBrain Spack recipe to use the archive and not the Git commit.
* Update the upstream Spack recipe.
* A Zenodo record should be generated automatically, check if it's sensible.

## Writing Tests
### Generate Multi-Dimensional Test Data
Input array of any dimension and type can be generated using the template class
`DataGenerator`. For example:
```
auto dims = std::vector<size_t>{4, 2};
auto values = testing::DataGenerator<std::vector<std::array<double, 2>>::create(dims);
```
Generates an `std::vector<std::array<double, 2>>` initialized with suitable
values.

If "suitable" isn't specific enough, one can specify a callback:
```
auto callback = [](const std::vector<size_t>& indices) {
    return 42.0;
}

auto values = testing::DataGenerator<std::vector<double>>::create(dims, callback);
```

The `dims` can be generated via `testing::DataGenerator::default_dims` or by
using `testing::DataGenerator::sanitize_dims`. Remember, that certain
containers are fixed size and that we often compute the number of elements by
multiplying the dims.

### Generate Scalar Test Data
To generate a single "suitable" element use template class `DefaultValues`, e.g.
```
auto default_values = testing::DefaultValues<double>();
auto x = testing::DefaultValues<double>(indices);
```

### Accessing Elements
To access a particular element from an unknown container use the following trait:
```
using trait = testing::ContainerTraits<std::vector<std::array<int, 2>>;
// auto x = values[1][0];
auto x = trait::get(values, {1, 0});

// values[1][0] = 42.0;
trait::set(values, {1, 0}, 42.0);
```

### Utilities For Multi-Dimensional Arrays
Use `testing::DataGenerator::allocate` to allocate an array (without filling
it) and `testing::copy` to copy an array from one type to another. There's
`testing::ravel`, `testing::unravel` and `testing::flat_size` to compute the
position in a flat array from a multi-dimensional index, the reverse and the
number of element in the multi-dimensional array.

### Deduplicating DataSet and Attribute
Due to how HighFive is written testing `DataSet` and `Attribute` often requires
duplicating the entire test code because somewhere a `createDataSet` must be
replaced with `createAttribute`. Use `testing::AttributeCreateTraits` and
`testing::DataSetCreateTraits`. For example,
```
template<class CreateTraits>
void check_write(...) {
    // Same as one of:
    //   file.createDataSet(name, values);
    //   file.createAttribute(name, values);
    CreateTraits::create(file, name, values);
}
```

### Test Organization
#### Multi-Dimensional Arrays
All tests for reading/writing whole multi-dimensional arrays to datasets or
attributes belong in `tests/unit/test_all_types.cpp`. This
includes write/read cycles; checking all the generic edges cases, e.g. empty
arrays and mismatching sizes; and checking non-reallocation.

Read/Write cycles are implemented in two distinct checks. One for writing and
another for reading. When checking writing we read with a "trusted"
multi-dimensional array (a nested `std::vector`), and vice-versa when checking
reading. This matters because certain bugs, like writing a column major array
as if it were row-major can't be caught if one reads it back into a
column-major array.

Remember, `std::vector<bool>` is very different from all other `std::vector`s.

Every container `template<class T> C;` should at least be checked with all of
the following `T`s that are supported by the container: `bool`, `double`,
`std::string`, `std::vector`, `std::array`. The reason is `bool` and
`std::string` are special, `double` is just a POD, `std::vector` requires
dynamic memory allocation and `std::array` is statically allocated.

Similarly, each container should be put inside an `std::vector` and an
`std::array`.

#### Scalar Data Set
Write-read cycles for scalar values should be implemented in
`tests/unit/tests_high_five_scalar.cpp`.

#### Data Types
Unit-tests related to checking that `DataType` API, go in
`tests/unit/tests_high_data_type.cpp`.

#### Empty Arrays
Check related to empty arrays to in `tests/unit/test_empty_arrays.cpp`.

#### Selections
Anything selection related goes in `tests/unit/test_high_five_selection.cpp`.
This includes things like `ElementSet` and `HyperSlab`.

#### Strings
Regular write-read cycles for strings are performed along with the other types,
see above. This should cover compatibility of `std::string` with all
containers. However, additional testing is required, e.g. character set,
padding, fixed vs. variable length. These all go in
`tests/unit/test_string.cpp`.

#### Specific Tests For Optional Containers
If containers, e.g. `Eigen::Matrix` require special checks those go in files
called `tests/unit/test_high_five_*.cpp` where `*` is `eigen` for Eigen.

#### Memory Layout Assumptions
In HighFive we make assumptions about the memory layout of certain types. For
example, we assume that
```
auto array = std::vector<std::array<double, 2>>(n);
doube * ptr = (double*) array.data();
```
is a sensible thing to do. We assume similar about `bool` and
`details::Boolean`. These types of tests go into
`tests/unit/tests_high_five_memory_layout.cpp`.

#### H5Easy
Anything `H5Easy` related goes in files with the appropriate name.

#### Everything Else
What's left goes in `tests/unit/test_high_five_base.cpp`. This covers opening
files, groups, dataset or attributes; checking certain pathological edge cases;
etc.

## CMake Integration
In `tests/cmake_integration` we test that HighFive can be used in downstream projects;
and that those project can, in turn, be used.

### Overview
#### Vendoring Strategies
We'll refer to the process of embedding a copy of HighFive into a consuming
library as *vendoring*. The *vendoring strategies* will include the strategy to
not vendor.

There's two broad strategies for integrating HighFive into other code: *finding* or
*vendoring*. When finding HighFive, the assumption by the consumer is that it's
been installed somewhere and it can find it. When vendoring, the consumer
brings their own copy of HighFive and uses it. The different vendoring
strategies are:

- **find_package**: the standard way for finding dependencies in CMake. Usually
  the assumption is that HighFive was install properly, either systemwide or in
  a specific subdirectory. HighFive is then found with `find_package` (or
  `find_dependency` when called from `*Config.cmake`).

- **add_subdirectory**: the consuming code contains a submodule or subdirectory
  with the HighFive code; and `add_subdirectory` is used to bring HighFive and
  all it's targets into the consumer.

- **fetch_content**: the consuming code uses CMake's FetchContent to download
  and integrate HighFive.

- **external_project**: similar to FetchContent; we don't current test if this
  works.

#### Integration Strategies
These refer to downstream projects picking different HighFive targerts to
"link" with HighFive. There's four: two regular targets, a target that only
adds `-I <dir>` and one that skips all HighFive CMake code.

#### Location Hints
There are serveral ways of indicating where to find a package:

- **CMAKE_PREFIX_PATH**: which adds a list of directories to the list of
  directories that are used as prefixes when searching for `HighFiveConfig.`

- **HighFive_ROOT**: which specifies a guess for where to additionally look
  (but only when finding HighFive).

There's two types of directories where a dependency can be located:

- **install**: the place it ends up after `cmake --install build`.

- **build**: one can, if one wants to (and we have users that do), specify
  a build directory (not and install directory) as `HighFive_ROOT`.


#### CMake's `export()`
Furthermore, there's `export(...)`. Documentation describes it as being useful
for cross-compilation, when one wants to have a set of host tool along with a
library compiled for the device. It seems we don't need it in HighFive and can
make it and easily write test consumers work perfectly without.

However, if one of our consumers adds `export(...)` to their `CMakeLists.txt`
then their build breaks, complaining about missing HighFive targets (and it
seems they can't "fix it up" on their end because then CMake complains that
there's duplicate exported HighFive related targets).

The second way ommiting the missing `export()` can break downstream projects is
if they attempt to use HighFive's build directory (not install directory) as
`HighFive_ROOT` (or `CMAKE_PREFIX_PATH`).

#### HighFive Visibility
This is the idea that libraries behave differently depending on whether they
use HighFive in their distributed headers; or not. If they don't they could
attempt to hide the fact that they use HighFive from their users.

Currently, we don't check that libraries can hide the use of HighFive.

#### Applications vs Libraries
In what follows application will refer to code that compiles into an
executable. While libraries refer to code that compiles into a binary that
other developers will link to in their library or application.

Libraries and applications that have dependencies that use HighFive must be
able to agree on a common version of HighFive they want to use. Otherwise,
during the final linking phase, multiple definitions of the same HighFive
symbols will exist, and they linker will pick one (arbitrarily); which is only
safe if all definitions are identical.

### Test "Organization"
The script to check everything is unwieldy. Here's a summary of what it attemps
to do.

There's three downstream project in play: `dependent_library` is a library that
uses HighFive in its public API, `application` is an executable that uses
HighFive directly, `test_dependent_library` is an application that uses
`dependent_library` its purpose is to check that our users can write CMake code
that makes them integrate well with other projects.

The conceptually easy choices are:

- Application that don't have dependencies that use HighFive, can use any
  strategy to integrate HighFive; because they know they're the only ones using
  HighFive.

- Libraries and applications that have dependencies that use HighFive should
  use `find_package` since it's the easiest way of injecting a common version
  of HighFive everwhere.

Since we can't (and don't want to) force our consumers to use `find_package`
and ban vendoring, we have to test what happens when libraries vendor
HighFive. (Many of these are likely sources of headache if you try to figure
out which code is when using which of the multiple copies of HighFive
involved; and how they decide to use that version.)

We'll assume that there's some way that all involved projects agree on a single
version of HighFive (i.e. the exact same source code or git commit).

The `dependent_library` will integrate HighFive using any of the following
strategies: `external` in two variations one from HighFive's install dir and
the other from HighFive's build dir; `submodule` in two variations which uses
`add_subdirectory` once with and once without `EXCLUDE_FROM_ALL`,
`fetch_content` in one variation.

The `test_dependent_library` itself always incorporates `dependent_library`
using `find_package` (Config mode). Since that layer might choose any of the
strategies of integrating HighFive, we again check several. Additionally,
`test_dependent_library` isn't (and probably shouldn't be) required to
integrate HighFive directly: option `none`.

Imagine a script that tries all combinations of the above; and attempts to only
provide hints for the HighFive package location when needed, e.g. for `none`
there's a `find_dependency` in `Hi5DependentConfig` that needs to be told where
to look. This is what `test_cmake_integration.sh` attempts to do.
