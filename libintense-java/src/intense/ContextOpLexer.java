/* The following code was generated by JFlex 1.4 on 10/5/04 2:06 PM */

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



/**
 * This class is a scanner generated by 
 * <a href="http://www.jflex.de/">JFlex</a> 1.4
 * on 10/5/04 2:06 PM from the specification file
 * <tt>ContextOp.jflex</tt>
 */
class ContextOpLexer implements intense.IntenseLexer {

  /** This character denotes the end of file */
  public static final int YYEOF = -1;

  /** initial size of the lookahead buffer */
  private static final int ZZ_BUFFERSIZE = 16384;

  /** lexical states */
  public static final int IN_BINARY_STRING = 3;
  public static final int AFTER_STRING = 2;
  public static final int IN_STRING = 1;
  public static final int YYINITIAL = 0;

  /** 
   * Translates characters to character classes
   */
  private static final char [] ZZ_CMAP = {
     0,  0,  0,  0,  0,  0,  0, 23, 20, 18,  3, 19, 22, 21,  0,  0, 
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
     1,  0, 24, 27,  0,  0,  0,  0,  0,  0,  0, 30,  0, 28, 29,  2, 
     6, 16, 10, 11, 16,  8, 16, 12,  7,  7, 26,  0, 33,  0, 34,  0, 
     0, 15, 14,  9, 14, 13, 17,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
     0,  0,  0,  0,  0,  5,  0,  0,  0,  0,  0, 35,  4, 25, 32,  0, 
     0, 15, 14,  9, 14, 13, 17,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 31,  0, 
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 
     0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
  };

  /** 
   * Translates DFA states to action switch labels.
   */
  private static final int [] ZZ_ACTION = zzUnpackAction();

  private static final String ZZ_ACTION_PACKED_0 =
    "\4\0\1\1\1\2\1\1\1\2\1\3\2\1\1\4"+
    "\1\5\1\6\1\7\1\10\1\11\1\12\1\13\1\14"+
    "\1\15\1\16\1\17\1\20\1\0\1\21\1\22\1\23"+
    "\1\24\1\25\1\26\1\27\1\30\1\31\1\32\1\33"+
    "\1\34\1\35\1\0\1\1\1\36\1\1\2\37\2\1"+
    "\1\40\1\41\2\0\1\42\2\43\1\21\1\0\1\2"+
    "\1\36\1\1\1\0\1\1\2\0\1\44\10\0\1\45"+
    "\1\46\1\45\1\47\6\0\1\50\1\46\1\50\1\47"+
    "\6\0\1\51\1\46\1\51\1\47\6\0\1\52\1\46"+
    "\1\47";

  private static int [] zzUnpackAction() {
    int [] result = new int[104];
    int offset = 0;
    offset = zzUnpackAction(ZZ_ACTION_PACKED_0, offset, result);
    return result;
  }

  private static int zzUnpackAction(String packed, int offset, int [] result) {
    int i = 0;       /* index in packed string  */
    int j = offset;  /* index in unpacked array */
    int l = packed.length();
    while (i < l) {
      int count = packed.charAt(i++);
      int value = packed.charAt(i++);
      do result[j++] = value; while (--count > 0);
    }
    return j;
  }


  /** 
   * Translates a state to a row index in the transition table
   */
  private static final int [] ZZ_ROWMAP = zzUnpackRowMap();

