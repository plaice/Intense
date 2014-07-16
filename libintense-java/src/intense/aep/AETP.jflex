/*
 ******************************************************************************

 AETP.jflex

 Lexer for AETPClient AEPClient.Tokens.

 Built with the following lines from the Intense Project Java distribution
 GNU Makefile:

 src/intense/aep/AETPLexer.java : src/intense/aep/AETP.jflex
        cd src/intense/aep; java -classpath ../../../ext/JFlex.jar JFlex.Main \
        AETP.jflex

 Copyright 2001, 2004 Paul Swoboda.

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


package intense.aep;


import java.io.*;
import intense.*;
import intense.log.*;


%%

%class AETPLexer
%buffer 1024
%implements intense.IntenseLexer
%function yylex
%type int
%yylexthrow AEPException
%eofval{
return lval.setEOF();
%eofval}
%8bit


%init{

startingNewState = false;

inDimension = false;

inContext = false;

integersType = SEQUENCE;

type = -1;

%init}


%{

private Context.Token lval;

private StringBuffer currentString;

private boolean stringBufferRequiresQuotes;

private boolean stringBufferIsAllIntChars;

private byte[] utf8Buffer = new byte[8];

private CompoundDimension compoundDimension;

private boolean startingNewState;

private boolean inDimension;

private boolean inContext;

public static final int SEQUENCE = 0;

public static final int ID = 1;

public static final int COUNT = 2;

private int integersType;

public static final int AETP_CLIENT_LEXER = 0;

public static final int AETP_SERVER_LEXER = 1;

private int type;

public void setClientType
()
{
  type = AETP_CLIENT_LEXER;
}

public void setServerType
()
{
  type = AETP_SERVER_LEXER;
}

public void setSequenceIntegersType
()
{
  integersType = SEQUENCE;
}

public void setIdIntegersType
()
{
  integersType = ID;
}

public void setCountIntegersType
()
{
  integersType = COUNT;
}

public void beginInContext
()
{
  startingNewState = true;
  inDimension = false;
  inContext = true;
}

public void beginInDimension
()
{
  startingNewState = true;
  inDimension = true;
  inContext = false;
}

public void beginInToken
()
{
  startingNewState = true;
  inDimension = false;
  inContext = false;
}

private void addToCompoundDimension
(Dimension newDimension)
{
  if (compoundDimension == null) {
    compoundDimension = new CompoundDimension();
  }
  compoundDimension.append(newDimension);
}

/**
 * Get a Context.Token.
 *
 * @param token A Context.Token to populate.
 * @return The type of the token.
 */
public int getToken
(Context.Token token)
  throws IOException, AEPException
{
  int returnValue;

  lval = token;
  if (startingNewState) {
    if (inDimension) {
      yybegin(IN_DIMENSION);
    } else if (inContext) {
      yybegin(IN_CONTEXT);
    } else {
      yybegin(IN_TOKEN);
    }
    startingNewState = false;
  }
  returnValue = yylex();
  if (returnValue == Context.Token.ERROR) {
    if (token.getValue() != null) {
      throw new AEPException(
        "Parsed error token in AETPLexer with content \"" +
        token.getValue() + "\""
      );
    } else {
      throw new AEPException(
        "Parsed error token in AETPLexer (null content)"
      );
    }
  }
  return returnValue;
}

%}


%xstate IN_TOKEN IN_DIMENSION IN_STRING AFTER_STRING
%xstate IN_CONTEXT IN_CONTEXT_BINARY_STRING


%%

<YYINITIAL,IN_TOKEN,IN_DIMENSION,IN_CONTEXT>[ \t\r\n]+ {}

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
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError("\\n");
  } else {
    return ((AETPServer.LexerToken)lval).setError("\\n");
  }
}

<IN_STRING>\t {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError("\\t");
  } else {
    return ((AETPServer.LexerToken)lval).setError("\\t");
  }
}

<IN_STRING>\013 {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError("\\v");
  } else {
    return ((AETPServer.LexerToken)lval).setError("\\v");
  }
}

<IN_STRING>\b {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError("\\b");
  } else {
    return ((AETPServer.LexerToken)lval).setError("\\b");
  }
}

<IN_STRING>\r {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError("\\r");
  } else {
    return ((AETPServer.LexerToken)lval).setError("\\r");
  }
}

<IN_STRING>\f {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError("\\f");
  } else {
    return ((AETPServer.LexerToken)lval).setError("\\f");
  }
}

