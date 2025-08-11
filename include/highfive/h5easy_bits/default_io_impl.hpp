/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include "../H5Easy.hpp"
#include "H5Easy_misc.hpp"
#include "H5Easy_scalar.hpp"

namespace H5Easy {
namespace detail {

using HighFive::details::inspector;

template <typename T>
struct default_io_impl {
    inline static std::vector<size_t> shape(const T& data) {
        return inspector<T>::getDimensions(data);
    }

    inline static DataSet dump(File& file,
                               const std::string& path,
                               const T& data,
                               const DumpOptions& options) {
        using value_type = typename inspector<T>::base_type;
        if (HighFive::rest_vol_enabled()) {
            if constexpr (std::is_same_v<T, std::string>) {
                if (file.exist(path) && !options.overwrite()) {
                    throw dump_error(file, path);
                }
                if (file.exist(path)) {
                    file.unlink(path);
                }
                return file.createDataSet(path, data);
            }
        }

        DataSet dataset = initDataset<value_type>(file, path, shape(data), options);
        dataset.write(data);
        if (options.flush()) {
            file.flush();
        }
        return dataset;
    }

    inline static T load(const File& file, const std::string& path) {
        return file.getDataSet(path).read<T>();
    }

    inline static Attribute dumpAttribute(File& file,
                                          const std::string& path,
                                          const std::string& key,
                                          const T& data,
                                          const DumpOptions& options) {
        using value_type = typename inspector<T>::base_type;
        if (HighFive::rest_vol_enabled()) {
            if constexpr (std::is_same_v<T, std::string>) {
                auto get_attribute = [&](auto& obj) {
                    if (!obj.hasAttribute(key)) {
                        return obj.createAttribute(key, data);
                    } else if (options.overwrite()) {
                        Attribute attribute = obj.getAttribute(key);
                        attribute.write(data);
                        DataSpace dataspace = attribute.getSpace();
                        if (dataspace.getElementCount() != 1) {
                            throw error(file,
                                        path,
                                        "H5Easy::dumpAttribute: Existing field not a scalar");
                        }
                        return attribute;
                    }
                    throw error(
                        file,
                        path,
                        "H5Easy: Attribute exists, overwrite with H5Easy::DumpMode::Overwrite.");
                };
                if (!file.exist(path)) {
                    throw error(file, path, "H5Easy::dumpAttribute: path does not exist");
                }

                return apply_attr_func(file, path, get_attribute);
            }
        }

        Attribute attribute = initAttribute<value_type>(file, path, key, shape(data), options);
        attribute.write(data);
        if (options.flush()) {
            file.flush();
        }
        return attribute;
    }

    inline static T loadAttribute(const File& file,
                                  const std::string& path,
                                  const std::string& key) {
        auto read_attribute = [&key](const auto& obj) {
            return obj.getAttribute(key).template read<T>();
        };

        return apply_attr_func(file, path, read_attribute);
    }
};

}  // namespace detail
}  // namespace H5Easy
