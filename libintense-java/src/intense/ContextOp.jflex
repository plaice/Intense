/*
 ******************************************************************************

 ContextOp.jflex

 This is the jflex input file for the ContextOp lexer.

 Built with the following lines from the Intense Project Java distribution
 GNU Makefile:

 src/intense/ContextOpLexer.java : src/intense/ContextOp.jflex
        cd src/intense; java -classpath ../../ext/JFlex.jar JFlex.Main \
        ContextOp.jflex

 Copyright 2001 by Paul Swoboda.  All Rights Reserved.

 This file is part of the Intense project.

 Intense is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 Intense is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Intense; if not, write to the Free Software Foundation, Inc.,
 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.

 ******************************************************************************
 */


package intense;


import java.io.*;


%%

%class ContextOpLexer
%implements intense.IntenseLexer
%function yylex
%type int
%yylexthrow IntenseException
%eofval{
return lval.setEOF();
%eofval}
%8bit

%{

private Context.Token lval;

private StringBuffer currentString;

private boolean stringBufferRequiresQuotes;

private boolean stringBufferIsAllIntChars;

private byte[] utf8Buffer = new byte[8];

/**
 * Get a Context.Token.
 *
 * @param token A Context.Token to populate.
 * @return The type of the token.
 */
public int getToken
(Context.Token token)
  throws IOException, IntenseException
{
  lval = token;
  return yylex();
}

%}

%state IN_STRING AFTER_STRING IN_BINARY_STRING

%%

<YYINITIAL> [ \t\r\n]+ {}

<YYINITIAL>\/\/[^\n]*\n {}

<IN_STRING>\\. {
  char escapedChar = yytext().charAt(1);
  switch (escapedChar) {
  case 'n':
    currentString.append('\n');
    break;
  case 't':
    currentString.append('\t');
    break;
  case 'b':
    currentString.append('\b');
    break;
  case 'r':
    currentString.append('\r');
    break;
  case 'f':
    currentString.append('\f');
    break;
  case 'v':
    currentString.append('\013');
    break;
  case 'a':
    currentString.append('\007');
    break;
  case '\\':
    currentString.append('\\');
    break;
  case '"':
    currentString.append('\"');
    break;
  default:
    currentString.append(escapedChar);
    break;
  }
  stringBufferRequiresQuotes = true;
  stringBufferIsAllIntChars = false;
}

<IN_STRING>\\U(00)?(00)?(00)?((0[789a-dA-D])|(5[cC])|(22)|(23)|(20)|(7[eE])|(5[eE])|(2[dD])|(3[cC])|(3[eE])|(5[bB])|(5[dD])|(2[bB])|(3[aA])) {
  // Reserved Characters:
  try {
    int length = StringUtil.utf8EncodeHex(
      utf8Buffer, 0, yytext().getBytes(), yytext().length() - 2, 1
    );

    currentString.append(new String(utf8Buffer, 0, length, "UTF-8"));
    stringBufferRequiresQuotes = true;
    stringBufferIsAllIntChars = false;
  } catch (Exception e) {
    return lval.setError(
      "INTERNAL ERROR: Bad UTF-8 string " + yytext() + " : " + e.getMessage()
    );
  }
}

<IN_STRING>\\U(00)?(00)?(00)?(3[0-9]) {
  try {
    int length = StringUtil.utf8EncodeHex(
      utf8Buffer, 0, yytext().getBytes(), yytext().length() - 2, 1
    );

    currentString.append(new String(utf8Buffer, 0, length, "UTF-8"));
  } catch (Exception e) {
    return lval.setError(
      "INTERNAL ERROR: Bad UTF-8 string " + yytext() + " : " + e.getMessage()
    );
  }
}

<IN_STRING>\\U[a-fA-F0-9][a-fA-F0-9] {
  try {
    int length = StringUtil.utf8EncodeHex(
      utf8Buffer, 0, yytext().getBytes(), 2, 1
    );

    currentString.append(new String(utf8Buffer, 0, length, "UTF-8"));
    stringBufferIsAllIntChars = false;
  } catch (Exception e) {
    return lval.setError(
      "INTERNAL ERROR: Bad UTF-8 string " + yytext() + " : " + e.getMessage()
    );
  }
}

<IN_STRING>\\U[a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9] {
  try {
    int length = StringUtil.utf8EncodeHex(
      utf8Buffer, 0, yytext().getBytes(), 2, 2
    );

    currentString.append(new String(utf8Buffer, 0, length, "UTF-8"));
    stringBufferIsAllIntChars = false;
  } catch (Exception e) {
    return lval.setError(
      "INTERNAL ERROR: Bad UTF-8 string " + yytext() + " : " + e.getMessage()
    );
  }
}

<IN_STRING>\\U[a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9] {
  try {
    int length = StringUtil.utf8EncodeHex(
      utf8Buffer, 0, yytext().getBytes(), 2, 3
    );

    currentString.append(new String(utf8Buffer, 0, length, "UTF-8"));
    stringBufferIsAllIntChars = false;
  } catch (Exception e) {
    return lval.setError(
      "INTERNAL ERROR: Bad UTF-8 string " + yytext() + " : " + e.getMessage()
    );
  }
}

<IN_STRING>\\U[a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9][a-fA-F0-9]
{
  try {
    int length = StringUtil.utf8EncodeHex(
      utf8Buffer, 0, yytext().getBytes(), 2, 4
    );

    currentString.append(new String(utf8Buffer, 0, length, "UTF-8"));
    stringBufferIsAllIntChars = false;
  } catch (Exception e) {
    return lval.setError(
      "INTERNAL ERROR: Bad UTF-8 string " + yytext() + " : " + e.getMessage()
    );
  }
}