  private static final String ZZ_ROWMAP_PACKED_0 =
    "\0\0\0\44\0\110\0\154\0\220\0\264\0\330\0\374"+
    "\0\u0120\0\u0144\0\u0168\0\u018c\0\u01b0\0\u01b0\0\u01d4\0\u01b0"+
    "\0\u0120\0\u0120\0\u01b0\0\u01b0\0\u01b0\0\u01f8\0\u021c\0\u01b0"+
    "\0\u0240\0\u0264\0\u01b0\0\u01b0\0\u01b0\0\u01b0\0\u01b0\0\u01b0"+
    "\0\u01b0\0\u021c\0\u01b0\0\u01b0\0\u01b0\0\u01b0\0\u0288\0\u02ac"+
    "\0\u01b0\0\u02d0\0\u02ac\0\u01b0\0\u02f4\0\u0318\0\u01b0\0\u01b0"+
    "\0\u033c\0\u0360\0\u0384\0\u01b0\0\u03a8\0\u03cc\0\u03f0\0\u01b0"+
    "\0\u03f0\0\u0414\0\u0438\0\u045c\0\u0480\0\u04a4\0\u01b0\0\u04c8"+
    "\0\u04ec\0\u0510\0\u0534\0\u0558\0\u057c\0\u05a0\0\u05c4\0\u05e8"+
    "\0\u060c\0\u060c\0\u060c\0\u0630\0\u0654\0\u0678\0\u069c\0\u06c0"+
    "\0\u06e4\0\u0708\0\u072c\0\u072c\0\u072c\0\u0750\0\u0774\0\u0798"+
    "\0\u07bc\0\u07e0\0\u0804\0\u0828\0\u084c\0\u084c\0\u084c\0\u0870"+
    "\0\u0894\0\u08b8\0\u08dc\0\u0900\0\u0924\0\u01b0\0\u01b0\0\u01b0";

  private static int [] zzUnpackRowMap() {
    int [] result = new int[104];
    int offset = 0;
    offset = zzUnpackRowMap(ZZ_ROWMAP_PACKED_0, offset, result);
    return result;
  }

  private static int zzUnpackRowMap(String packed, int offset, int [] result) {
    int i = 0;  /* index in packed string  */
    int j = offset;  /* index in unpacked array */
    int l = packed.length();
    while (i < l) {
      int high = packed.charAt(i++) << 16;
      result[j++] = high | packed.charAt(i++);
    }
    return j;
  }

  /** 
   * The transition table of the DFA
   */
  private static final int [] ZZ_TRANS = zzUnpackTrans();

