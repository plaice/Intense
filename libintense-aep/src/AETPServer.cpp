// ****************************************************************************
//
// AETPServer.cpp : AEP 2.0 AETP (text-mode) server, for the abstract/core
// AEP implementation.
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


#include <intense.hpp>
#include <queue>
#include "AEPCommon.hpp"
#include "AEPClient.hpp"
#include "AEPServer.hpp"
#include "FlexLexer.hpp"
#include "AETPLexer.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;
using namespace intense::aep;


const char* AETPServer::LexerToken::typeStrings[] = {
  "VOID", "DIMENSION", "BASEVALUE", "DASH", "DASHDASH", "DASHDASHDASH", "PLUS",
  "LANGLE", "RANGLE", "LSQUARE", "RSQUARE", "ERROR", "ENDOFFILE",
  "BREAK", "SYNCH", "CLIENT_DISCONNECT", "JOIN", "LEAVE", "ASSIGN", "APPLY",
  "CLEAR", "SELF", "CLIENT", "PREFENCE", "POSTFENCE", "ID", "SEQUENCE", "TEXT",
  "DIM_KEYWORD", "COMPOUND_DIMENSION", "CONTEXT_KEYWORD", "OP_KEYWORD",
  "NOTIFY"
};


void AEPServer::SynchToken::aetpPrint
(ostream& os) const
{
  os << "SYNCH " << clientSequence << ";\n\n";
}


void AEPServer::ClientDisconnectToken::aetpPrint
(ostream& os) const
{
  os << "DISCONNECT " << clientSequence << ";\n\n";
}


void AEPServer::JoinToken::aetpPrint
(ostream& os) const
{
  os << "JOIN " << clientSequence << ' ' << participantId;
  if (notify) {
    os << " NOTIFY";
  }
  if (dimension != NULL) {
    os << " DIM " << dimension->canonical();
  }
  os << ";\n\n";
}


void AEPServer::LeaveToken::aetpPrint
(ostream& os) const
{
  os << "LEAVE " << clientSequence << ' ' << participantId << ";\n\n";
}


void AEPServer::AssignToken::aetpPrint
(ostream& os) const
{
  os << "ASSIGN " << clientSequence << ' ' << participantId;
  if (dimension != NULL) {
    os << " DIM " << dimension->canonical();
  }
  if (preFenceBit&flags) {
    os << " PREFENCE";
  }
  if (postFenceBit&flags) {
    os << " POSTFENCE";
  }
  if (notifySelfBit&flags) {
    os << " SELF";
  }
  if (notifyClientBit&flags) {
    os << " CLIENT";
  }
  os << "\nCONTEXT\n" << *context << ";\n\n";
}


void AEPServer::ApplyToken::aetpPrint
(ostream& os) const
{
  os << "APPLY " << clientSequence << ' ' << participantId;
  if (dimension != NULL) {
    os << " DIM " << dimension->canonical();
  }
  if (preFenceBit&flags) {
    os << " PREFENCE";
  }
  if (postFenceBit&flags) {
    os << " POSTFENCE";
  }
  if (notifySelfBit&flags) {
    os << " SELF";
  }
  if (notifyClientBit&flags) {
    os << " CLIENT";
  }
  os << "\nOP\n" << *op << ";\n\n";
}


void AEPServer::ClearToken::aetpPrint
(ostream& os) const
{
  os << "CLEAR " << clientSequence << ' ' << participantId;
  if (dimension != NULL) {
    os << " DIM " << dimension->canonical();
  }
  if (preFenceBit&flags) {
    os << " PREFENCE";
  }
  if (postFenceBit&flags) {
    os << " POSTFENCE";
  }
  if (notifySelfBit&flags) {
    os << " SELF";
  }
  if (notifyClientBit&flags) {
    os << " CLIENT";
  }
  os << ";\n\n";
}


AETPServer::AETPServer
(AEther& aether, istream& in, ostream& out, Log* log,
 Log::Priority logLevel, bool errorTolerant)
  : AEPStreamServer(aether, in, out, log, logLevel, errorTolerant),
    lexer(new AETPLexer(in, AETPLexer::AETP_SERVER_LEXER))
{}


AETPServer::~AETPServer
()
{
  delete lexer;
}


