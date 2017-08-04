
//
//  INI module.
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/02/21 Waync created.
//

///
/// \file
/// \brief INI module.
///
/// INI file is a text file for configuration, can be edited by any text editor.
/// The content of a INI file is consist of SECTIONs, each SECTION is consist of
/// ITEMs. And each ITEM is a key=value pair.
///
/// Format:
///
/// - SECTION is consist of '[' name ']': [section].
/// - ITEM is consist of a key and value pair, seperate with '=': key=value;
///   another format is key="value" or key='value' so spaces can include in the value.
/// - Comment All characters after ';' are treated as comment and been discard.
/// - Space before and after SECTION name and key/value of ITEM will been trimed.
/// - SECTION name and key/value of ITEM can use BIG5 code.
///
/// Example:
///
/// \code
/// #include "swIni.h"
///
/// Ini ini;
/// ini.load("test.ini");               // Load test.ini.
///
/// ini["Sec3"]["a"] = 1;               // Insert an int.
/// ini["Sec3"]["b"] = 3.14f;           // Insert a float.
/// ini["Sec3"]["c"] = 1.1111;          // Insert a double.
/// ini["Sec3"]["d"] = "test";          // Insert a string.
/// ini["Sec3"]["e"] = false;           // Insert a bool.
///
/// int a = ini["Sec3"]["a"];           // Read an int.
/// float b = ini["Sec3"]["b"];         // Read a float.
/// double c = ini["Sec3"]["c"];        // Read a double.
/// std::string d = ini["Sec3"]["d"];   // Read a sring.
/// bool e = ini["Sec3"]["e"];          // Read a bool.
///
/// ini.store("test2.ini");             // Save to test2.ini.
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2005/02/21
///

#pragma once

#include <map>
#include <sstream>
#include <vector>

#include "swinc.h"

namespace sw2 {

///
/// \brief INI module.
///

class Ini
{
public:

  typedef std::string value_type;

  ///
  /// \brief Apply a value.
  ///

  template<class T>
  Ini& operator=(T v)
  {
    std::stringstream ss;
    ss << v;
    value = ss.str();
    return *this;
  }

  ///
  /// \brief Get a value.
  ///

  template<class T>
  operator T() const
  {
    std::stringstream ss(value);
    T v = T();
    ss >> v;
    return v;
  }

  operator value_type() const
  {
    return value;
  }

  ///
  /// \brief Load INI from a file.
  /// \param [in] fileName Source file name.
  /// \return Return true if success else return false.
  ///

  bool load(std::string const& fileName);

  ///
  /// \brief Load INI from a stream.
  /// \param [in] ins Source stream.
  /// \return Return true if success else return false.
  ///

  bool load(std::istream& ins);

  ///
  /// \brief Save INI to a file.
  /// \param [in] fileName Destination file name.
  /// \return Return true if success else return false.
  ///

  bool store(std::string const& fileName) const;

  ///
  /// \brief Save INI to a stream.
  /// \param [in] outs Destination stream.
  /// \return Return true if success else return false.
  ///

  bool store(std::ostream& outs) const;

  ///
  /// \brief Get item count.
  /// \return Return item count.
  ///

  int size() const
  {
    return (int)items.size();
  }

  ///
  /// \brief Clear all items.
  ///

  void clear();

  ///
  /// \brief Insert a new item.
  /// \param [in] key Item key name.
  /// \return Return true if success else return false.
  ///

  bool insert(value_type const& key);

  ///
  /// \brief Remove an item.
  /// \param [in] key Item key.
  /// \return Return true if success else return false.
  ///

  bool remove(value_type const& key);

  ///
  /// \brief Find an item.
  /// \param [in] key Item key.
  /// \return Return the item pointer if success else return 0.
  ///

  Ini* find(value_type const& key);
  Ini const* find(value_type const& key) const;

  ///
  /// \brief Find an item.
  /// \param [in] key Item key.
  /// \return Return the item reference if success else throw an assertion.
  /// \note If the item is not exist then a new item is inserted and return to
  ///        the caller. const version will throw an assertion.
  ///

  Ini& operator[](char const* key);
  Ini& operator[](value_type const& key);

  Ini const& operator[](char const* key) const;
  Ini const& operator[](value_type const& key) const;

  value_type key;                       ///< key.
  value_type value;                     ///< value.

  std::vector<Ini> items;               ///< items.

  std::map<value_type, size_t> index;   // Index map for performance.
};

} // namespace sw2

// end of swIni.h
