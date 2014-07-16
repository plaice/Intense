// ****************************************************************************
//
// IntenseException.hpp : libintense exceptions.
//
// Copyright 2001, 2002 Paul Swoboda.
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


#ifndef __INTENSEEXCEPTION_HPP__
#define __INTENSEEXCEPTION_HPP__


namespace intense {


  class IntenseException
    : public std::runtime_error {

  public:

    IntenseException
    (const std::string& what)
      : std::runtime_error(what)
    {}

  };


  class BestFitException
    : public IntenseException {

  public:

    BestFitException
    (const std::string& what)
      : IntenseException(what)
    {}

  };


  class ContextParseException
    : public IntenseException {

  public:

    ContextParseException
    (const std::string& what)
      : IntenseException(what)
    {}

  };


  class ContextOpParseException
    : public IntenseException {

  public:

    ContextOpParseException
    (const std::string& what)
      : IntenseException(what)
    {}

  };


}


#endif //  __INTENSEEXCEPTION_HPP__