void AETPServer::send
(AEPCommon::Token& token)
{
  token.aetpPrint(*out);
}


AETPServer::Token* AETPServer::receive
()
{
  Token* token;
  LexerToken lexerToken;
  LexerToken::Type type;

  type = (LexerToken::Type)(lexer->getToken(lexerToken));
  switch (type) {
  case LexerToken::VOID:
    throw AEPException("Received VOID AETPServer::LexerToken");
  case LexerToken::SYNCH:
    {
      long long clientSequence;

      // Client sequence:
      readSequence(lexerToken, clientSequence, "client sequence", type);
      if (lexer->getToken(lexerToken) != LexerToken::BREAK) {
        throwBadLexerToken(lexerToken, type);
      }
      token = new SynchToken(clientSequence);
    }
    break;
  case LexerToken::CLIENT_DISCONNECT:
    {
      long long clientSequence;

      // Client sequence:
      readSequence(lexerToken, clientSequence, "client sequence", type);
      if (lexer->getToken(lexerToken) != LexerToken::BREAK) {
        throwBadLexerToken(lexerToken, type);
      }
      token = new ClientDisconnectToken(clientSequence);
    }
    break;
  case LexerToken::JOIN:
    {
      long long clientSequence;
      long long participantId;
      bool notify = false;
      CompoundDimension* dimension = NULL;
      int newTokenType;

      // Client sequence:
      readSequence(lexerToken, clientSequence, "client sequence", type);
      // Participant id:
      readParticipantId(lexerToken, participantId, type);
      while ((newTokenType = lexer->getToken(lexerToken))
             != LexerToken::BREAK) {
        switch (newTokenType) {
        case LexerToken::NOTIFY:
          if (notify) {
            throw AEPException(
              "More than one occurrence of NOTIFY keyword in JOIN"
            );
          }
          notify = true;
          break;
        case LexerToken::DIM_KEYWORD:
           switch (lexer->getToken(lexerToken)) {
          case LexerToken::COMPOUND_DIMENSION:
            if (dimension != NULL) {
              throw AEPException(
                "More than one occurrence of DIM keyword in JOIN"
              );
            }
            dimension = lexerToken.aetpValue.compoundDimension;
            break;
           default:
             throwBadLexerToken(lexerToken, type);
           }
           break;
        default:
          throwBadLexerToken(lexerToken, type);
        }
      }
      token = new JoinToken(clientSequence, participantId, notify, dimension);
      // JoinToken copies non-NULL dimensions:
      delete dimension;
    }
    break;
  case LexerToken::LEAVE:
    {
      long long clientSequence;
      long long participantId;

      // Client sequence:
      readSequence(lexerToken, clientSequence, "client sequence", type);
      // Participant id:
      readParticipantId(lexerToken, participantId, type);
      if (lexer->getToken(lexerToken) != LexerToken::BREAK) {
        throwBadLexerToken(lexerToken, type);
      }
      token = new LeaveToken(clientSequence, participantId);
    }
    break;
  case LexerToken::ASSIGN:
    {
      long long clientSequence;
      long long participantId;
      CompoundDimension* dimension = NULL;
      Context* context = NULL;
      int newTokenType;
      bool haveSeenDimension = false;
      int flags = 0;

      // Client sequence:
      readSequence(lexerToken, clientSequence, "client sequence", type);
      // Participant id:
      readParticipantId(lexerToken, participantId, type);
      while ((newTokenType = lexer->getToken(lexerToken))
             != LexerToken::CONTEXT_KEYWORD) {
        switch (newTokenType) {
        case LexerToken::DIM_KEYWORD:
          if (haveSeenDimension) {
            throw AEPException(
              "More than one occurrence of DIM keyword in ASSIGN"
            );
          }
          if (lexer->getToken(lexerToken) == LexerToken::COMPOUND_DIMENSION) {
            dimension = lexerToken.aetpValue.compoundDimension;
            haveSeenDimension = true;
          } else {
            throwBadLexerToken(lexerToken, type);
          }
          break;
        case LexerToken::PREFENCE:
          if (flags&AEPServer::preFenceBit) {
            throw AEPException(
              "More than one occurrence of PREFENCE keyword in ASSIGN"
            );
          }
          flags |= AEPServer::preFenceBit;
          break;
        case LexerToken::POSTFENCE:
          if (flags&AEPServer::postFenceBit) {
            throw AEPException(
              "More than one occurrence of POSTFENCE keyword in ASSIGN"
            );
          }
          flags |= AEPServer::postFenceBit;
          break;
        case LexerToken::SELF:
          if (flags&AEPServer::notifySelfBit) {
            throw AEPException(
              "More than one occurrence of SELF keyword in ASSIGN"
            );
          }
          flags |= AEPServer::notifySelfBit;
          break;
        case LexerToken::CLIENT:
          if (flags&AEPServer::notifyClientBit) {
            throw AEPException(
              "More than one occurrence of CLIENT keyword in ASSIGN"
            );
          }
          flags |= AEPServer::notifyClientBit;
          break;
        default:
          throwBadLexerToken(lexerToken, type);
        }
      }
      context = new Context;
      lexer->beginInContext();
      context->recogniseNode(*lexer);
      lexer->beginInToken();
      if (lexer->getToken(lexerToken) != LexerToken::BREAK) {
        throwBadLexerToken(lexerToken, type);
      }
      token = new AssignToken(
        clientSequence, participantId, context, dimension, flags
      );
      // AsynchronousToken copies non-NULL dimensions:
      delete dimension;
    }
    break;
  case LexerToken::APPLY:
    {
      long long clientSequence;
      long long participantId;
      CompoundDimension* dimension = NULL;
      ContextOp* op = NULL;
      int newTokenType;
      bool haveSeenDimension = false;
      int flags = 0;

      // Client sequence:
      readSequence(lexerToken, clientSequence, "client sequence", type);
      // Participant id:
      readParticipantId(lexerToken, participantId, type);
      while ((newTokenType = lexer->getToken(lexerToken))
             != LexerToken::OP_KEYWORD) {
        switch (newTokenType) {
        case LexerToken::DIM_KEYWORD:
          if (haveSeenDimension) {
            throw AEPException(
              "Multiple occurrences of DIM keyword in APPLY"
            );
          }
          if (lexer->getToken(lexerToken) == LexerToken::COMPOUND_DIMENSION) {
            dimension = lexerToken.aetpValue.compoundDimension;
            haveSeenDimension = true;
          } else {
            throwBadLexerToken(lexerToken, type);
          }
          break;
        case LexerToken::PREFENCE:
          if (flags&AEPServer::preFenceBit) {
            throw AEPException(
              "Multiple occurrences of PREFENCE keyword in APPLY"
            );
          }
          flags |= AEPServer::preFenceBit;
          break;
        case LexerToken::POSTFENCE:
          if (flags&AEPServer::postFenceBit) {
            throw AEPException(
              "Multiple occurrences of POSTFENCE keyword in APPLY"
            );
          }
          flags |= AEPServer::postFenceBit;
          break;
        case LexerToken::SELF:
          if (flags&AEPServer::notifySelfBit) {
            throw AEPException(
              "Multiple occurrences of SELF keyword in APPLY"
            );
          }
          flags |= AEPServer::notifySelfBit;
          break;
        case LexerToken::CLIENT:
          if (flags&AEPServer::notifyClientBit) {
            throw AEPException(
              "Multiple occurrences of CLIENT keyword in APPLY"
            );
          }
          flags |= AEPServer::notifyClientBit;
          break;
        default:
          throwBadLexerToken(lexerToken, type);
        }
      }
      op = new ContextOp;
      lexer->beginInContext();
      op->recogniseNode(*lexer);
      lexer->beginInToken();
      if (lexer->getToken(lexerToken) != LexerToken::BREAK) {
        throwBadLexerToken(lexerToken, type);
      }
      token = new ApplyToken(
        clientSequence, participantId, op, dimension, flags
      );
      // AsynchronousToken copies non-NULL dimensions:
      delete dimension;
    }
    break;
  case LexerToken::CLEAR:
    {
      long long clientSequence;
      long long participantId;
      CompoundDimension* dimension = NULL;
      int newTokenType;
      bool haveSeenDimension = false;
      int flags = 0;

      // Client sequence:
      readSequence(lexerToken, clientSequence, "client sequence", type);
      // Participant id:
      readParticipantId(lexerToken, participantId, type);
      while ((newTokenType = lexer->getToken(lexerToken))
             != LexerToken::BREAK) {
        switch (newTokenType) {
        case LexerToken::DIM_KEYWORD:
          if (haveSeenDimension) {
            throw AEPException(
              "More than one occurrence of DIM keyword in CLEAR"
            );
          }
          if (lexer->getToken(lexerToken) == LexerToken::COMPOUND_DIMENSION) {
            dimension = lexerToken.aetpValue.compoundDimension;
            haveSeenDimension = true;
          } else {
            throwBadLexerToken(lexerToken, type);
          }
          break;
        case LexerToken::PREFENCE:
          if (flags&AEPServer::preFenceBit) {
            throw AEPException(
              "More than one occurrence of PREFENCE keyword in CLEAR"
            );
          }
          flags |= AEPServer::preFenceBit;
          break;
        case LexerToken::POSTFENCE:
          if (flags&AEPServer::postFenceBit) {
            throw AEPException(
              "More than one occurrence of POSTFENCE keyword in CLEAR"
            );
          }
          flags |= AEPServer::postFenceBit;
          break;
        case LexerToken::SELF:
          if (flags&AEPServer::notifySelfBit) {
            throw AEPException(
              "More than one occurrence of SELF keyword in CLEAR"
            );
          }
          flags |= AEPServer::notifySelfBit;
          break;
        case LexerToken::CLIENT:
          if (flags&AEPServer::notifyClientBit) {
            throw AEPException(
              "More than one occurrence of CLIENT keyword in CLEAR"
            );
          }
          flags |= AEPServer::notifyClientBit;
          break;
        default:
          throwBadLexerToken(lexerToken, type);
        }
      }
      token = new ClearToken(
        clientSequence, participantId, dimension, flags
      );
      // AsynchronousToken copies non-NULL dimensions:
      delete dimension;
    }
    break;
  case LexerToken::ERROR:
    {
      ostringstream oss;

      oss << "Parsed error token in AETPServer::receive: \""
          << *(lexerToken.value.errorText) << "\"";
      throw AEPException(oss.str());
    }
  default:
    {
      ostringstream oss;

      oss << "Parsed bad token type " << lexerToken.getType()
          << " in AETPServer::receive";
      throw AEPException(oss.str());
    }
  }
  return token;
}


