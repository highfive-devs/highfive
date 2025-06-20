/*
 *  Copyright (c), 2025, HighFive Developers
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <catch2/catch_template_test_macros.hpp>

#include <highfive/highfive.hpp>

using namespace HighFive;

template <typename T, bool T1, bool T2, bool T3, bool T4>
constexpr void check_constructible_assignable() {
    static_assert(std::is_copy_constructible<T>::value == T1, "Unexpected copy ctor");
    static_assert(std::is_copy_assignable<T>::value == T2, "Unexpected copy assignment");
    static_assert(std::is_move_constructible<T>::value == T3, "Unexpected move ctor");
    static_assert(std::is_move_assignable<T>::value == T4, "Unexpected move assignment");
}

TEST_CASE("Enshrine constructible/assignable status", "[core]") {
    // Object isn't constructible at all, because its dtor has
    // been deleted.
    check_constructible_assignable<Object, false, false, false, false>();
    check_constructible_assignable<File, true, true, true, true>();
    check_constructible_assignable<CompoundType, true, true, true, true>();
}

template <class T>
constexpr void check_nothrow_movable() {
    // Check that if it's moveable, it's nothrow movable.
    static_assert(std::is_move_constructible<T>::value ==
                      std::is_nothrow_move_constructible<T>::value,
                  "For regular classes the move ctor should be noexcept");
    static_assert(std::is_move_assignable<T>::value == std::is_nothrow_move_assignable<T>::value,
                  "For regular classes the move assignment should be noexcept");
}

TEST_CASE("Nothrow movable exceptions", "[core]") {
    check_nothrow_movable<Exception>();
    check_nothrow_movable<FileException>();
    check_nothrow_movable<ObjectException>();
    check_nothrow_movable<AttributeException>();
    check_nothrow_movable<DataSpaceException>();
    check_nothrow_movable<DataSetException>();
    check_nothrow_movable<GroupException>();
    check_nothrow_movable<PropertyException>();
    check_nothrow_movable<ReferenceException>();
    check_nothrow_movable<DataTypeException>();
}

template <class T>
constexpr void check_nothrow_object() {
    // HighFive objects are reference counted. During move assignment, the
    // reference count can drop to zero, triggering freeing of the object, which
    // can fail. Hence, move assignment can file, but move construction shouldn't.
    static_assert(std::is_move_constructible<T>::value ==
                      std::is_nothrow_move_constructible<T>::value,
                  "Move ctor not noexcept");
    static_assert(!std::is_nothrow_move_assignable<T>::value,
                  "Move assignment of HighFive::Object can't be noexcept");
    static_assert(!std::is_nothrow_copy_constructible<T>::value,
                  "Copy ctor should not be noexcept");
    static_assert(!std::is_nothrow_copy_assignable<T>::value,
                  "Copy assignment should not be noexcept");
}

TEST_CASE("HighFive::Objects are nothrow move constructible", "[core]") {
    check_nothrow_object<Object>();
    check_nothrow_object<File>();
    check_nothrow_object<Attribute>();
    check_nothrow_object<DataSpace>();
    check_nothrow_object<DataSet>();
    check_nothrow_object<Group>();
    check_nothrow_object<Selection>();
    check_nothrow_object<DataType>();
    check_nothrow_object<AtomicType<double>>();
    check_nothrow_object<CompoundType>();
    check_nothrow_object<StringType>();
}

TEST_CASE("Regular HighFive objects are nothrow movable", "[core]") {
    check_nothrow_movable<RegularHyperSlab>();
    check_nothrow_movable<HyperSlab>();
    check_nothrow_movable<ElementSet>();
    check_nothrow_movable<ProductSet>();
}