  private static final String ZZ_TRANS_PACKED_0 =
    "\1\5\1\6\1\7\1\10\1\11\1\5\1\12\2\13"+
    "\1\5\3\13\3\5\1\13\1\5\1\10\2\11\1\10"+
    "\2\11\1\14\1\15\1\0\1\16\1\17\1\5\1\20"+
    "\1\21\1\22\1\23\1\24\1\25\1\26\1\27\1\26"+
    "\1\30\1\31\1\26\3\32\1\26\3\32\3\26\1\32"+
    "\1\26\1\33\1\34\1\35\1\36\1\37\1\40\1\41"+
    "\4\42\1\26\6\42\1\43\1\10\1\43\1\10\16\43"+
    "\1\10\2\43\1\10\2\43\1\44\1\43\1\45\11\43"+
    "\1\46\1\10\1\46\1\10\2\46\14\47\1\10\2\46"+
    "\1\10\16\46\1\5\1\11\1\50\1\0\1\11\1\5"+
    "\3\50\1\5\3\50\3\5\1\50\1\5\1\0\2\11"+
    "\1\0\2\11\2\0\1\51\2\0\1\5\1\0\2\11"+
    "\3\0\1\11\1\6\1\0\1\10\2\11\3\0\1\11"+
    "\3\0\3\11\1\0\1\11\1\10\2\11\1\10\2\11"+
    "\5\0\1\11\1\0\2\11\3\0\1\50\1\0\1\52"+
    "\2\0\15\50\10\0\1\51\2\0\1\50\7\0\1\10"+
    "\1\0\1\10\16\0\1\10\2\0\1\10\16\0\2\11"+
    "\2\0\2\11\3\0\1\11\3\0\3\11\1\0\1\11"+
    "\1\0\2\11\1\0\2\11\5\0\1\11\1\0\2\11"+
    "\3\0\1\53\1\54\1\53\2\54\1\53\3\12\1\53"+
    "\3\12\1\55\2\53\1\12\1\53\10\54\1\51\2\54"+
    "\1\56\6\54\1\53\1\54\1\53\2\54\1\53\3\13"+
    "\1\53\3\13\1\55\2\53\1\13\1\53\10\54\1\57"+
    "\2\54\1\56\6\54\30\0\1\60\65\0\1\61\2\62"+
    "\1\0\3\62\3\0\1\62\13\0\1\63\7\0\1\26"+
    "\1\0\1\26\2\0\15\26\13\0\1\26\7\0\1\42"+
    "\27\0\4\42\1\0\6\42\3\64\1\0\1\64\1\65"+
    "\36\64\1\26\1\0\1\26\2\0\1\26\3\32\1\26"+
    "\3\32\3\26\1\32\1\26\13\0\1\26\14\0\14\66"+
    "\22\0\1\50\1\0\1\50\2\0\15\50\10\0\1\51"+
    "\2\0\1\50\6\0\1\52\1\67\1\52\1\70\1\67"+
    "\15\52\10\67\1\71\2\67\1\52\6\67\1\50\1\0"+
    "\1\50\2\0\1\50\3\72\1\50\3\72\3\50\1\72"+
    "\1\50\10\0\1\51\1\0\1\73\1\50\1\73\5\0"+
    "\1\50\1\0\1\50\2\0\1\50\3\74\1\50\3\74"+
    "\3\50\1\74\1\50\10\0\1\51\2\0\1\50\6\0"+
    "\6\54\3\61\1\54\3\61\1\75\2\54\1\61\11\54"+
    "\1\0\2\54\1\76\14\54\3\62\1\54\3\62\1\75"+
    "\2\54\1\62\11\54\1\57\2\54\1\76\6\54\34\0"+
    "\1\77\15\0\1\100\1\101\1\102\1\101\1\103\1\104"+
    "\1\105\5\101\30\0\14\47\22\0\3\67\1\70\40\67"+
    "\1\53\1\54\1\53\2\54\1\53\3\72\1\53\3\72"+
    "\1\50\2\53\1\72\1\53\10\54\1\51\2\54\1\50"+
    "\6\54\6\0\3\106\1\0\3\106\3\0\1\106\23\0"+
    "\1\53\1\54\1\53\2\54\1\53\3\74\1\53\3\74"+
    "\1\55\2\53\1\74\1\53\10\54\1\51\2\54\1\50"+
    "\6\54\6\0\3\106\1\0\3\106\3\0\1\106\13\0"+
    "\1\73\1\0\1\73\13\0\3\107\1\0\3\107\3\0"+
    "\1\107\31\0\1\110\1\111\1\112\1\111\2\112\1\111"+
    "\1\112\2\111\2\112\30\0\14\112\30\0\3\112\1\111"+
    "\3\112\2\111\3\112\30\0\1\111\3\112\2\111\2\112"+
    "\1\111\3\112\30\0\3\113\1\111\3\113\1\111\1\112"+
    "\1\111\1\113\1\112\30\0\7\112\1\111\4\112\22\0"+
    "\6\54\3\106\1\54\3\106\1\0\2\54\1\106\11\54"+
    "\1\0\2\54\1\0\14\54\3\107\1\54\3\107\1\75"+
    "\2\54\1\107\11\54\1\0\2\54\1\0\6\54\6\0"+
    "\1\114\1\115\1\116\1\115\1\117\1\120\1\121\5\115"+
    "\30\0\14\115\30\0\1\122\1\123\1\124\1\123\2\124"+
    "\1\123\1\124\2\123\2\124\30\0\14\124\30\0\3\124"+
    "\1\123\3\124\2\123\3\124\30\0\1\123\3\124\2\123"+
    "\2\124\1\123\3\124\30\0\3\125\1\123\3\125\1\123"+
    "\1\124\1\123\1\125\1\124\30\0\7\124\1\123\4\124"+
    "\30\0\1\126\1\127\1\130\1\127\1\131\1\132\1\133"+
    "\5\127\30\0\14\127\30\0\1\134\1\135\1\136\1\135"+
    "\2\136\1\135\1\136\2\135\2\136\30\0\14\136\30\0"+
    "\3\136\1\135\3\136\2\135\3\136\30\0\1\135\3\136"+
    "\2\135\2\136\1\135\3\136\30\0\3\137\1\135\3\137"+
    "\1\135\1\136\1\135\1\137\1\136\30\0\7\136\1\135"+
    "\4\136\30\0\1\140\1\141\1\142\1\141\1\143\1\144"+
    "\1\145\5\141\30\0\14\141\30\0\1\146\1\147\1\146"+
    "\1\147\2\146\1\147\1\146\2\147\2\146\30\0\14\146"+
    "\30\0\3\146\1\147\3\146\2\147\3\146\30\0\1\147"+
    "\3\146\2\147\2\146\1\147\3\146\30\0\3\150\1\147"+
    "\3\150\1\147\1\146\1\147\1\150\1\146\30\0\7\146"+
    "\1\147\4\146\22\0";