<IN_STRING>\n {
  return lval.setError("\\n");
}

<IN_STRING>\t {
  return lval.setError("\\t");
}

<IN_STRING>\013 {
  return lval.setError("\\v");
}

<IN_STRING>\b {
  return lval.setError("\\b");
}

<IN_STRING>\r {
  return lval.setError("\\r");
}

<IN_STRING>\f {
  return lval.setError("\\f");
}

<IN_STRING>\007 {
  return lval.setError("\\a");
}

<IN_STRING>\" {
  yybegin(AFTER_STRING);
}

<IN_STRING>\040 {
  currentString.append(' ');
  stringBufferRequiresQuotes = true;
  stringBufferIsAllIntChars = false;
}

<IN_STRING>[\#\~\^\-\<\>\[\]\+\: ]+ {
  currentString.append(yytext());
  stringBufferRequiresQuotes = true;
  stringBufferIsAllIntChars = false;
}

<IN_STRING>[0-9]+ {
  currentString.append(yytext());
}

<IN_STRING>[^\"\#\\\n\t\013\b\r\f\007\~\^\-\<\>\[\]\+\: ]+ {
  currentString.append(yytext());
  stringBufferIsAllIntChars = false;
}

<AFTER_STRING>[ \t\r\n]+ {}

<AFTER_STRING>\" {
  yybegin(IN_STRING);
}

<AFTER_STRING>: {
  yybegin(YYINITIAL);
  return lval.setStringDimension(new StringDimension(
    currentString.toString(),
    stringBufferRequiresQuotes||stringBufferIsAllIntChars
  ));
}

<AFTER_STRING>. {
  yypushback(1);
  yybegin(YYINITIAL);
  return lval.setBaseValue(new StringBaseValue(currentString.toString()));
}

<YYINITIAL>\"\" {
  return lval.setBaseValue(new StringBaseValue());
}

<YYINITIAL>\" {
  currentString = new StringBuffer();
  stringBufferRequiresQuotes = false;
  stringBufferIsAllIntChars = true;
  yybegin(IN_STRING);
}

<YYINITIAL>\# {
  currentString = new StringBuffer();
  yybegin(IN_BINARY_STRING);
}

<IN_BINARY_STRING>([0-9a-fA-F][0-9a-fA-F])+ {
  currentString.append(yytext());
}

<IN_BINARY_STRING>[ \t\r\n]+ {}

<IN_BINARY_STRING>[^0-9a-fA-F] {
  yypushback(1);
  yybegin(YYINITIAL);
  return lval.setBaseValue(
    BinaryBaseValue.hexStringToBinaryBaseValue(currentString.toString())
  );
}

<YYINITIAL>\-?[1-9][0-9]*: {
  try {
    return lval.setIntegerDimension(new IntegerDimension(
      Integer.parseInt(yytext().substring(0, yytext().length() - 1))
    ));
  } catch (NumberFormatException e) {
    throw new IntenseException(
      "Non-int-convertable IntegerDimension string \"" + yytext() + "\" in " +
      "Context/ContextOp parse"
    );
  }
}

<YYINITIAL>[^\"\#\\\n\t\013\b\r\f\007\~\^\-\<\>\[\]\+\: ]+: {
  return lval.setStringDimension(new StringDimension(
    yytext().substring(0, yytext().length() - 1), false
  ));
}

<YYINITIAL>\-?[0-9]+(\.[0-9]+)?([eE][\-\+]?[0-9]+)?[^0-9\.eE:] {
  yypushback(1);
  try {
    return lval.setBaseValue(new NumberBaseValue(
      Double.parseDouble(yytext())
    ));
  } catch (NumberFormatException e) {
    throw new IntenseException(
      "Non-double-convertable NumberBaseValue string \"" + yytext() +
      "\" in Context/ContextOp parse"
    );
  }
}

<YYINITIAL>[^\"\#\\\n\t\013\b\r\f\007\~\^\-\<\>\[\]\+\: ]+ {
  return lval.setBaseValue(new StringBaseValue(yytext()));
}

<YYINITIAL>\~ {
  return lval.setBaseValue(new AlphaBaseValue());
}

<YYINITIAL>\^ {
  return lval.setBaseValue(new OmegaBaseValue());
}

<YYINITIAL>\- {
  return lval.setDash();
}

<YYINITIAL>\-\- {
  return lval.setDashDash();
}

<YYINITIAL>\-\-\- {
  return lval.setDashDashDash();
}

<YYINITIAL>\< {
  return lval.setLAngle();
}

<YYINITIAL>\> {
  return lval.setRAngle();
}

<YYINITIAL>\[ {
  return lval.setLSquare();
}

<YYINITIAL>\] {
  return lval.setRSquare();
}

<YYINITIAL>\+ {
  return lval.setPlus();
}

<YYINITIAL>[^\-0-9\/\"\#\<\>\[\]\+\:\t\r\n]+ {
  return lval.setError(yytext());
}

<YYINITIAL><<EOF>> {
  return lval.setEOF();
}

<IN_STRING><<EOF>> {
  return lval.setEOF();
}

<AFTER_STRING><<EOF>> {
  return lval.setEOF();
}

<IN_BINARY_STRING><<EOF>> {
  return lval.setEOF();
}
