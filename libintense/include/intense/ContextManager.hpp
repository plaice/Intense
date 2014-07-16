// ****************************************************************************
//
// ContextManager.hpp : C++ global context manager for libintense.
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


#ifndef __CONTEXTMANAGER_HPP__
#define __CONTEXTMANAGER_HPP__


namespace intense {


  class ContextManager {

  protected:

    static ContextManager* instancePtr;

    Context context;

    ContextManager
    ()
      : context()
    {}

  public:

    virtual ~ContextManager
    ()
    {}

    // Singleton accessor:
    static ContextManager& instance
    ();

    // Current context accessor:
    virtual Context& currentContext
    ();

  };


}


#define CURRENT ContextManager::instance().currentContext()


#endif //  __CONTEXTMANAGER_HPP__
