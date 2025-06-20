#pragma once

#include <H5Opublic.h>

namespace HighFive {
namespace detail {

inline hid_t h5o_open(hid_t loc_id, const char* name, hid_t lapl_id) {
    hid_t hid = H5Oopen(loc_id, name, lapl_id);
    if (hid < 0) {
        HDF5ErrMapper::ToException<GroupException>(std::string("Unable to open \"") + name + "\":");
    }

    return hid;
}

inline herr_t h5o_close(hid_t id) {
    herr_t err = H5Oclose(id);
    if (err < 0) {
        HDF5ErrMapper::ToException<ObjectException>("Unable to close object.");
    }

    return err;
}

#if H5O_info_t_vers >= 1
using h5o_info1_t = H5O_info1_t;
#else
using h5o_info1_t = H5O_info_t;
#endif

inline herr_t h5o_get_info1(hid_t loc_id, h5o_info1_t* info) {
#if H5Oget_info_vers >= 1
    herr_t err = H5Oget_info1(loc_id, info);
#else
    herr_t err = H5Oget_info(loc_id, info);
#endif

    if (err < 0) {
        HDF5ErrMapper::ToException<ObjectException>("Unable to obtain info for object");
    }
    return err;
}

#if H5_VERSION_GE(1, 10, 3)
inline herr_t h5o_get_info2(hid_t loc_id, h5o_info1_t* info, unsigned fields) {
    herr_t err = H5Oget_info2(loc_id, info, fields);
    if (err < 0) {
        HDF5ErrMapper::ToException<ObjectException>("Unable to obtain info for object");
    }
    return err;
}
#endif

#if H5_VERSION_GE(1, 12, 0)
inline herr_t h5o_get_info3(hid_t loc_id, H5O_info2_t* info, unsigned fields) {
    herr_t err = H5Oget_info3(loc_id, info, fields);
    if (err < 0) {
        HDF5ErrMapper::ToException<ObjectException>("Unable to obtain info for object");
    }
    return err;
}
#endif

}  // namespace detail
}  // namespace HighFive