<IN_STRING>\007 {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError("\\a");
  } else {
    return ((AETPServer.LexerToken)lval).setError("\\a");
  }
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
  if (inDimension) {
    yybegin(IN_DIMENSION);
    addToCompoundDimension(new StringDimension(
      currentString.toString(),
      stringBufferRequiresQuotes||stringBufferIsAllIntChars
    ));
  } else if (inContext) {
    yybegin(IN_CONTEXT);
    return lval.setStringDimension(new StringDimension(
      currentString.toString(),
      stringBufferRequiresQuotes||stringBufferIsAllIntChars
    ));
  } else {
    if (type == AETP_CLIENT_LEXER) {
      return ((AETPClient.LexerToken)lval).setError(":");
    } else {
      return ((AETPServer.LexerToken)lval).setError(":");
    }
  }
}

<AFTER_STRING>. {
  yypushback(1);
  if (inDimension) {
    // No ':', so that's the end of the compound dimension:
    CompoundDimension dimension = compoundDimension;

    yybegin(IN_TOKEN);
    inDimension = false;
    inContext = false;
    compoundDimension = null;
    if (type == AETP_CLIENT_LEXER) {
      return ((AETPClient.LexerToken)lval).setCompoundDimension(dimension);
    } else {
      return ((AETPServer.LexerToken)lval).setCompoundDimension(dimension);
    }
  } else if (inContext) {
    yybegin(IN_CONTEXT);
    return lval.setBaseValue(
      new StringBaseValue(currentString.toString())
    );
  } else {
    yybegin(IN_TOKEN);
    inDimension = false;
    inContext = false;
    if (type == AETP_CLIENT_LEXER) {
      return ((AETPClient.LexerToken)lval).setText(currentString.toString());
    } else {
      return ((AETPServer.LexerToken)lval).setText(currentString.toString());
    }
  }
}

<IN_TOKEN>\"\" {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setText("");
  } else {
    return ((AETPServer.LexerToken)lval).setText("");
  }
}

<IN_TOKEN>\" {
  currentString = new StringBuffer();
  stringBufferRequiresQuotes = false;
  stringBufferIsAllIntChars = true;
  yybegin(IN_STRING);
  continue;
}

<IN_TOKEN>\-?[0-9]+ {
  switch (integersType) {
  case SEQUENCE:
    try {
      if (type == AETP_CLIENT_LEXER) {
        return ((AETPClient.LexerToken)lval).setSequence(new Long(yytext()));
      } else {
        return ((AETPServer.LexerToken)lval).setSequence(new Long(yytext()));
      }
    } catch (NumberFormatException e) {
      throw new AEPException("Invalid sequence string \"" + yytext() + "\"");
    }
  case ID:
    try {
      if (type == AETP_CLIENT_LEXER) {
        return ((AETPClient.LexerToken)lval).setId(new Long(yytext()));
      } else {
        return ((AETPServer.LexerToken)lval).setId(new Long(yytext()));
      }
    } catch (NumberFormatException e) {
      throw new AEPException("Invalid id string \"" + yytext() + "\"");
    }
  case COUNT:
    try {
      if (type == AETP_CLIENT_LEXER) {
        return ((AETPClient.LexerToken)lval).setCount(new Integer(yytext()));
      } else {
        // Not yet used in server:
        return ((AETPServer.LexerToken)lval).setError(yytext());
      }
    } catch (NumberFormatException e) {
      throw new AEPException("Invalid count string \"" + yytext() + "\"");
    }
  }
}

<IN_DIMENSION>[0-9a-zA-Z_]*[a-zA-Z_][0-9a-zA-Z_]*: {
  addToCompoundDimension(new StringDimension(
    yytext().substring(0, yytext().length() - 1), false)
  );
}

<IN_DIMENSION>\-?[1-9][0-9]*: {
  int value;

  try {
    value = Integer.parseInt(yytext().substring(0, yytext().length() - 1));
  } catch (NumberFormatException e) {
    throw new AEPException("Invalid integer dimension \"" + yytext() + "\"");
  }
  addToCompoundDimension(new IntegerDimension(value));
}

<IN_DIMENSION>[0-9a-zA-Z_]*[a-zA-Z_][0-9a-zA-Z_]* {
  CompoundDimension dimension;

  addToCompoundDimension(new StringDimension(yytext(), false));
  dimension = compoundDimension;
  yybegin(IN_TOKEN);
  inDimension = false;
  compoundDimension = null;
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setCompoundDimension(dimension);
  } else {
    return ((AETPServer.LexerToken)lval).setCompoundDimension(dimension);
  }
}

