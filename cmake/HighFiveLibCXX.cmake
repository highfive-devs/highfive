# HighFiveLibCXX.cmake
# Sets up CMAKE_* variables to use libc++ with clang when HIGHFIVE_USE_LIBCXX is enabled

if(HIGHFIVE_USE_LIBCXX AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Set compiler flags to use libc++
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    
    # Set linker flags to use libc++
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -stdlib=libc++")
    
    
    # Add necessary compile definitions for libc++
    add_compile_definitions(_LIBCPP_VERSION)
    
    message(STATUS "HighFive: Using libc++ for tests (clang only)")
elseif(HIGHFIVE_USE_LIBCXX AND NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(WARNING "HIGHFIVE_USE_LIBCXX is enabled but compiler is not clang. libc++ is only supported with clang.")
endif()
