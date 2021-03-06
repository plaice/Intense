// ****************************************************************************
//
// ContextOpLexer.hpp : A subclass of yyFlexLexer (generated by flex) for
// lexing ContextOps from strings.
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


namespace intense {


  class ContextOpLexer
    : public yyFlexLexer, public IntenseLexer {

    void* lval;

    std::ostringstream stringBuffer;

    bool stringBufferRequiresQuotes;

    bool stringBufferIsAllIntChars;

    char utf8Buffer[7];

  public:

    ContextOpLexer
    (std::istream& is);

    ~ContextOpLexer
    ();

    int getToken
    (Context::Token& token)
    {
      lval = (void*)&token;
      return contextop_yylex();
    }

  private:

    int contextop_yylex
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

  };


}
