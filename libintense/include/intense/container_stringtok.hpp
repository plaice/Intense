// ****************************************************************************
//
// container_stringtok.hpp : A string tokenizer template function, taken
// entirely from the stdc++ docs :-).
//
// This file is part of the Intense project.
//
// Intense is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// Intense is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Intense; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
//
// ****************************************************************************


#ifndef __CONTAINER_STRINGTOK_HPP__
#define __CONTAINER_STRINGTOK_HPP__


#include <string>


template <typename Container>
void stringtok
(Container &container, std::string const &in,
 const char * const delimiters = " \t\r\n")
{
  const std::string::size_type len = in.length();
  std::string::size_type i = 0;

  while (i < len) {
    // eat leading whitespace
    i = in.find_first_not_of(delimiters, i);
    if (i == std::string::npos) {
      return; // nothing left but white space
    }
    // find the end of the token:
    std::string::size_type j = in.find_first_of(delimiters, i);
    // push token
    if (j == std::string::npos) {
      container.push_back(in.substr(i));
      return;
    } else {
      container.push_back(in.substr(i, j-i));
    }
    // set up for next loop
    i = j + 1;
  }
}


#endif // __CONTAINER_STRINGTOK_H__
