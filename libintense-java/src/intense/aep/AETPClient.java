// ****************************************************************************
//
// AETPClient.java : AEP 2.0 textual (AETP) streamed client, compatible with
// the both the C++ and Java AETPServer.
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


public class AETPClient
  extends AEPStreamClient {


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

    // Extra AETPClient.LexerToken types:
    public static final int BREAK = 13;
    public static final int SERVER_DISCONNECT = 14;
    public static final int NOTIFY = 15;
    public static final int ASSIGN = 16;
    public static final int APPLY = 17;
    public static final int CLEAR = 18;
    public static final int KICK = 19;
    public static final int ACK = 20;
    public static final int DENY = 21;
    public static final int COUNT = 22;
    public static final int ID_KEYWORD = 23;
    public static final int ID = 24;
    public static final int SEQUENCE = 25;
    public static final int TEXT = 26;
    public static final int DIM_KEYWORD = 27;
    public static final int ROOT_KEYWORD = 28;
    public static final int COMPOUND_DIMENSION = 29;
    public static final int EXT_KEYWORD = 30;
    public static final int INT_KEYWORD = 31;

    public static final String[] typeStrings = {
      "VOID", "DIMENSION", "BASEVALUE", "DASH", "DASHDASH", "DASHDASHDASH",
      "PLUS", "LANGLE", "RANGLE", "LSQUARE", "RSQUARE", "ERROR", "ENDOFFILE",
      "BREAK", "SERVER_DISCONNECT", "NOTIFY", "ASSIGN", "APPLY", "CLEAR",
      "KICK", "ACK", "DENY", "COUNT", "ID_KEYWORD", "ID", "SEQUENCE", "TEXT",
      "DIM_KEYWORD", "ROOT_KEYWORD", "COMPOUND_DIMENSION", "EXT_KEYWORD",
      "INT_KEYWORD"
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

    int setServerDisconnect
    ()
    {
      return type = SERVER_DISCONNECT;
    }

    int setNotify
    ()
    {
      return type = NOTIFY;
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

    int setKick
    ()
    {
      return type = KICK;
    }

    int setAck
    ()
    {
      return type = ACK;
    }

    int setDeny
    ()
    {
      return type = DENY;
    }

    int setCount
    (int count)
    {
      value = new Integer(count);
      return type = COUNT;
    }

    int setIdKeyword
    ()
    {
      return type = ID_KEYWORD;
    }

    int setId
    (long id)
    {
      value = new Long(id);
      return type = ID;
    }

    int setSequence
    (long sequence)
    {
      value = new Long(sequence);
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

    int setRootKeyword
    ()
    {
      return type = ROOT_KEYWORD;
    }

    int setCompoundDimension
    (CompoundDimension compoundDimension)
    {
      value = compoundDimension;
      return type = COMPOUND_DIMENSION;
    }

    int setExternalKeyword
    ()
    {
      return type = EXT_KEYWORD;
    }
    
    int setInternalKeyword
    ()
    {
      return type = INT_KEYWORD;
    }

  };


  protected Reader is;

  protected PrintStream os;

  AETPLexer lexer;

  protected void construct
  ()
  {
    this.is = new InputStreamReader(inputStream);
    this.os = new PrintStream(outputStream, true);
    lexer = new AETPLexer(is);
    lexer.setClientType();
  }

  public AETPClient
  (InputStream is, OutputStream os)
    throws AEPException
  {
    super(is, os, null, Log.NOTICE, true, false, 0);
    construct();
  }

  public AETPClient
  (InputStream is, OutputStream os, Log log)
    throws AEPException
  {
    super(is, os, log, Log.NOTICE, true, false, 0);
    construct();
  }

  public AETPClient
  (InputStream is, OutputStream os, Log log, int logLevel)
    throws AEPException
  {
    super(is, os, log, logLevel, true, false, 0);
    construct();
  }

  public AETPClient
  (InputStream is, OutputStream os, Log log, int logLevel,
   boolean errorTolerant)
    throws AEPException
  {
    super(is, os, log, logLevel, errorTolerant, false, 0);
    construct();
  }

  public AETPClient
  (InputStream is, OutputStream os, Log log, int logLevel,
   boolean errorTolerant, boolean useReceiverThread)
    throws AEPException
  {
    super(is, os, log, logLevel, errorTolerant, useReceiverThread, 0);
    construct();
  }

  public AETPClient
  (InputStream is, OutputStream os, Log log, int logLevel,
   boolean errorTolerant, boolean useReceiverThread, int maxReceiveSize)
    throws AEPException
  {
    super(
      is, os, log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize
    );
    construct();
  }

  protected void send
  (AEPServer.Token token)
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
      throw new AEPException("Received VOID AETPClient.LexerToken");
    case LexerToken.SERVER_DISCONNECT:
      {
        long serverSequence;

        // Server sequence:
        serverSequence = readSequence(lexerToken, "server sequence", type);
        // Break:
        if (lexer.getToken(lexerToken) != LexerToken.BREAK) {
          throwBadLexerToken(lexerToken, type);
        }
        token = new ServerDisconnectToken(serverSequence);
      }
      break;
    case LexerToken.NOTIFY:
      {
        long serverSequence;
        int targetsSize;
        int nodesSize;
        boolean haveSeenAssign = false;
        boolean haveSeenApply = false;

        serverSequence = readSequence(lexerToken, "server sequence", type);
        token = new NotifyToken(serverSequence);
        // Number of targets:
        ((AETPLexer)lexer).setCountIntegersType();
        if (lexer.getToken(lexerToken) != LexerToken.COUNT) {
          throwBadLexerToken(lexerToken, type);
        }
        targetsSize = ((Integer)lexerToken.getValue()).intValue();
        // Number of nodes:
        ((AETPLexer)lexer).setCountIntegersType();
        if (lexer.getToken(lexerToken) != LexerToken.COUNT) {
          throwBadLexerToken(lexerToken, type);
        }
        nodesSize = ((Integer)lexerToken.getValue()).intValue();
        for (int i = 0; i < targetsSize; i++) {
          long participantId;
          int nodeIndex;
          CompoundDimension dimension = null;
          boolean dimensionIsExternal = false;
          int targetType;

          switch (targetType = lexer.getToken(lexerToken)) {
          case LexerToken.ASSIGN:
          case LexerToken.APPLY:
            ((AETPLexer)lexer).setIdIntegersType();
            if (lexer.getToken(lexerToken) != LexerToken.ID) {
              throwBadLexerToken(lexerToken, type);
            }
            participantId = ((Long)lexerToken.getValue()).longValue();
            // nodeIndex:
            ((AETPLexer)lexer).setCountIntegersType();
            if (lexer.getToken(lexerToken) != LexerToken.COUNT) {
              throwBadLexerToken(lexerToken, type);
            }
            nodeIndex = ((Integer)lexerToken.getValue()).intValue();
            if (lexer.getToken(lexerToken) == LexerToken.DIM_KEYWORD) {
              if (lexer.getToken(lexerToken) !=
                  LexerToken.COMPOUND_DIMENSION) {
                throwBadLexerToken(lexerToken, type);
              }
              dimension = (CompoundDimension)lexerToken.getValue();
              lexer.getToken(lexerToken);
              if (lexerToken.getType() == LexerToken.EXT_KEYWORD) {
                dimensionIsExternal = true;
              } else if (lexerToken.getType() == LexerToken.INT_KEYWORD) {
                dimensionIsExternal = false;
              } else {
                throwBadLexerToken(lexerToken, type);
              }
            } else if (lexerToken.getType() != LexerToken.ROOT_KEYWORD) {
              throwBadLexerToken(lexerToken, type);
            }
            if (targetType == LexerToken.ASSIGN) {
              haveSeenAssign = true;
              if (dimension != null) {
                ((NotifyToken)token).addTarget(new NotifyToken.AssignTarget(
                  participantId, nodeIndex, dimension, dimensionIsExternal
                ));
              } else {
                ((NotifyToken)token).addTarget(
                  new NotifyToken.AssignTarget(participantId, nodeIndex)
                );
              }
            } else { // targetType == LexerToken.APPLY
              haveSeenApply = true;
              if (dimension != null) {
                ((NotifyToken)token).addTarget(
                  new NotifyToken.ApplyTarget(
                    participantId, nodeIndex, dimension, dimensionIsExternal
                  ));
              } else {
                ((NotifyToken)token).addTarget(
                  new NotifyToken.ApplyTarget(participantId, nodeIndex)
                );
              }
            }
            // Assign/ApplyTarget constructor does NOT copy dimensions.
            break;
          case LexerToken.CLEAR:
            ((AETPLexer)lexer).setIdIntegersType();
            if (lexer.getToken(lexerToken) != LexerToken.ID) {
              throwBadLexerToken(lexerToken, type);
            }
            participantId = ((Long)lexerToken.getValue()).longValue();
            if (lexer.getToken(lexerToken) == LexerToken.DIM_KEYWORD) {
              if (lexer.getToken(lexerToken) !=
                  LexerToken.COMPOUND_DIMENSION) {
                throwBadLexerToken(lexerToken, type);
              }
              dimension = (CompoundDimension)lexerToken.getValue();
            } else if (lexerToken.getType() != LexerToken.ROOT_KEYWORD) {
              throwBadLexerToken(lexerToken, type);
            }
            if (dimension != null) {
              ((NotifyToken)token).addTarget(
                new NotifyToken.ClearTarget(participantId, dimension)
              );
            } else {
              ((NotifyToken)token).addTarget(
                new NotifyToken.ClearTarget(participantId)
              );
            }
            break;
          case LexerToken.KICK:
            ((AETPLexer)lexer).setIdIntegersType();
            if (lexer.getToken(lexerToken) != LexerToken.ID) {
              throwBadLexerToken(lexerToken, type);
            }
            participantId = ((Long)lexerToken.getValue()).longValue();
            ((NotifyToken)token).addTarget(
              new NotifyToken.KickTarget(participantId)
            );
            break;
          default:
            throwBadLexerToken(lexerToken, type);
          }
        }
        for (int i = 0; i < nodesSize; i++) {
          Context node;

          if (haveSeenAssign) {
            node = new Context();
          } else if (haveSeenApply) {
            node = new ContextOp();
          } else {
            throw new AEPException(
              "Received NOTIFY with non-zero node count but no assign or " +
              "apply targets"
            );
          }
          lexer.beginInContext();
          node.recogniseNode(lexer);
          lexer.beginInToken();
          ((NotifyToken)token).addNode(node);
        }
      }
      if (lexer.getToken(lexerToken) != LexerToken.BREAK) {
        throwBadLexerToken(lexerToken, type);
      }
      break;
    case LexerToken.ACK:
    case LexerToken.DENY:
      {
        long clientSequence;
        long serverSequence;
        String message = null;

        serverSequence = readSequence(lexerToken, "server sequence", type);
        clientSequence = readSequence(lexerToken, "client sequence", type);
        // Message or break:
        if (lexer.getToken(lexerToken) == LexerToken.TEXT) {
          message = lexerToken.getValue().toString();
          lexer.getToken(lexerToken);
          if (lexer.getToken(lexerToken) != LexerToken.BREAK) {
            throwBadLexerToken(lexerToken, type);
          }
        } else if (lexerToken.getType() != LexerToken.BREAK) {
          throwBadLexerToken(lexerToken, type);
        }
        switch (type) {
        case LexerToken.ACK:
          token = new AckToken(serverSequence, clientSequence, message);
          break;
        case LexerToken.DENY:
          token = new DenyToken(serverSequence, clientSequence, message);
          break;
        case LexerToken.ERROR:
          token = new ErrorToken(serverSequence, clientSequence, message);
          break;
        }
      }
    break;
    case LexerToken.ERROR:
      {
        throw new AEPException(
          "Parsed error token in AETPClient.receive: \"" +
          lexerToken.getValue() + "\""
        );
      }
    default:
      {
        throw new AEPException(
          "INTERNAL ERROR: Parsed bad token type " +
          lexerToken.getTypeString() + " in AETPClient.receive",
          Log.FATAL
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

  void throwBadLexerToken
  (LexerToken token, int inTokenType)
    throws AEPException
  {
    throw new AEPException(
      "Unexpected token of type " + token.getTypeString() +
      " in " + LexerToken.getTypeString(inTokenType)
    );
  }

  protected String getName
  ()
  {
    return "AETPClient";
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
