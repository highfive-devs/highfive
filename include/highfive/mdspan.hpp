#pragma once

#include "bits/H5Inspector_decl.hpp"
#include "H5Exception.hpp"

#include <mdspan>
#include <vector>
#include <array>
#include <sstream>
#include <utility>
#include <type_traits>

namespace HighFive {
namespace details {

// Specialization for std::mdspan
template <class ElementType, class Extents, class LayoutPolicy, class AccessorPolicy>
struct inspector<std::mdspan<ElementType, Extents, LayoutPolicy, AccessorPolicy>> {
    using type = std::mdspan<ElementType, Extents, LayoutPolicy, AccessorPolicy>;
    using value_type = typename type::element_type;
    using base_type = typename inspector<value_type>::base_type;
    using hdf5_type = base_type;
    using extents_type = typename type::extents_type;
    using accessor_type = typename type::accessor_type;

    static constexpr size_t ndim = type::rank();
    static constexpr size_t min_ndim = ndim + inspector<value_type>::min_ndim;
    static constexpr size_t max_ndim = ndim + inspector<value_type>::max_ndim;

    static constexpr bool is_trivially_copyable =
        std::is_trivially_copyable<value_type>::value &&
        inspector<value_type>::is_trivially_nestable &&
        (std::is_same_v<std::default_accessor<value_type>, accessor_type>
#ifdef __cpp_lib_aligned_accessor
         || std::is_same_v<std::aligned_accessor<value_type>, accessor_type>
#endif
         ) &&
        (std::is_same_v<typename type::layout_type, std::layout_right> ||
         (std::is_same_v<typename type::layout_type, std::layout_left> && ndim == 1));
    static constexpr bool is_trivially_nestable = false;

  private:
    using index_type = typename extents_type::index_type;

    // Helper to access the first element (at index 0 in all dimensions)
    static auto get_first_element(const type& val) {
        std::array<index_type, ndim> indices{};
        return val[indices];
    }

    template <typename T>
    static auto data_impl(T& val) -> decltype(inspector<value_type>::data(*val.data_handle())) {
        if (!is_trivially_copyable) {
            throw DataSetException("Invalid use of `inspector<std::mdspan<...>>::data`.");
        }

        if (val.empty()) {
            return nullptr;
        }

        return inspector<value_type>::data(*val.data_handle());
    }

  public:
    static size_t getRank(const type& val) {
        if (val.empty()) {
            return min_ndim;
        }
        return ndim + inspector<value_type>::getRank(get_first_element(val));
    }

    static std::vector<size_t> getDimensions(const type& val) {
        std::vector<size_t> sizes;
        sizes.reserve(ndim);
        for (size_t r = 0; r < ndim; ++r) {
            sizes.push_back(val.extent(r));
        }
        if (!val.empty()) {
            auto s = inspector<value_type>::getDimensions(get_first_element(val));
            sizes.insert(sizes.end(), s.begin(), s.end());
        }
        return sizes;
    }

    static void prepare(type& val, const std::vector<size_t>& dims) {
        if (dims.size() < ndim) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims.size()
               << " dimensions into an mdspan with rank " << ndim << ".";
            throw DataSpaceException(os.str());
        }

        // Check that dimensions match
        for (size_t r = 0; r < ndim; ++r) {
            if (dims[r] != val.extent(r)) {
                std::ostringstream os;
                os << "Mismatching dimensions for mdspan: expected " << val.extent(r)
                   << " for dimension " << r << ", but got " << dims[r] << ".";
                throw DataSpaceException(os.str());
            }
        }
    }

    static hdf5_type* data(type& val) {
        return data_impl(val);
    }

    static const hdf5_type* data(const type& val) {
        return data_impl(val);
    }

    static void serialize(const type& val, const std::vector<size_t>& dims, hdf5_type* m) {
        auto subdims = std::vector<size_t>(dims.begin() + ndim, dims.end());
        auto subsize = compute_total_size(subdims);

        std::array<index_type, ndim> indices{};
        auto iterate = [&](auto& self, size_t dim) -> void {
            if (dim == ndim) {
                // Base case: serialize element
                inspector<value_type>::serialize(val[indices], subdims, m);
                m += subsize;
            } else {
                // Recursive case: iterate over current dimension
                const auto n = static_cast<index_type>(val.extent(dim));
                for (indices[dim] = 0; indices[dim] < n; ++indices[dim]) {
                    self(self, dim + 1);
                }
            }
        };

        iterate(iterate, 0);
    }

    static void unserialize(const hdf5_type* vec_align,
                            const std::vector<size_t>& dims,
                            type& val) {
        if (dims.size() < ndim) {
            std::ostringstream os;
            os << "Impossible to pair DataSet with " << dims.size()
               << " dimensions into an mdspan with rank " << ndim << ".";
            throw DataSpaceException(os.str());
        }

        auto subdims = std::vector<size_t>(dims.begin() + ndim, dims.end());
        auto subsize = compute_total_size(subdims);

        std::array<index_type, ndim> indices{};
        auto iterate = [&](auto& self, size_t dim) -> void {
            if (dim == ndim) {
                // Base case: unserialize element
                inspector<value_type>::unserialize(vec_align, subdims, val[indices]);
                vec_align += subsize;
            } else {
                // Recursive case: iterate over current dimension
                const auto n = static_cast<index_type>(dims[dim]);
                for (indices[dim] = 0; indices[dim] < n; ++indices[dim]) {
                    self(self, dim + 1);
                }
            }
        };

        iterate(iterate, 0);
    }
};

}  // namespace details
}  // namespace HighFive
