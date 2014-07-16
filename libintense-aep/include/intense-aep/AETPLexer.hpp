// ****************************************************************************
//
// AETPLexer.hpp : Lexer for AEP 2.0 AETP (text-mode) client and server,
// for the abstract/core AEP implementation.
//
// Copyright 2001, 2002, 2004 Paul Swoboda.
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


#ifndef __AETPLEXER_HPP__
#define __AETPLEXER_HPP__

namespace intense {


  namespace aep {


    class AETPLexer
      : public yyFlexLexer, public IntenseLexer {

    protected:

      void* lval;

      std::ostringstream stringBuffer;

      bool stringBufferRequiresQuotes;

      bool stringBufferIsAllIntChars;

      char utf8Buffer[7];

      CompoundDimension* compoundDimension;

      bool startingNewState;

      bool inDimension;

      bool inContext;

    public:

      typedef enum { SEQUENCE, ID, COUNT } IntegersType;

      IntegersType integersType;

      typedef enum { AETP_CLIENT_LEXER, AETP_SERVER_LEXER } Type;

    protected:

      Type type;

    public:

      AETPLexer
      (std::istream& is, Type type_);

      virtual ~AETPLexer
      ();

      int getToken
      (Context::Token& token);

      void beginInContext
      ()
      {
        startingNewState = true;
        inDimension = false;
        inContext = true;
      }

      void beginInDimension
      ()
      {
        startingNewState = true;
        inDimension = true;
        inContext = false;
      }

      void beginInToken
      ()
      {
        startingNewState = true;
        inDimension = false;
        inContext = false;
      }

    private:

      int aetp_yylex
      ();

      void stringReset
      ()
      {
        stringBuffer.str("");
        stringBufferRequiresQuotes = false;
        stringBufferIsAllIntChars = true;
      }

      void addToString
      (const char* stringToAdd)
      {
        stringBuffer << stringToAdd;
      }

      void addToBinaryString
      (const char* stringToAdd)
      {
        stringBuffer << stringToAdd;
      }

      BinaryBaseValue* stringToBinaryBaseValue
      ();

      void resetCompoundDimension
      ()
      {
        compoundDimension = NULL;
      }

      void addToCompoundDimension
      (Dimension& newDimension)
      {
        if (compoundDimension == NULL) {
          compoundDimension = new CompoundDimension;
        }
        compoundDimension->append(DimensionRef(newDimension));
      }

    };


  }


}


#endif // __AETPLEXER_HPP__