<IN_DIMENSION>\-?[1-9][0-9]* {
  CompoundDimension dimension;
  int value;

  try {
    value = Integer.parseInt(yytext());
  } catch (NumberFormatException e) {
    throw new AEPException(
      "Invalid integer dimension \"" + yytext() + "\""
    );
  }
  addToCompoundDimension(new IntegerDimension(value));
  dimension = compoundDimension;
  yybegin(IN_TOKEN);
  inDimension = false;
  compoundDimension = null;
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setCompoundDimension(dimension);
  } else {
    return ((AETPServer.LexerToken)lval).setCompoundDimension(dimension);
  }
}

<IN_DIMENSION>\" {
  yybegin(IN_STRING);
}

<YYINITIAL>SYNCH {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  } else {
    yybegin(IN_TOKEN);
    return ((AETPServer.LexerToken)lval).setSynch();
  }
}

<YYINITIAL>DISCONNECT {
  yybegin(IN_TOKEN);
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setServerDisconnect();
  } else {
    return ((AETPServer.LexerToken)lval).setClientDisconnect();
  }
}

<YYINITIAL>NOTIFY {
  if (type == AETP_CLIENT_LEXER) {
    yybegin(IN_TOKEN);
    return ((AETPClient.LexerToken)lval).setNotify();
  } else {
    return ((AETPServer.LexerToken)lval).setError(yytext());
  }
}

<IN_TOKEN>NOTIFY {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  } else {
    return ((AETPServer.LexerToken)lval).setNotify();
  }
}

<YYINITIAL,IN_TOKEN>ASSIGN {
  yybegin(IN_TOKEN);
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setAssign();
  } else {
    return ((AETPServer.LexerToken)lval).setAssign();
  }
}

<YYINITIAL,IN_TOKEN>APPLY {
  yybegin(IN_TOKEN);
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setApply();
  } else {
    return ((AETPServer.LexerToken)lval).setApply();
  }
}

<YYINITIAL,IN_TOKEN>CLEAR {
  yybegin(IN_TOKEN);
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setClear();
  } else {
    return ((AETPServer.LexerToken)lval).setClear();
  }
}

<YYINITIAL,IN_TOKEN>KICK {
  yybegin(IN_TOKEN);
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setKick();
  } else {
    return ((AETPServer.LexerToken)lval).setError(yytext());
  }
}

<YYINITIAL>ACK {
  yybegin(IN_TOKEN);
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setAck();
  } else {
    return ((AETPServer.LexerToken)lval).setError(yytext());
  }
}

<YYINITIAL>DENY {
  yybegin(IN_TOKEN);
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setDeny();
  } else {
    return ((AETPServer.LexerToken)lval).setError(yytext());
  }
}

<IN_TOKEN,IN_CONTEXT>\; {
  yybegin(YYINITIAL);
  inDimension = false;
  inContext = false;
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setBreak();
  } else {
    return ((AETPServer.LexerToken)lval).setBreak();
  }
}

<YYINITIAL>JOIN {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  } else {
    yybegin(IN_TOKEN);
    return ((AETPServer.LexerToken)lval).setJoin();
  }
}

<YYINITIAL>LEAVE {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  } else {
    yybegin(IN_TOKEN);
    return ((AETPServer.LexerToken)lval).setLeave();
  }
}

<IN_TOKEN>CONTEXT {
  yybegin(IN_CONTEXT);
  // So we can reuse the string rules:
  inContext = true;
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  } else {
    return ((AETPServer.LexerToken)lval).setContextKeyword();
  }
}

<IN_TOKEN>OP {
  yybegin(IN_CONTEXT);
  // So we can reuse the string rules:
  inContext = true;
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  } else {
    return ((AETPServer.LexerToken)lval).setOpKeyword();
  }
}

<IN_TOKEN>DIM {
  yybegin(IN_DIMENSION);
  // So we can reuse the string rules:
  inDimension = true;
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setDimKeyword();
  } else {
    return ((AETPServer.LexerToken)lval).setDimKeyword();
  }
}

<IN_TOKEN>ROOT {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setRootKeyword();
  } else {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  }
}

<IN_TOKEN>EXT {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setExternalKeyword();
  } else {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  }
}

<IN_TOKEN>INT {
  if (type == AETP_CLIENT_LEXER) {
    return ((AETPClient.LexerToken)lval).setInternalKeyword();
  } else {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  }
}

