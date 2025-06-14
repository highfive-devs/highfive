/*
 *  Copyright (c), 2017, Adrien Devresse <adrien.devresse@epfl.ch>
 *
 *  Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#pragma once

#include <string>

#include "../H5PropertyList.hpp"
#include "H5_definitions.hpp"
#include "H5Converter_misc.hpp"

namespace HighFive {

enum class IndexType : std::underlying_type<H5_index_t>::type {
    NAME = H5_INDEX_NAME,
    CRT_ORDER = H5_INDEX_CRT_ORDER,
};

///
/// \brief NodeTraits: Base class for Group and File
///
template <typename Derivate>
class NodeTraits {
  public:
    ///
    /// \brief createDataSet Create a new dataset in the current file of
    /// datatype type and of size space
    /// \param dataset_name identifier of the dataset
    /// \param space Associated DataSpace, see \ref DataSpace for more information
    /// \param type Type of Data
    /// \param createProps A property list with data set creation properties
    /// \param accessProps A property list with data set access properties
    /// \param parents Create intermediate groups if needed. Default: true.
    /// \return DataSet Object
    DataSet createDataSet(const std::string& dataset_name,
                          const DataSpace& space,
                          const DataType& type,
                          const DataSetCreateProps& createProps = DataSetCreateProps::Default(),
                          const DataSetAccessProps& accessProps = DataSetAccessProps::Default(),
                          bool parents = true);

    ///
    /// \brief createDataSet create a new dataset in the current file with a
    /// size specified by space
    /// \param dataset_name identifier of the dataset
    /// \param space Associated DataSpace, see \ref DataSpace for more information
    /// \param createProps A property list with data set creation properties
    /// \param accessProps A property list with data set access properties
    /// \param parents Create intermediate groups if needed. Default: true.
    /// \return DataSet Object
    template <typename T>
    DataSet createDataSet(const std::string& dataset_name,
                          const DataSpace& space,
                          const DataSetCreateProps& createProps = DataSetCreateProps::Default(),
                          const DataSetAccessProps& accessProps = DataSetAccessProps::Default(),
                          bool parents = true);

    ///
    /// \brief createDataSet create a new dataset in the current file and
    /// write to it, inferring the DataSpace from the data.
    /// \param dataset_name identifier of the dataset
    /// \param data Associated data, must support DataSpace::From, see
    /// \ref DataSpace for more information
    /// \param createProps A property list with data set creation properties
    /// \param accessProps A property list with data set access properties
    /// \param parents Create intermediate groups if needed. Default: true.
    /// \return DataSet Object
    template <typename T>
    DataSet createDataSet(const std::string& dataset_name,
                          const T& data,
                          const DataSetCreateProps& createProps = DataSetCreateProps::Default(),
                          const DataSetAccessProps& accessProps = DataSetAccessProps::Default(),
                          bool parents = true);


    ///
    /// \brief get an existing dataset in the current file
    /// \param dataset_name
    /// \param accessProps property list to configure dataset chunk cache
    /// \return return the named dataset, or throw exception if not found
    DataSet getDataSet(const std::string& dataset_name,
                       const DataSetAccessProps& accessProps = DataSetAccessProps::Default()) const;

    ///
    /// \brief create a new group, and eventually intermediate groups
    /// \param group_name
    /// \param parents Create intermediate groups if needed. Default: true.
    /// \return the group object
    Group createGroup(const std::string& group_name, bool parents = true);

    ///
    /// \brief create a new group, and eventually intermediate groups
    /// \param group_name
    /// \param createProps A property list with group creation properties
    /// \param parents Create intermediate groups if needed. Default: true.
    /// \return the group object
    Group createGroup(const std::string& group_name,
                      const GroupCreateProps& createProps,
                      bool parents = true);

    ///
    /// \brief open an existing group with the name group_name
    /// \param group_name
    /// \return the group object
    Group getGroup(const std::string& group_name) const;

    ///
    /// \brief open a commited datatype with the name type_name
    /// \param type_name
    /// \return the datatype object
    DataType getDataType(
        const std::string& type_name,
        const DataTypeAccessProps& accessProps = DataTypeAccessProps::Default()) const;

    ///
    /// \brief return the number of leaf objects of the node / group
    /// \return number of leaf objects
    size_t getNumberObjects() const;

    ///
    /// \brief return the name of the object with the given index
    /// \return the name of the object
    std::string getObjectName(size_t index) const;

    ///
    /// \brief moves an object and its content within an HDF5 file.
    /// \param src_path relative path of the object to current File/Group
    /// \param dst_path new relative path of the object to current File/Group
    /// \param parents Create intermediate groups if needed. Default: true.
    /// \return boolean that is true if the move was successful
    bool rename(const std::string& src_path,
                const std::string& dst_path,
                bool parents = true) const;

    ///
    /// \brief list all leaf objects name of the node / group
    /// \param idx_type tell if the list should be ordered by Name or CreationOrderTime.
    /// CreationOrderTime can be use only if the file/group has been created with
    /// the HighFive::LinkCreationTime property.
    /// \return number of leaf objects
    std::vector<std::string> listObjectNames(IndexType idx_type = IndexType::NAME) const;

    ///
    /// \brief check a dataset or group exists in the current node / group
    /// \param node_path dataset/group name to check
    /// \return true if a dataset/group with the associated name exists, or false
    bool exist(const std::string& node_path) const;

    ///
    /// \brief unlink the given dataset or group
    /// \param node_path dataset/group name to unlink
    void unlink(const std::string& node_path) const;

    ///
    /// \brief Returns the kind of link of the given name (soft, hard...)
    /// \param node_path The entry to check, path relative to the current group
    LinkType getLinkType(const std::string& node_path) const;

    ///
    /// \brief A shorthand to get the kind of object pointed to (group, dataset, type...)
    /// \param node_path The entry to check, path relative to the current group
    ObjectType getObjectType(const std::string& node_path) const;

    ///
    /// \brief A shorthand to create softlink to any object which provides `getPath`
    /// The link will be created with default properties along with required parent groups
    template <typename T, typename = decltype(&T::getPath)>
    void createSoftLink(const std::string& linkName, const T& obj) {
        static_assert(!std::is_same<T, Attribute>::value,
                      "hdf5 doesn't support soft links to Attributes");
        createSoftLink(linkName, obj.getPath());
    }

    ///
    /// \brief Creates softlinks
    /// \param link_name The name of the link
    /// \param obj_path The target object path
    /// \param linkCreateProps A Link_Create property list. Notice "parents=true" overrides
    /// \param linkAccessProps The Link_Access property list
    /// \param parents Whether parent groups should be created: Default: true
    void createSoftLink(const std::string& link_name,
                        const std::string& obj_path,
                        LinkCreateProps linkCreateProps = LinkCreateProps(),
                        const LinkAccessProps& linkAccessProps = LinkAccessProps(),
                        bool parents = true);

    void createExternalLink(const std::string& link_name,
                            const std::string& h5_file,
                            const std::string& obj_path,
                            LinkCreateProps linkCreateProps = LinkCreateProps(),
                            const LinkAccessProps& linkAccessProps = LinkAccessProps(),
                            bool parents = true);

    ///
    /// \brief Creates hardlinks
    /// \param link_name The name of the link
    /// \param target_obj The target object
    /// \param linkCreateProps A Link_Create property list. Notice "parents=true" overrides
    /// \param linkAccessProps The Link_Access property list
    /// \param parents Whether parent groups should be created: Default: true
    template <typename T, typename = decltype(&T::getPath)>
    void createHardLink(const std::string& link_name,
                        const T& target_obj,
                        LinkCreateProps linkCreateProps = LinkCreateProps(),
                        const LinkAccessProps& linkAccessProps = LinkAccessProps(),
                        bool parents = true);

  private:
    using derivate_type = Derivate;

    // A wrapper over the low-level H5Lexist
    // It makes behavior consistent among versions and by default transforms
    // errors to exceptions
    bool _exist(const std::string& node_path, bool raise_errors = true) const;
};


///
/// \brief The possible types of group entries (link concept)
///
enum class LinkType {
    Hard,
    Soft,
    External,
    Other  // Reserved or User-defined
};


}  // namespace HighFive