  private static int [] zzUnpackTrans() {
    int [] result = new int[2376];
    int offset = 0;
    offset = zzUnpackTrans(ZZ_TRANS_PACKED_0, offset, result);
    return result;
  }

  private static int zzUnpackTrans(String packed, int offset, int [] result) {
    int i = 0;       /* index in packed string  */
    int j = offset;  /* index in unpacked array */
    int l = packed.length();
    while (i < l) {
      int count = packed.charAt(i++);
      int value = packed.charAt(i++);
      value--;
      do result[j++] = value; while (--count > 0);
    }
    return j;
  }


  /* error codes */
  private static final int ZZ_UNKNOWN_ERROR = 0;
  private static final int ZZ_NO_MATCH = 1;
  private static final int ZZ_PUSHBACK_2BIG = 2;

  /* error messages for the codes above */
  private static final String ZZ_ERROR_MSG[] = {
    "Unkown internal scanner error",
    "Error: could not match input",
    "Error: pushback value was too large"
  };

  /**
   * ZZ_ATTRIBUTE[aState] contains the attributes of state <code>aState</code>
   */
  private static final int [] ZZ_ATTRIBUTE = zzUnpackAttribute();

  private static final String ZZ_ATTRIBUTE_PACKED_0 =
    "\4\0\10\1\2\11\1\1\1\11\2\1\3\11\2\1"+
    "\1\11\1\0\1\1\7\11\1\1\4\11\1\0\1\1"+
    "\1\11\2\1\1\11\2\1\2\11\2\0\1\1\1\11"+
    "\2\1\1\0\1\11\2\1\1\0\1\1\2\0\1\11"+
    "\10\0\4\1\6\0\4\1\6\0\4\1\6\0\3\11";

  private static int [] zzUnpackAttribute() {
    int [] result = new int[104];
    int offset = 0;
    offset = zzUnpackAttribute(ZZ_ATTRIBUTE_PACKED_0, offset, result);
    return result;
  }

  private static int zzUnpackAttribute(String packed, int offset, int [] result) {
    int i = 0;       /* index in packed string  */
    int j = offset;  /* index in unpacked array */
    int l = packed.length();
    while (i < l) {
      int count = packed.charAt(i++);
      int value = packed.charAt(i++);
      do result[j++] = value; while (--count > 0);
    }
    return j;
  }

  /** the input device */
  private java.io.Reader zzReader;

  /** the current state of the DFA */
  private int zzState;

  /** the current lexical state */
  private int zzLexicalState = YYINITIAL;

  /** this buffer contains the current text to be matched and is
      the source of the yytext() string */
  private char zzBuffer[] = new char[ZZ_BUFFERSIZE];

  /** the textposition at the last accepting state */
  private int zzMarkedPos;

  /** the textposition at the last state to be included in yytext */
  private int zzPushbackPos;

  /** the current text position in the buffer */
  private int zzCurrentPos;

  /** startRead marks the beginning of the yytext() string in the buffer */
  private int zzStartRead;

  /** endRead marks the last character in the buffer, that has been read
      from input */
  private int zzEndRead;

  /** number of newlines encountered up to the start of the matched text */
  private int yyline;

  /** the number of characters up to the start of the matched text */
  private int yychar;

  /**
   * the number of characters from the last newline up to the start of the 
   * matched text
   */
  private int yycolumn;

  /** 
   * zzAtBOL == true <=> the scanner is currently at the beginning of a line
   */
  private boolean zzAtBOL = true;

  /** zzAtEOF == true <=> the scanner is at the EOF */
  private boolean zzAtEOF;

  /* user code: */

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