<IN_TOKEN>PREFENCE {
  if (type == AETP_SERVER_LEXER) {
    return ((AETPServer.LexerToken)lval).setPreFence();
  } else {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  }
}

<IN_TOKEN>POSTFENCE {
  if (type == AETP_SERVER_LEXER) {
    return ((AETPServer.LexerToken)lval).setPostFence();
  } else {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  }
}

<IN_TOKEN>SELF {
  if (type == AETP_SERVER_LEXER) {
    return ((AETPServer.LexerToken)lval).setSelf();
  } else {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  }
}

<IN_TOKEN>CLIENT {
  if (type == AETP_SERVER_LEXER) {
    return ((AETPServer.LexerToken)lval).setClient();
  } else {
    return ((AETPClient.LexerToken)lval).setError(yytext());
  }
}

<YYINITIAL>[^ \-0-9A-Z\"\:\t\r\n]+ {
  return lval.setError(yytext());
}

<IN_CONTEXT>\"\" {
  return lval.setBaseValue(new StringBaseValue());
}

<IN_CONTEXT>\" {
  currentString = new StringBuffer();
  stringBufferRequiresQuotes = false;
  stringBufferIsAllIntChars = true;
  yybegin(IN_STRING);
}

<IN_CONTEXT>\# {
  currentString = new StringBuffer();
  yybegin(IN_CONTEXT_BINARY_STRING);
}

<IN_CONTEXT_BINARY_STRING>([0-9a-fA-F][0-9a-fA-F])+ {
  currentString.append(yytext());
}

<IN_CONTEXT_BINARY_STRING>[ \t\r\n]+ {}

<IN_CONTEXT_BINARY_STRING>[^0-9a-fA-F] {
  yypushback(1);
  yybegin(YYINITIAL);
  try {
    return lval.setBaseValue(
      BinaryBaseValue.hexStringToBinaryBaseValue(currentString.toString())
    );
  } catch (IntenseException e) {
    throw new AEPException(e);
  }
}

<IN_CONTEXT>\-?[1-9][0-9]*: {
  try {
    return lval.setIntegerDimension(new IntegerDimension(
      Integer.parseInt(yytext().substring(0, yytext().length() - 1))
    ));
  } catch (NumberFormatException e) {
    throw new AEPException(
      "Non-int-convertable IntegerDimension string \"" + yytext() + "\" in " +
      "Context/ContextOp parse"
    );
  }
}

<IN_CONTEXT>[^\"\#\\\n\t\013\b\r\f\007\~\^\-\<\>\[\]\+\: ]+: {
  return lval.setStringDimension(new StringDimension(
    yytext().substring(0, yytext().length() - 1), false
  ));
}

<IN_CONTEXT>\-?[0-9]+(\.[0-9]+)?([eE][\-\+]?[0-9]+)?[^0-9\.eE:] {
  yypushback(1);
  try {
    return lval.setBaseValue(new NumberBaseValue(
      Double.parseDouble(yytext())
    ));
  } catch (NumberFormatException e) {
    throw new AEPException(
      "Non-double-convertable NumberBaseValue string \"" + yytext() +
      "\" in Context/ContextOp parse"
    );
  }
}

<IN_CONTEXT>[^\"\#\\\n\t\013\b\r\f\007\~\^\-\<\>\[\]\+\: ]+ {
  return lval.setBaseValue(new StringBaseValue(yytext()));
}

<IN_CONTEXT>\~ {
  return lval.setBaseValue(new AlphaBaseValue());
}

<IN_CONTEXT>\^ {
  return lval.setBaseValue(new OmegaBaseValue());
}

<IN_CONTEXT>\- {
  return lval.setDash();
}

<IN_CONTEXT>\-\- {
  return lval.setDashDash();
}

<IN_CONTEXT>\-\-\- {
  return lval.setDashDashDash();
}

<IN_CONTEXT>\< {
  return lval.setLAngle();
}

<IN_CONTEXT>\> {
  return lval.setRAngle();
}

<IN_CONTEXT>\[ {
  return lval.setLSquare();
}

<IN_CONTEXT>\] {
  return lval.setRSquare();
}

<IN_CONTEXT>\+ {
  return lval.setPlus();
}

<IN_CONTEXT>[^\"\#\<\>\[\]\+\:\t\r\n]+ {
  return lval.setError(yytext());
}

<<EOF>> {
  return lval.setEOF();
}