void AETPServer::readSequence
(LexerToken& lexerToken, long long& dest, const char* sequenceName,
 LexerToken::Type inTokenType)
{
  ((AETPLexer*)lexer)->integersType = AETPLexer::SEQUENCE;
  switch (lexer->getToken(lexerToken)) {
  case LexerToken::SEQUENCE:
    dest = lexerToken.aetpValue.sequence;
    break;
  case LexerToken::BREAK:
    {
      const char* inTokenName = LexerToken::getTypeString(inTokenType);
      ostringstream oss;

      oss << "Missing " << sequenceName << " in " << inTokenName;
      throw AEPException(oss.str());
    }
  default:
    {
      const char* inTokenName = LexerToken::getTypeString(inTokenType);

      throwBadLexerToken(lexerToken, inTokenType);
    }
  }
}


void AETPServer::readParticipantId
(LexerToken& lexerToken, long long& dest, LexerToken::Type inTokenType)
{
  ((AETPLexer*)lexer)->integersType = AETPLexer::ID;
  switch (lexer->getToken(lexerToken)) {
  case LexerToken::ID:
    dest = lexerToken.aetpValue.id;
    break;
  case LexerToken::BREAK:
    {
      const char* inTokenName = LexerToken::getTypeString(inTokenType);
      ostringstream oss;

      oss << "Missing participantId in " << inTokenName;
      throw AEPException(oss.str());
    }
  default:
    {
      const char* inTokenName = LexerToken::getTypeString(inTokenType);

      throwBadLexerToken(lexerToken, inTokenType);
    }
  }
}


void AETPServer::throwBadLexerToken
(LexerToken& token, LexerToken::Type inTokenType)
{
  ostringstream oss;

  oss << "Unexpected token of type " << token.getTypeString()
      << " in " << LexerToken::getTypeString(inTokenType);
  throw AEPException(oss.str());
}


const char* AETPServer::getName
()
{
  ostringstream oss;

  oss << "AETPServer:" << this;
  return oss.str().data();
}
