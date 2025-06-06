/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <memory>
#include <stdexcept>
#include <string>

#include <H5Ipublic.h>

namespace HighFive {

///
/// \brief Basic HighFive Exception class
///
///
class Exception: public std::exception {
  public:
    explicit Exception(const std::string& err_msg)
        : _errmsg(err_msg) {}

    Exception(const Exception& other) = default;
    Exception(Exception&& other) noexcept = default;

    Exception& operator=(const Exception& other) = default;
    Exception& operator=(Exception&& other) noexcept = default;

    ~Exception() noexcept override {}

    ///
    /// \brief get the current exception error message
    /// \return
    ///
    inline const char* what() const noexcept override {
        return _errmsg.c_str();
    }

    ///
    /// \brief define the error message
    /// \param errmsg
    ///
    inline virtual void setErrorMsg(const std::string& errmsg) {
        _errmsg = errmsg;
    }

    ///
    /// \brief nextException
    /// \return pointer to the next exception in the chain, or NULL if not
    /// existing
    ///
    inline Exception* nextException() const {
        return _next.get();
    }

    ///
    /// \brief HDF5 library error mapper
    /// \return HDF5 major error number
    ///
    inline hid_t getErrMajor() const {
        return _err_major;
    }

    ///
    /// \brief HDF5 library error mapper
    /// \return HDF5 minor error number
    ///
    inline hid_t getErrMinor() const {
        return _err_minor;
    }

  private:
    std::string _errmsg;
    std::shared_ptr<Exception> _next = nullptr;
    hid_t _err_major = 0, _err_minor = 0;

    friend struct HDF5ErrMapper;
};

///
/// \brief Exception specific to HighFive Object interface
///
class ObjectException: public Exception {
  public:
    explicit ObjectException(const std::string& err_msg)
        : Exception(err_msg) {}
};

///
/// \brief Exception specific to HighFive DataType interface
///
class DataTypeException: public Exception {
  public:
    explicit DataTypeException(const std::string& err_msg)
        : Exception(err_msg) {}
};

///
/// \brief Exception specific to HighFive File interface
///
class FileException: public Exception {
  public:
    explicit FileException(const std::string& err_msg)
        : Exception(err_msg) {}
};

///
/// \brief Exception specific to HighFive DataSpace interface
///
class DataSpaceException: public Exception {
  public:
    explicit DataSpaceException(const std::string& err_msg)
        : Exception(err_msg) {}
};

///
/// \brief Exception specific to HighFive Attribute interface
///
class AttributeException: public Exception {
  public:
    explicit AttributeException(const std::string& err_msg)
        : Exception(err_msg) {}
};

///
/// \brief Exception specific to HighFive DataSet interface
///
class DataSetException: public Exception {
  public:
    explicit DataSetException(const std::string& err_msg)
        : Exception(err_msg) {}
};

///
/// \brief Exception specific to HighFive Group interface
///
class GroupException: public Exception {
  public:
    explicit GroupException(const std::string& err_msg)
        : Exception(err_msg) {}
};

///
/// \brief Exception specific to HighFive Property interface
///
class PropertyException: public Exception {
  public:
    explicit PropertyException(const std::string& err_msg)
        : Exception(err_msg) {}
};

///
/// \brief Exception specific to HighFive Reference interface
///
class ReferenceException: public Exception {
  public:
    explicit ReferenceException(const std::string& err_msg)
        : Exception(err_msg) {}
};
}  // namespace HighFive

#include "bits/H5Exception_misc.hpp"
