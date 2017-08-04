
//
//  INI module.
//
//  Copyright (c) 2005 Waync Cheng.
//  All Rights Reserved.
//
//  2005/02/21 Waync created.
//

#include <limits.h>

#include <fstream>

#include "swIni.h"
#include "swUtil.h"

namespace sw2 {

namespace impl {

//
// Internal, helper.
//

bool getSectionName(std::string const& line, uint ln, std::string& section)
{
  //
  // Is section start?
  //

  uint cn = 0;
  if ('[' != line[0]) {
    SW2_TRACE_ERROR("Section start '[' expected at line %d #%d.", ln, cn);
    return false;
  }

  //
  // Get section name, process for each character.
  //

  for (cn = 1; cn < line.length(); cn++) {
    char ch = line[cn];                 // Read first char.
    if (']' == ch) {                    // Is section end?
      Util::trim(section);
      return true;                      // Skip remaining chars.
    }
    section += ch;
  }

  SW2_TRACE_ERROR("Section end ']' expected at line %d #%d.", ln, cn);

  return false;
}

bool getkeyValue(std::string const& line, uint ln, std::string& key, std::string& value)
{
  uint cn = 0;

  //
  // Get key.
  //

  for (; cn < line.length(); cn++) {
    char ch = line[cn];                 // Read 1st char.
    if ('=' == ch) {                    // End of key?
      Util::trim(key);
      break;
    }
    key += ch;
  }

  //
  // Validate.
  //

  if ('=' != line[cn]) {
    SW2_TRACE_ERROR("'=' expected at line %d #%d.", ln, cn);
    return false;
  }

  if (line.length() != cn) {
    cn += 1;
  }

  //
  // Get value.
  //

  for (; cn < line.length(); cn++) {    // Skip space.
    if (' ' != line[cn]) {
      break;
    }
  }

  if ('"' == line[cn]) {
    std::string::size_type pos = line.find_last_of('"');
    if (line.npos == pos) {
      SW2_TRACE_ERROR("Unmatch \"value\" %d.", ln);
      return false;
    }
    value = line.substr(cn + 1, pos - cn - 1);
  } else if ('\'' == line[cn]) {
    std::string::size_type pos = line.find_last_of('\'');
    if (line.npos == pos) {
      SW2_TRACE_ERROR("Unmatch \'value\' %d.", ln);
      return false;
    }
    value = line.substr(cn + 1, pos - cn - 1);
  } else {
    for (; cn < line.length(); cn++) {  // Process remain char(s).
      char ch = line[cn];               // Read first char.
      if (';' == ch) {                  // Skip comment.
        break;
      }
      value += ch;
    }
    Util::trim(value);
  }

  return true;
}

} // namespace impl

//
// swIni.
//

bool Ini::load(std::string const& fileName)
{
  std::ifstream inf(fileName.c_str());
  if (!inf) {
    return false;
  }

  clear();                              // Reset content.

  bool ret = load(inf);                 // Parse lines.
  inf.close();

  return ret;
}

bool Ini::load(std::istream& ins)
{
  //
  // Parse lines.
  //

  value_type line;

  uint ln = 0;
  while (getline(ins, line)) {

    ln += 1;

    if (Util::trim(line).empty()) {
      continue;
    }

    //
    // Is comment?
    //

    if (';' == line[0]) {
      continue;
    }

    //
    // Find section.
    //

sections:

    value_type section;
    if (!impl::getSectionName(line, ln, section)) {
      return false;
    }

    //
    // Insert new section.
    //

    (*this)[section];

    //
    // Loop for all section items.
    //

    while (getline(ins, line)) {

      ln += 1;
      if (Util::trim(line).empty()) {
        continue;
      }

      if (';' == line[0]) {             // Is comment?
        continue;
      }

      if ('[' == line[0]) {             // Section again?
        goto sections;
      }

      //
      // Find key-value pair.
      //

      value_type key, value;
      if (!impl::getkeyValue(line, ln, key, value)) {
        return false;
      }

      //
      // Insert new key-value.
      //

      (*this)[section][key] = value;
    }
  }

  return true;
}

bool Ini::store(std::string const& fileName) const
{
  std::ofstream outf(fileName.c_str());
  if (!outf) {
    return false;
  }

  bool ret = store(outf);
  outf.close();

  return ret;
}

bool Ini::store(std::ostream& outs) const
{
  for (int i = 0; i < size(); i++) {    // For each section.
    Ini const& sec = items[i];
    outs << "[" << sec.key << "]\n";
    for (int j = 0; j < sec.size(); j++) { // For each section item.
      Ini const& item = sec.items[j];
      if ('"' == item.value[0]) {
        outs << item.key << "='" << item.value << "'\n";
      } else if ('\'' == item.value[0]) {
        outs << item.key << "=\"" << item.value << "\"\n";
      } else if (!item.value.empty() &&
                  (' ' == item.value[0] ||
                   ' ' == item.value[item.value.size() - 1] ||
                   value_type::npos != item.value.find(';'))) {
        outs << item.key << "=\"" << item.value << "\"\n";
      } else {
        outs << item.key << "=" << item.value << "\n";
      }
    }
    outs << "\n";
  }

  return true;
}

void Ini::clear()
{
  items.clear();
  index.clear();
}

bool Ini::insert(value_type const& key)
{
  std::map<value_type, size_t>::iterator it = index.find(key);
  if (index.end() != it) {
    return true;
  }

  Ini ini;
  ini.key = key;
  items.push_back(ini);

  index[key] = items.size() - 1;

  return true;
}

bool Ini::remove(value_type const& key)
{
  std::map<value_type, size_t>::iterator it = index.find(key);
  if (index.end() == it) {
    return false;
  }

  items.erase(items.begin() + it->second);
  index.erase(it);

  return true;
}

Ini* Ini::find(value_type const& key)
{
  //
  // See Effective C++ 3rd, Ch1-Item3.
  //

  return const_cast<Ini*>(static_cast<Ini const*>(this)->find(key));
}

Ini const* Ini::find(value_type const& key) const
{
  std::map<value_type, size_t>::const_iterator it = index.find(key);
  if (index.end() == it) {
    return 0;
  }

  return &items[it->second];
}

Ini& Ini::operator[](char const* key)
{
  return operator[](value_type(key));
}

Ini& Ini::operator[](value_type const& key)
{
  insert(key);                          // Make sure item exists.
  return *find(key);
}

Ini const& Ini::operator[](char const* key) const
{
  return operator[](value_type(key));
}

Ini const& Ini::operator[](value_type const& key) const
{
  std::map<value_type, size_t>::const_iterator it = index.find(key);
  if (index.end() == it) {
    assert(0);
  }

  return items[it->second];
}

} // namespace sw2

// end of swIni.cpp