  /**
   * Creates a new scanner
   * There is also a java.io.InputStream version of this constructor.
   *
   * @param   in  the java.io.Reader to read input from.
   */
  ContextOpLexer(java.io.Reader in) {
    this.zzReader = in;
  }

  /**
   * Creates a new scanner.
   * There is also java.io.Reader version of this constructor.
   *
   * @param   in  the java.io.Inputstream to read input from.
   */
  ContextOpLexer(java.io.InputStream in) {
    this(new java.io.InputStreamReader(in));
  }


  /**
   * Refills the input buffer.
   *
   * @return      <code>false</code>, iff there was new input.
   * 
   * @exception   java.io.IOException  if any I/O-Error occurs
   */
  private boolean zzRefill() throws java.io.IOException {

    /* first: make room (if you can) */
    if (zzStartRead > 0) {
      System.arraycopy(zzBuffer, zzStartRead,
                       zzBuffer, 0,
                       zzEndRead-zzStartRead);

      /* translate stored positions */
      zzEndRead-= zzStartRead;
      zzCurrentPos-= zzStartRead;
      zzMarkedPos-= zzStartRead;
      zzPushbackPos-= zzStartRead;
      zzStartRead = 0;
    }

    /* is the buffer big enough? */
    if (zzCurrentPos >= zzBuffer.length) {
      /* if not: blow it up */
      char newBuffer[] = new char[zzCurrentPos*2];
      System.arraycopy(zzBuffer, 0, newBuffer, 0, zzBuffer.length);
      zzBuffer = newBuffer;
    }

    /* finally: fill the buffer with new input */
    int numRead = zzReader.read(zzBuffer, zzEndRead,
                                            zzBuffer.length-zzEndRead);

    if (numRead < 0) {
      return true;
    }
    else {
      zzEndRead+= numRead;
      return false;
    }
  }

    
  /**
   * Closes the input stream.
   */
  public final void yyclose() throws java.io.IOException {
    zzAtEOF = true;            /* indicate end of file */
    zzEndRead = zzStartRead;  /* invalidate buffer    */

    if (zzReader != null)
      zzReader.close();
  }


  /**
   * Resets the scanner to read from a new input stream.
   * Does not close the old reader.
   *
   * All internal variables are reset, the old input stream 
   * <b>cannot</b> be reused (internal buffer is discarded and lost).
   * Lexical state is set to <tt>ZZ_INITIAL</tt>.
   *
   * @param reader   the new input stream 
   */
  public final void yyreset(java.io.Reader reader) {
    zzReader = reader;
    zzAtBOL  = true;
    zzAtEOF  = false;
    zzEndRead = zzStartRead = 0;
    zzCurrentPos = zzMarkedPos = zzPushbackPos = 0;
    yyline = yychar = yycolumn = 0;
    zzLexicalState = YYINITIAL;
  }


  /**
   * Returns the current lexical state.
   */
  public final int yystate() {
    return zzLexicalState;
  }


  /**
   * Enters a new lexical state
   *
   * @param newState the new lexical state
   */
  public final void yybegin(int newState) {
    zzLexicalState = newState;
  }


  /**
   * Returns the text matched by the current regular expression.
   */
  public final String yytext() {
    return new String( zzBuffer, zzStartRead, zzMarkedPos-zzStartRead );
  }


  /**
   * Returns the character at position <tt>pos</tt> from the 
   * matched text. 
   * 
   * It is equivalent to yytext().charAt(pos), but faster
   *
   * @param pos the position of the character to fetch. 
   *            A value from 0 to yylength()-1.
   *
   * @return the character at position pos
   */
  public final char yycharat(int pos) {
    return zzBuffer[zzStartRead+pos];
  }


  /**
   * Returns the length of the matched text region.
   */
  public final int yylength() {
    return zzMarkedPos-zzStartRead;
  }


