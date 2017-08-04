
//
//  Virtual file system.
//
//  Copyright (c) 2007 Waync Cheng.
//  All Rights Reserved.
//
//  2007/08/24 Waync created.
//

///
/// \file
/// \brief Virtual file system.
///
/// Archive module hides the detail of different file systems, support a simple
/// interface to access files. A file system can be a file folder of the native
/// file system or a Zip file or a Zip file in the memory.
///
/// Example:
///
/// \code
/// #include "swArchive.h"
///
/// //
/// // Allocate an interface of Archive instance, and use this interface to access files.
/// //
///
/// Archive* pItf = Archive::alloc();
/// if (0 == pItf)
/// { // Fail.
/// }
/// else
/// { // Success.
/// }
///
/// //
/// // Add new file system.
/// //
///
/// if (!pItf->addFileSystem("../data/")) // Add a folder.
/// { // Add fail.
/// }
///
/// if (!pItf->addFileSystem("media/model.zip")) // Add a Zip file.
/// { // Add fail.
/// }
///
/// if (!pItf->addFileSystem(stream))   // Add a Zip file stream in the memory.
/// { // Add fail.
/// }
///
/// //
/// // Load a file.
/// //
///
/// if (pItf->isExist("conf/config.ini"))
/// { // File is exist.
///    std::stringstream stream;        // The output buffer.
///    if (pItf->loadFile("conf/config.ini", stream))
///    { // Load success.  Now, the stream contains the content of "config.ini".
///    }
///    else
///    { // Load fail.
///    }
/// }
/// else
/// { // File is not exist.
/// }
///
/// //
/// // Release the instance after use.
/// //
///
/// Archive::free(pItf);
///
/// \endcode
///
/// \author Waync Cheng
/// \date 2007/08/24
///

#pragma once

#include "swinc.h"

namespace sw2 {

//
// \brief Virtual file system.
//

class ArchiveFileSystem
{
public:
  virtual ~ArchiveFileSystem() {}

  ///
  /// \brief Check is a file exist in the file system.
  /// \param [in] name Name of the file.
  /// \return Return true if the file is exist else return false.
  ///

  virtual bool isFileExist(std::string const& name) const=0;

  ///
  /// \brief Load a specified file.
  /// \param [in] name Name of the file.
  /// \param [out] outs The output of the file content(stream buffer).
  /// \param [in] password If not empty then use the password to decode the file.
  /// \return Return true if success else return false.
  /// \note The path of the file name is relative to the file system.
  /// \note If load successful, outs is a full copy of the file.
  ///

  virtual bool loadFile(std::string const& name, std::ostream& outs, std::string const& password) const=0;
};

///
/// \brief Virtual file system manager.
///

class Archive
{
public:

  ///
  /// \brief Allocate an interface of Archive instance.
  /// \return Return a valid interface pointer if success else return 0.
  ///

  static Archive* alloc();

  ///
  /// \brief Release a unused Archive instance.
  /// \param [in] pItf The instance to release.
  ///

  static void free(Archive* pItf);

  ///
  /// \brief Add a file system.
  /// \param [in] name Name of the file system.
  /// \return Return true if success else return false.
  /// \note A file system can be a folder, or a Zip file.
  ///

  virtual bool addFileSystem(std::string const& name)=0;

  ///
  /// \brief Add a memory file system.
  /// \param [in] stream The memory file system.
  /// \return Return true if success else return false.
  /// \note A memory file system can be a Zip file stream.
  /// \note If add successful, the memory file system is duplicated internally.
  ///

  virtual bool addFileSystem(std::istream& stream)=0;

  ///
  /// \brief Add a user define file system.
  /// \param [in] pFileSystem The user define file system.
  /// \return Return true if success else return false.
  ///

  virtual bool addFileSystem(ArchiveFileSystem *pFileSystem)=0;

  ///
  /// \brief Check is a file exist in the file system(s).
  /// \param [in] name Name of the file.
  /// \return Return true if the file is exist else return false.
  /// \note The search order is the reverse order of addition.
  ///

  virtual bool isFileExist(std::string const& name) const=0;

  ///
  /// \brief Load a specified file.
  /// \param [in] name Name of the file.
  /// \param [out] outs The output of the file content(stream buffer).
  /// \param [in] password If not empty then use the password to decode the file.
  /// \return Return true if success else return false.
  /// \note The path of the file name is relative to the file system.
  /// \note The search order is the reverse order of addition.
  /// \note If load successful, outs is a full copy of the file.
  ///

  virtual bool loadFile(std::string const& name, std::ostream& outs, std::string const& password = "") const=0;
};

} // namespace sw2

// end of swArchive.h
