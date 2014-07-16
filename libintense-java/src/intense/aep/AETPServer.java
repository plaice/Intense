// ****************************************************************************
//
// AETPServer.java : AEP 2.0 textual (AETP) streamed server, compatible with
// the both the C++ and Java AETPClient.
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


package intense.aep;


import java.lang.*;
import java.util.*;
import java.util.concurrent.locks.*;
import java.io.*;
import intense.*;
import intense.log.*;


public class AETPServer
  extends AEPStreamServer {


  protected static class LexerToken
    extends Context.Token {

    // Context.Token types (preserving enum sequence):
    public static final int VOID = 0;
    public static final int DIMENSION = 1;
    public static final int BASEVALUE = 2;
    public static final int DASH = 3;
    public static final int DASHDASH = 4;
    public static final int DASHDASHDASH = 5;
    public static final int PLUS = 6;
    public static final int LANGLE = 7;
    public static final int RANGLE = 8;
    public static final int LSQUARE = 9;
    public static final int RSQUARE = 10;
    public static final int ERROR = 11;
    public static final int ENDOFFILE = 12;

    // Extra AETPServer.LexerToken types:
    public static final int BREAK = 13;
    public static final int SYNCH = 14;
    public static final int CLIENT_DISCONNECT = 15;
    public static final int JOIN = 16;
    public static final int LEAVE = 17;
    public static final int ASSIGN = 18;
    public static final int APPLY = 19;
    public static final int CLEAR = 20;
    public static final int SELF = 21;
    public static final int CLIENT = 22;
    public static final int PREFENCE = 23;
    public static final int POSTFENCE = 24;
    public static final int ID = 25;
    public static final int SEQUENCE = 26;
    public static final int TEXT = 27;
    public static final int DIM_KEYWORD = 28;
    public static final int COMPOUND_DIMENSION = 29;
    public static final int CONTEXT_KEYWORD = 30;
    public static final int OP_KEYWORD = 31;
    public static final int NOTIFY = 32;

    public static final String[] typeStrings = {
      "VOID", "DIMENSION", "BASEVALUE", "DASH", "DASHDASH", "DASHDASHDASH",
      "PLUS", "LANGLE", "RANGLE", "LSQUARE", "RSQUARE", "ERROR", "ENDOFFILE",
      "BREAK", "SYNCH", "CLIENT_DISCONNECT", "JOIN", "LEAVE", "ASSIGN",
      "APPLY", "CLEAR", "SELF", "CLIENT", "PREFENCE", "POSTFENCE", "ID",
      "SEQUENCE", "TEXT", "DIM_KEYWORD", "COMPOUND_DIMENSION",
      "CONTEXT_KEYWORD", "OP_KEYWORD", "NOTIFY"
    };

    private int type;

    public int getType
    ()
    {
      return type;
    }

    public String getTypeString
    ()
    {
      return typeStrings[type];
    }

    public static String getTypeString
    (int type)
    {
      return typeStrings[type];
    }

    public LexerToken
    ()
    {
      super();
    }

    int setBreak
    ()
    {
      return type = BREAK;
    }

    int setSynch
    ()
    {
      return type = SYNCH;
    }

    int setClientDisconnect
    ()
    {
      return type = CLIENT_DISCONNECT;
    }

    int setJoin
    ()
    {
      return type = JOIN;
    }

    int setLeave
    ()
    {
      return type = LEAVE;
    }

    int setAssign
    ()
    {
      return type = ASSIGN;
    }

    int setApply
    ()
    {
      return type = APPLY;
    }

    int setClear
    ()
    {
      return type = CLEAR;
    }

    int setPreFence
    ()
    {
      return type = PREFENCE;
    }

    int setPostFence
    ()
    {
      return type = POSTFENCE;
    }

    int setSelf
    ()
    {
      return type = SELF;
    }

    int setClient
    ()
    {
      return type = CLIENT;
    }

    int setId
    (Long id)
    {
      value = id;
      return type = ID;
    }

    int setSequence
    (Long sequence)
    {
      value = sequence;
      return type = SEQUENCE;
    }

    int setText
    (String text)
    {
      value = text;
      return type = TEXT;
    }

    int setDimKeyword
    ()
    {
      return type = DIM_KEYWORD;
    }

    int setCompoundDimension
    (CompoundDimension compoundDimension)
    {
      value = compoundDimension;
      return type = COMPOUND_DIMENSION;
    }

    int setContextKeyword
    ()
    {
      return type = CONTEXT_KEYWORD;
    }

    int setOpKeyword
    ()
    {
      return type = OP_KEYWORD;
    }

    int setNotify
    ()
    {
      return type = NOTIFY;
    }

  }


  protected Reader is;

  protected PrintStream os;

  AETPLexer lexer;

  protected void construct
  ()
  {
    this.is = new InputStreamReader(inputStream);
    this.os = new PrintStream(outputStream, true);
    lexer = new AETPLexer(is);
    lexer.setServerType();
  }

  public AETPServer
  (AEther aether, InputStream is, OutputStream os)
  {
    super(aether, is, os, null, Log.NOTICE, true);
    construct();
  }

  public AETPServer
  (AEther aether, InputStream is, OutputStream os, Log log)
  {
    super(aether, is, os, log, Log.NOTICE, true);
    construct();
  }

  public AETPServer
  (AEther aether, InputStream is, OutputStream os, Log log,
   int logLevel)
  {
    super(aether, is, os, log, logLevel, true);
    construct();
  }

  public AETPServer
  (AEther aether, InputStream is, OutputStream os, Log log,
   int logLevel, boolean errorTolerant)
  {
    super(aether, is, os, log, logLevel, errorTolerant);
    construct();
  }

  protected void send
  (AEPClient.Token token)
    throws IOException, AEPException
  {
    token.aetpPrint(os);
    os.flush();
    outputStream.flush();
  }

  protected Token receive
  ()
    throws IOException, IntenseException, AEPException
  {
    Token token = null;
    LexerToken lexerToken = new LexerToken();
    int type;

    type = lexer.getToken(lexerToken);
    switch (type) {
    case LexerToken.VOID:
      throw new AEPException("Received VOID AETPServer.LexerToken");
    case LexerToken.SYNCH:
      {
        long clientSequence;

        clientSequence = readSequence(lexerToken, "client sequence", type);
        if (lexer.getToken(lexerToken) != LexerToken.BREAK) {
          throwBadLexerToken(lexerToken, type);
        }
        token = new SynchToken(clientSequence);
      }
      break;
    case LexerToken.CLIENT_DISCONNECT:
      {
        long clientSequence;

        clientSequence = readSequence(lexerToken, "client sequence", type);
        if (lexer.getToken(lexerToken) != LexerToken.BREAK) {
          throwBadLexerToken(lexerToken, type);
        }
        token = new ClientDisconnectToken(clientSequence);
      }
      break;
    case LexerToken.JOIN:
      {
        long clientSequence;
        long participantId;
        boolean notify = false;
        CompoundDimension dimension = null;
        int newTokenType;

        clientSequence = readSequence(lexerToken, "client sequence", type);
        participantId = readParticipantId(lexerToken, type);
        while ((newTokenType = lexer.getToken(lexerToken))
               != LexerToken.BREAK) {
          switch (newTokenType) {
          case LexerToken.NOTIFY:
            if (notify) {
              throw new AEPException(
                "More than one occurrence of NOTIFY keyword in JOIN"
              );
            }
            notify = true;
            break;
          case LexerToken.DIM_KEYWORD:
            switch (lexer.getToken(lexerToken)) {
            case LexerToken.COMPOUND_DIMENSION:
              if (dimension != null) {
                throw new AEPException(
                  "More than one occurrence of DIM keyword in JOIN"
                );
              }
              dimension = (CompoundDimension)lexerToken.getValue();
              break;
            default:
              throwBadLexerToken(lexerToken, type);
            }
            break;
          default:
            throwBadLexerToken(lexerToken, type);
          }
        }
        token = new JoinToken(
          clientSequence, participantId, notify, dimension
        );
      }
      break;
    case LexerToken.LEAVE:
      {
        long clientSequence;
        long participantId;

        // Client sequence:
        clientSequence = readSequence(lexerToken, "client sequence", type);
        // Participant id:
        participantId = readParticipantId(lexerToken, type);
        if (lexer.getToken(lexerToken) != LexerToken.BREAK) {
          throwBadLexerToken(lexerToken, type);
        }
        token = new LeaveToken(clientSequence, participantId);
      }
      break;
    case LexerToken.ASSIGN:
      {
        long clientSequence;
        long participantId;
        CompoundDimension dimension = null;
        Context context = null;
        int newTokenType;
        boolean haveSeenDimension = false;
        int flags = 0;

        clientSequence = readSequence(lexerToken, "client sequence", type);
        participantId = readParticipantId(lexerToken, type);
        while ((newTokenType = lexer.getToken(lexerToken))
               != LexerToken.CONTEXT_KEYWORD) {
          switch (newTokenType) {
          case LexerToken.DIM_KEYWORD:
            if (haveSeenDimension) {
              throw new AEPException(
                "More than one occurrence of DIM keyword in ASSIGN"
              );
            }
            if (lexer.getToken(lexerToken) == LexerToken.COMPOUND_DIMENSION) {
              dimension = (CompoundDimension)lexerToken.getValue();
              haveSeenDimension = true;
            } else {
              throwBadLexerToken(lexerToken, type);
            }
            break;
          case LexerToken.PREFENCE:
            if ((flags&AEPServer.preFenceBit) != 0) {
              throw new AEPException(
                "More than one occurrence of PREFENCE keyword in ASSIGN"
              );
            }
            flags |= AEPServer.preFenceBit;
            break;
          case LexerToken.POSTFENCE:
            if ((flags&AEPServer.postFenceBit) != 0) {
              throw new AEPException(
                "More than one occurrence of POSTFENCE keyword in ASSIGN"
              );
            }
            flags |= AEPServer.postFenceBit;
            break;
          case LexerToken.SELF:
            if ((flags&AEPServer.notifySelfBit) != 0) {
              throw new AEPException(
                "More than one occurrence of SELF keyword in ASSIGN"
              );
            }
            flags |= AEPServer.notifySelfBit;
            break;
          case LexerToken.CLIENT:
            if ((flags&AEPServer.notifyClientBit) != 0) {
              throw new AEPException(
                "More than one occurrence of CLIENT keyword in ASSIGN"
              );
            }
            flags |= AEPServer.notifyClientBit;
            break;
          default:
            throwBadLexerToken(lexerToken, type);
          }
        }
        context = new Context();
        lexer.beginInContext();
        context.recogniseNode(lexer, null);
        lexer.beginInToken();
        if (lexer.getToken(lexerToken) != LexerToken.BREAK) {
          throwBadLexerToken(lexerToken, type);
        }
        token = new AssignToken(
          clientSequence, participantId, context, dimension, flags
        );
      }
      break;
    case LexerToken.APPLY:
      {
        long clientSequence;
        long participantId;
        CompoundDimension dimension = null;
        ContextOp op = null;
        int newTokenType;
        boolean haveSeenDimension = false;
        int flags = 0;

        clientSequence = readSequence(lexerToken, "client sequence", type);
        participantId = readParticipantId(lexerToken, type);
        while ((newTokenType = lexer.getToken(lexerToken))
               != LexerToken.OP_KEYWORD) {
          switch (newTokenType) {
          case LexerToken.DIM_KEYWORD:
            if (haveSeenDimension) {
              throw new AEPException(
                "Multiple occurrences of DIM keyword in APPLY"
              );
            }
            if (lexer.getToken(lexerToken) == LexerToken.COMPOUND_DIMENSION) {
              dimension = (CompoundDimension)lexerToken.getValue();
              haveSeenDimension = true;
            } else {
              throwBadLexerToken(lexerToken, type);
            }
            break;
          case LexerToken.PREFENCE:
            if ((flags&AEPServer.preFenceBit) != 0) {
              throw new AEPException(
                "Multiple occurrences of PREFENCE keyword in APPLY"
              );
            }
            flags |= AEPServer.preFenceBit;
            break;
          case LexerToken.POSTFENCE:
            if ((flags&AEPServer.postFenceBit) != 0) {
              throw new AEPException(
                "Multiple occurrences of POSTFENCE keyword in APPLY"
              );
            }
            flags |= AEPServer.postFenceBit;
            break;
          case LexerToken.SELF:
            if ((flags&AEPServer.notifySelfBit) != 0) {
              throw new AEPException(
                "Multiple occurrences of SELF keyword in APPLY"
              );
            }
            flags |= AEPServer.notifySelfBit;
            break;
          case LexerToken.CLIENT:
            if ((flags&AEPServer.notifyClientBit) != 0) {
              throw new AEPException(
                "Multiple occurrences of CLIENT keyword in APPLY"
              );
            }
            flags |= AEPServer.notifyClientBit;
            break;
          default:
            throwBadLexerToken(lexerToken, type);
          }
        }
        op = new ContextOp();
        lexer.beginInContext();
        ((ContextOp)op).recogniseNode(lexer);
        lexer.beginInToken();
        if (lexer.getToken(lexerToken) != LexerToken.BREAK) {
          throwBadLexerToken(lexerToken, type);
        }
        token = new ApplyToken(
          clientSequence, participantId, op, dimension, flags
        );
      }
      break;
    case LexerToken.CLEAR:
      {
        long clientSequence;
        long participantId;
        CompoundDimension dimension = null;
        int newTokenType;
        boolean haveSeenDimension = false;
        int flags = 0;

        clientSequence = readSequence(lexerToken, "client sequence", type);
        participantId = readParticipantId(lexerToken, type);
        while ((newTokenType = lexer.getToken(lexerToken))
               != LexerToken.BREAK) {
          switch (newTokenType) {
          case LexerToken.DIM_KEYWORD:
            if (haveSeenDimension) {
              throw new AEPException(
                "More than one occurrence of DIM keyword in CLEAR"
              );
            }
            if (lexer.getToken(lexerToken) == LexerToken.COMPOUND_DIMENSION) {
              dimension = (CompoundDimension)lexerToken.getValue();
              haveSeenDimension = true;
            } else {
              throwBadLexerToken(lexerToken, type);
            }
            break;
          case LexerToken.PREFENCE:
            if ((flags&AEPServer.preFenceBit) != 0) {
              throw new AEPException(
                "More than one occurrence of PREFENCE keyword in CLEAR"
              );
            }
            flags |= AEPServer.preFenceBit;
            break;
          case LexerToken.POSTFENCE:
            if ((flags&AEPServer.postFenceBit) != 0) {
              throw new AEPException(
                "More than one occurrence of POSTFENCE keyword in CLEAR"
              );
            }
            flags |= AEPServer.postFenceBit;
            break;
          case LexerToken.SELF:
            if ((flags&AEPServer.notifySelfBit) != 0) {
              throw new AEPException(
                "More than one occurrence of SELF keyword in CLEAR"
              );
            }
            flags |= AEPServer.notifySelfBit;
            break;
          case LexerToken.CLIENT:
            if ((flags&AEPServer.notifyClientBit) != 0) {
              throw new AEPException(
                "More than one occurrence of CLIENT keyword in CLEAR"
              );
            }
            flags |= AEPServer.notifyClientBit;
            break;
          default:
            throwBadLexerToken(lexerToken, type);
          }
        }
        token = new ClearToken(
          clientSequence, participantId, dimension, flags
        );
      }
      break;
    case LexerToken.ERROR:
      {
        throw new AEPException(
          "Parsed error token in AETPServer.receive: \"" +
          lexerToken.getValue() + "\""
        );
      }
    default:
      {
        throw new AEPException(
          "Parsed bad token type " + lexerToken.getType() +
          " in AETPServer.receive"
        );
      }
    }
    return token;
  }

  long readSequence
  (LexerToken lexerToken, String sequenceName, int inTokenType)
    throws IOException, AEPException
  {
    long returnValue = -1;

    ((AETPLexer)lexer).setSequenceIntegersType();
    switch (lexer.getToken(lexerToken)) {
    case LexerToken.SEQUENCE:
      returnValue = ((Long)lexerToken.getValue()).longValue();
      break;
    case LexerToken.BREAK:
      {
        String inTokenName = LexerToken.getTypeString(inTokenType);
        throw new AEPException(
          "Missing " + sequenceName + " in " + inTokenName
        );
      }
    default:
      {
        String inTokenName = LexerToken.getTypeString(inTokenType);

        throwBadLexerToken(lexerToken, inTokenType);
      }
    }
    return returnValue;
  }

  long readParticipantId
  (LexerToken lexerToken, int inTokenType)
    throws IOException, AEPException
  {
    long returnValue = -1;

    ((AETPLexer)lexer).setIdIntegersType();
    switch (lexer.getToken(lexerToken)) {
    case LexerToken.ID:
      returnValue = ((Long)lexerToken.getValue()).longValue();
      break;
    case LexerToken.BREAK:
      {
        String inTokenName = LexerToken.getTypeString(inTokenType);
        throw new AEPException(
          "Missing participantId in " + inTokenName
        );
      }
    default:
      {
        String inTokenName = LexerToken.getTypeString(inTokenType);

        throwBadLexerToken(lexerToken, inTokenType);
      }
    }
    return returnValue;
  }

  void throwBadLexerToken
  (LexerToken token, int inTokenType)
    throws AEPException
  {
    throw new AEPException(
      "Unexpected token of type " + token.getTypeString() +
      " in " + LexerToken.getTypeString(inTokenType)
    );
  }

  public String getName
  ()
  {
    return "AETPServer";
  }

  protected boolean locksReception
  ()
  {
    return false;
  }

  /**
   * The AETP input stream is obtained from (and buffered by!) the
   * JFlex-generated AETPLexer.
   */
  protected boolean blockForAvailableData
  ()
    throws IOException
  {
    return false;
  }

}
