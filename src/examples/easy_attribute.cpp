/// To enable plug-ins, load the relevant libraries BEFORE HighFive. E.g.
///
///   #include <xtensor/xtensor.hpp>
///   #include <Eigen/Eigen>
///   #include <highfive/H5Easy.hpp>
///
/// or ask HighFive to include them. E.g.
///
///   #define H5_USE_XTENSOR
///   #define H5_USE_EIGEN
///   #include <highfive/H5Easy.hpp>
///

// optionally enable plug-in xtensor
#ifdef H5_USE_XTENSOR
#include "bits/xtensor_header_version.hpp"
#if HIGHFIVE_XTENSOR_HEADER_VERSION == 1
#include <xtensor/xtensor.hpp>
#elif HIGHFIVE_XTENSOR_HEADER_VERSION == 2
#include <xtensor/containers/xtensor.hpp>
#else
#error "Failed to detect HIGHFIVE_XTENSOR_HEADER_VERSION."
#endif
#endif

// optionally enable plug-in Eigen
#ifdef H5_USE_EIGEN
#include <Eigen/Eigen>
#endif

#include <highfive/H5Easy.hpp>

int main() {
    H5Easy::File file("example.h5", H5Easy::File::Overwrite);

    std::vector<double> measurement = {1.0, 2.0, 3.0};
    std::string desc = "This is an important dataset.";
    double temperature = 1.234;

    H5Easy::dump(file, "/path/to/measurement", measurement);
    H5Easy::dumpAttribute(file, "/path/to/measurement", "description", desc);
    H5Easy::dumpAttribute(file, "/path/to/measurement", "temperature", temperature);

    return 0;
}
