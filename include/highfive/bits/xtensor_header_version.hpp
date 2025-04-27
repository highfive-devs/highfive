// clang-format off
#if HIGHFIVE_XTENSOR_HEADER_VERSION == 0
  #if __cplusplus >= 201703L
    #if __has_include(<xtensor/xtensor.hpp>)
      #define HIGHFIVE_XTENSOR_HEADER_VERSION 1
    #elif __has_include(<xtensor/containers/xtensor.hpp>)
      #define HIGHFIVE_XTENSOR_HEADER_VERSION 2
    #else
      #error "Unable to guess HIGHFIVE_XTENSOR_HEADER_VERSION. Please set manually."
    #endif
  #elif __cplusplus == 201402L
    // XTensor 0.26 and newer require C++17. Hence, if we have C++14, only
    // `HIGHFIVE_XTENSOR_HEADER_VERSION == 1` makes sense.
    #define HIGHFIVE_XTENSOR_HEADER_VERSION 1
  #elif defined(_MSC_VER) && __cplusplus == 199711L
    #error \
      "Use /Zc:__cplusplus to make MSVC set __cplusplus correctly or HIGHFIVE_XTENSOR_HEADER_VERSION to skip xtensor version deduction."
  #else
    #error "HighFive requires C++14 or newer."
  #endif
#endif
// clang-format on
