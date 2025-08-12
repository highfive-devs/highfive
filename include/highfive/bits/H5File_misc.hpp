/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once
#include <filesystem>
#include <string>

#include <H5Fpublic.h>

#include "../H5Utility.hpp"
#include "H5Utils.hpp"
#include "h5f_wrapper.hpp"

namespace HighFive {

namespace {  // unnamed

// libhdf5 uses a preprocessor trick on their oflags
// we can not declare them constant without a mapper
inline unsigned convert_open_flag(File::AccessMode openFlags) {
    unsigned res_open = 0;
    if (any(openFlags & File::ReadOnly))
        res_open |= H5F_ACC_RDONLY;
    if (any(openFlags & File::ReadWrite))
        res_open |= H5F_ACC_RDWR;
    if (any(openFlags & File::Create))
        res_open |= H5F_ACC_CREAT;
    if (any(openFlags & File::Truncate))
        res_open |= H5F_ACC_TRUNC;
    if (any(openFlags & File::Excl))
        res_open |= H5F_ACC_EXCL;
    return res_open;
}
}  // namespace

inline File::File(const std::string& filename,
                  AccessMode openFlags,
                  const FileAccessProps& fileAccessProps)
    : File(filename, openFlags, FileCreateProps::Default(), fileAccessProps) {}


inline File::File(const std::string& filename,
                  AccessMode access_mode,
                  const FileCreateProps& fileCreateProps,
                  const FileAccessProps& fileAccessProps) {
    if (rest_vol_enabled()) {
        if (!std::filesystem::path(filename).is_absolute()) {
            _filename = "/" + filename;
        } else {
            _filename = filename;
        }
    } else {
        _filename = filename;
    }

    unsigned openFlags = convert_open_flag(access_mode);

    unsigned createMode = openFlags & (H5F_ACC_TRUNC | H5F_ACC_EXCL);
    unsigned openMode = openFlags & (H5F_ACC_RDWR | H5F_ACC_RDONLY);
    bool mustCreate = createMode > 0;
    bool openOrCreate = (openFlags & H5F_ACC_CREAT) > 0;

    if (!mustCreate) {
        std::unique_ptr<SilenceHDF5> silencer;
        if (openOrCreate)
            silencer.reset(new SilenceHDF5());

        _hid = detail::nothrow::h5f_open(_filename.c_str(), openMode, fileAccessProps.getId());

        if (isValid())
            return;

        if (openOrCreate) {
            createMode = H5F_ACC_EXCL;
        } else {
            HDF5ErrMapper::ToException<FileException>(
                std::string("Unable to open file " + _filename));
        }
    }

    auto fcpl = fileCreateProps.getId();
    auto fapl = fileAccessProps.getId();
    _hid = detail::h5f_create(_filename.c_str(), createMode, fcpl, fapl);
}

inline const std::string& File::getName() const {
    if (_filename.empty()) {
        _filename = details::get_name([this](char* buffer, size_t length) {
            return detail::h5f_get_name(getId(), buffer, length);
        });
    }
    return _filename;
}

inline hsize_t File::getMetadataBlockSize() const {
    auto fapl = getAccessPropertyList();
    return MetadataBlockSize(fapl).getSize();
}

inline std::pair<H5F_libver_t, H5F_libver_t> File::getVersionBounds() const {
    auto fapl = getAccessPropertyList();
    auto fileVer = FileVersionBounds(fapl);
    return fileVer.getVersion();
}

#if H5_VERSION_GE(1, 10, 1)
inline H5F_fspace_strategy_t File::getFileSpaceStrategy() const {
    auto fcpl = getCreatePropertyList();
    FileSpaceStrategy spaceStrategy(fcpl);
    return spaceStrategy.getStrategy();
}

inline hsize_t File::getFileSpacePageSize() const {
    auto fcpl = getCreatePropertyList();

    if (getFileSpaceStrategy() != H5F_FSPACE_STRATEGY_PAGE) {
        HDF5ErrMapper::ToException<FileException>(
            std::string("Cannot obtain page size as paged allocation is not used."));
    }

    return FileSpacePageSize(fcpl).getPageSize();
}
#endif

inline void File::flush() {
    detail::h5f_flush(_hid, H5F_SCOPE_GLOBAL);
}

inline size_t File::getFileSize() const {
    if (rest_vol_enabled()) {
        // The REST VOL plugin requires a flush to get the correct file size
        detail::h5f_flush(_hid, H5F_SCOPE_GLOBAL);
    }
    hsize_t sizeValue = 0;
    detail::h5f_get_filesize(_hid, &sizeValue);
    return static_cast<size_t>(sizeValue);
}

inline size_t File::getFreeSpace() const {
    return static_cast<size_t>(detail::h5f_get_freespace(_hid));
}

}  // namespace HighFive