  /**
   * Reports an error that occured while scanning.
   *
   * In a wellformed scanner (no or only correct usage of 
   * yypushback(int) and a match-all fallback rule) this method 
   * will only be called with things that "Can't Possibly Happen".
   * If this method is called, something is seriously wrong
   * (e.g. a JFlex bug producing a faulty scanner etc.).
   *
   * Usual syntax/scanner level error handling should be done
   * in error fallback rules.
   *
   * @param   errorCode  the code of the errormessage to display
   */
  private void zzScanError(int errorCode) {
    String message;
    try {
      message = ZZ_ERROR_MSG[errorCode];
    }
    catch (ArrayIndexOutOfBoundsException e) {
      message = ZZ_ERROR_MSG[ZZ_UNKNOWN_ERROR];
    }

    throw new Error(message);
  } 


  /**
   * Pushes the specified amount of characters back into the input stream.
   *
   * They will be read again by then next call of the scanning method
   *
   * @param number  the number of characters to be read again.
   *                This number must not be greater than yylength()!
   */
  public void yypushback(int number)  {
    if ( number > yylength() )
      zzScanError(ZZ_PUSHBACK_2BIG);

    zzMarkedPos -= number;
  }


  /**
   * Resumes scanning until the next regular expression is matched,
   * the end of input is encountered or an I/O-Error occurs.
   *
   * @return      the next token
   * @exception   java.io.IOException  if any I/O-Error occurs
   */
  public int yylex() throws java.io.IOException, IntenseException {
    int zzInput;
    int zzAction;

    // cached fields:
    int zzCurrentPosL;
    int zzMarkedPosL;
    int zzEndReadL = zzEndRead;
    char [] zzBufferL = zzBuffer;
    char [] zzCMapL = ZZ_CMAP;

    int [] zzTransL = ZZ_TRANS;
    int [] zzRowMapL = ZZ_ROWMAP;
    int [] zzAttrL = ZZ_ATTRIBUTE;

    while (true) {
      zzMarkedPosL = zzMarkedPos;

      zzAction = -1;

      zzCurrentPosL = zzCurrentPos = zzStartRead = zzMarkedPosL;
  
      zzState = zzLexicalState;


      zzForAction: {
        while (true) {
    
          if (zzCurrentPosL < zzEndReadL)
            zzInput = zzBufferL[zzCurrentPosL++];
          else if (zzAtEOF) {
            zzInput = YYEOF;
            break zzForAction;
          }
          else {
            // store back cached positions
            zzCurrentPos  = zzCurrentPosL;
            zzMarkedPos   = zzMarkedPosL;
            boolean eof = zzRefill();
            // get translated positions and possibly new buffer
            zzCurrentPosL  = zzCurrentPos;
            zzMarkedPosL   = zzMarkedPos;
            zzBufferL      = zzBuffer;
            zzEndReadL     = zzEndRead;
            if (eof) {
              zzInput = YYEOF;
              break zzForAction;
            }
            else {
              zzInput = zzBufferL[zzCurrentPosL++];
            }
          }
          int zzNext = zzTransL[ zzRowMapL[zzState] + zzCMapL[zzInput] ];
          if (zzNext == -1) break zzForAction;
          zzState = zzNext;

          int zzAttributes = zzAttrL[zzState];
          if ( (zzAttributes & 1) == 1 ) {
            zzAction = zzState;
            zzMarkedPosL = zzCurrentPosL;
            if ( (zzAttributes & 8) == 8 ) break zzForAction;
          }

        }
      }

      // store back cached position
      zzMarkedPos = zzMarkedPosL;

      switch (zzAction < 0 ? zzAction : ZZ_ACTION[zzAction]) {
        case 10: 
          { return lval.setBaseValue(new OmegaBaseValue());
          }
        case 43: break;
        case 1: 
          { return lval.setBaseValue(new StringBaseValue(yytext()));
          }
        case 44: break;
        case 24: 
          { yybegin(AFTER_STRING);
          }
        case 45: break;
        case 19: 
          { return lval.setError("\\v");
          }
        case 46: break;
        case 29: 
          { yypushback(1);
  yybegin(YYINITIAL);
  return lval.setBaseValue(
    BinaryBaseValue.hexStringToBinaryBaseValue(currentString.toString())
  );
          }
        case 47: break;
        case 7: 
          { return lval.setDash();
          }
        case 48: break;
        case 34: 
          { return lval.setDashDash();
          }
        case 49: break;
        case 11: 
          { return lval.setLAngle();
          }
        case 50: break;
        case 18: 
          { return lval.setError("\\t");
          }
        case 51: break;
        case 41: 
          { try {
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
        case 52: break;
        case 30: 
          { return lval.setStringDimension(new StringDimension(
    yytext().substring(0, yytext().length() - 1), false
  ));
          }
        case 53: break;
        case 9: 
          { return lval.setBaseValue(new AlphaBaseValue());
          }
        case 54: break;
        case 14: 
          { currentString.append(yytext());
  stringBufferIsAllIntChars = false;
          }
        case 55: break;
        case 15: 
          { currentString.append(' ');
  stringBufferRequiresQuotes = true;
  stringBufferIsAllIntChars = false;
          }
        case 56: break;
        case 31: 
          { yypushback(1);
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
        case 57: break;
        case 21: 
          { return lval.setError("\\r");
          }
        case 58: break;
        case 23: 
          { return lval.setError("\\a");
          }
        case 59: break;
        case 38: 
          { // Reserved Characters:
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
        case 60: break;
        case 22: 
          { return lval.setError("\\f");
          }
        case 61: break;
        case 37: 
          { try {
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
        case 62: break;
        case 35: 
          { char escapedChar = yytext().charAt(1);
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
        case 63: break;
        case 3: 
          { return lval.setError(yytext());
          }
        case 64: break;
        case 28: 
          { yybegin(YYINITIAL);
  return lval.setStringDimension(new StringDimension(
    currentString.toString(),
    stringBufferRequiresQuotes||stringBufferIsAllIntChars
  ));
          }
        case 65: break;
        case 39: 
          { try {
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
        case 66: break;
        case 42: 
          { try {
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
        case 67: break;
        case 25: 
          { currentString.append(yytext());
  stringBufferRequiresQuotes = true;
  stringBufferIsAllIntChars = false;
          }
        case 68: break;
        case 13: 
          { return lval.setLSquare();
          }
        case 69: break;
        case 27: 
          { yybegin(IN_STRING);
          }
        case 70: break;
        case 12: 
          { return lval.setRAngle();
          }
        case 71: break;
        case 16: 
          { return lval.setError("\\n");
          }
        case 72: break;
        case 6: 
          { currentString = new StringBuffer();
  yybegin(IN_BINARY_STRING);
          }
        case 73: break;
        case 36: 
          { return lval.setDashDashDash();
          }
        case 74: break;
        case 8: 
          { return lval.setPlus();
          }
        case 75: break;
        case 17: 
          { currentString.append(yytext());
          }
        case 76: break;
        case 26: 
          { yypushback(1);
  yybegin(YYINITIAL);
  return lval.setBaseValue(new StringBaseValue(currentString.toString()));
          }
        case 77: break;
        case 4: 
          { currentString = new StringBuffer();
  stringBufferRequiresQuotes = false;
  stringBufferIsAllIntChars = true;
  yybegin(IN_STRING);
          }
        case 78: break;
        case 20: 
          { return lval.setError("\\b");
          }
        case 79: break;
        case 40: 
          { try {
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
        case 80: break;
        case 33: 
          { return lval.setBaseValue(new StringBaseValue());
          }
        case 81: break;
        case 32: 
          { try {
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
        case 82: break;
        case 5: 
          { return lval.setRSquare();
          }
        case 83: break;
        case 2: 
          { 
          }
        case 84: break;
        default: 
          if (zzInput == YYEOF && zzStartRead == zzCurrentPos) {
            zzAtEOF = true;
            switch (zzLexicalState) {
            case IN_BINARY_STRING:
              { return lval.setEOF(); }
            case 105: break;
            case AFTER_STRING:
              { return lval.setEOF(); }
            case 106: break;
            case IN_STRING:
              { return lval.setEOF(); }
            case 107: break;
            case YYINITIAL:
              { return lval.setEOF(); }
            case 108: break;
            default:
              { return lval.setEOF();
 }
            }
          } 
          else {
            zzScanError(ZZ_NO_MATCH);
          }
      }
    }
  }


}
