// ****************************************************************************
//
// AETPClient.cpp : AEP 2.0 AETP (text-mode) client, for the abstract/core
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
#include <FlexLexer.h>
#include "AETPLexer.hpp"
#include "AEPCommon.hpp"
#include "AEPClient.hpp"


using namespace std;
using namespace intense;
using namespace intense::io;
using namespace intense::aep;


const char* AETPClient::LexerToken::typeStrings[] = {
  "VOID", "DIMENSION", "BASEVALUE", "DASH", "DASHDASH", "DASHDASHDASH", "PLUS",
  "LANGLE", "RANGLE", "LSQUARE", "RSQUARE", "ERROR", "ENDOFFILE", "BREAK",
  "SERVER_DISCONNECT", "NOTIFY", "ASSIGN", "APPLY", "CLEAR", "KICK", "ACK",
  "DENY", "COUNT", "ID_KEYWORD", "ID", "SEQUENCE", "TEXT", "DIM_KEYWORD",
  "ROOT_KEYWORD", "COMPOUND_DIMENSION", "EXT_KEYWORD", "INT_KEYWORD"
};


void AEPClient::ClientTerminateToken::aetpPrint
(ostream& os) const
{
  throw AEPException(
    "INTERNAL ERROR: ClientTerminateToken::aetpPrint() called", Log::FATAL
  );
}


void AEPClient::ServerDisconnectToken::aetpPrint
(ostream& os) const
{
  os << "DISCONNECT " << serverSequence << ";\n\n";
}


void AEPClient::NotifyToken::aetpPrint
(ostream& os) const
{
  os << "NOTIFY " << serverSequence << ' ' << targets.size() << ' '
     << nodes.size() << '\n';
  for (int i = 0; i < targets.size(); i++) {
    targets[i]->aetpPrint(os);
  }
  for (int i = 0; i < nodes.size(); i++) {
    os << nodes[i]->canonical() << '\n';
  }
  os << ";\n\n";
}


void AEPClient::NotifyToken::AssignTarget::aetpPrint
(ostream& os) const
{
  os << "ASSIGN " << participantId << ' ' << contextNodeIndex;
  if (dimension != NULL) {
    os << " DIM " << dimension->canonical();
    if (dimensionIsExternal) {
      os << " EXT";
    } else {
      os << " INT";

    }
  } else {
    os << " ROOT";
  }
  os << '\n';
}


void AEPClient::NotifyToken::ApplyTarget::aetpPrint
(ostream& os) const
{
  os << "APPLY " << participantId << ' ' << opNodeIndex;
  if (dimension != NULL) {
    os << " DIM " << dimension->canonical();
    if (dimensionIsExternal) {
      os << " EXT";
    } else {
      os << " INT";
    }
  } else {
    os << " ROOT";
  }
  os << '\n';
}


void AEPClient::NotifyToken::ClearTarget::aetpPrint
(ostream& os) const
{
  os << "CLEAR " << participantId;
  if (dimension != NULL) {
    os << " DIM " << dimension->canonical();
  } else {
    os << " ROOT";
  }
  os << '\n';
}


void AEPClient::NotifyToken::KickTarget::aetpPrint
(ostream& os) const
{
  os << "KICK " << participantId << '\n';
}


void AEPClient::AckToken::aetpPrint
(ostream& os) const
{
  os << "ACK " << serverSequence << ' ' << clientSequence << ";\n\n";
}


void AEPClient::DenyToken::aetpPrint
(ostream& os) const
{
  os << "DENY " << serverSequence << ' ' << clientSequence << ";\n\n";
}


void AEPClient::ErrorToken::aetpPrint
(ostream& os) const
{
  os << "ERROR " << serverSequence << ' ' << clientSequence << ";\n\n";
}


AETPClient::AETPClient
(istream& in, ostream& out, Log* log, Log::Priority logLevel,
 bool errorTolerant, bool useReceiverThread, int maxReceiveSize)
  : AEPStreamClient(
      in, out, log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize
    ),
    lexer(new AETPLexer(in, AETPLexer::AETP_CLIENT_LEXER))
{}


AETPClient::~AETPClient
()
{
  delete lexer;
}


void AETPClient::send
(AEPCommon::Token& token)
{
  token.aetpPrint(*out);
}


AETPClient::Token* AETPClient::receive
()
{
  Token* token;
  LexerToken lexerToken;
  LexerToken::Type type;

  type = (LexerToken::Type)(lexer->getToken(lexerToken));
  switch (type) {
  case LexerToken::VOID:
    throw AEPException("Received VOID AETPClient::LexerToken");
  case LexerToken::SERVER_DISCONNECT:
    {
      long serverSequence;

      // Server sequence:
      readSequence(lexerToken, serverSequence, "server sequence", type);
      // Break:
      if (lexer->getToken(lexerToken) != LexerToken::BREAK) {
        throwBadLexerToken(lexerToken, type);
      }
      token = new ServerDisconnectToken(serverSequence);
    }
    break;
  case LexerToken::NOTIFY:
    {
      long serverSequence;
      int targetsSize;
      int nodesSize;
      bool haveSeenAssign = false;
      bool haveSeenApply = false;

      // Server sequence:
      readSequence(lexerToken, serverSequence, "server sequence", type);
      token = new NotifyToken(serverSequence);
      try {
        // Number of targets:
        ((AETPLexer*)lexer)->integersType = AETPLexer::COUNT;
        if (lexer->getToken(lexerToken) != LexerToken::COUNT) {
          throwBadLexerToken(lexerToken, type);
        }
        targetsSize = lexerToken.aetpValue.count;
        // Number of nodes:
        ((AETPLexer*)lexer)->integersType = AETPLexer::COUNT;
        if (lexer->getToken(lexerToken) != LexerToken::COUNT) {
          throwBadLexerToken(lexerToken, type);
        }
        nodesSize = lexerToken.aetpValue.count;
        for (int i = 0; i < targetsSize; i++) {
          long participantId;
          int nodeIndex;
          CompoundDimension* dimension = NULL;
          bool dimensionIsExternal;
          LexerToken::Type targetType;

          switch ((targetType =
                   (LexerToken::Type)lexer->getToken(lexerToken))) {
          case LexerToken::ASSIGN:
          case LexerToken::APPLY:
            // participantId:
            ((AETPLexer*)lexer)->integersType = AETPLexer::ID;
            if (lexer->getToken(lexerToken) != LexerToken::ID) {
              throwBadLexerToken(lexerToken, type);
            }
            participantId = lexerToken.aetpValue.id;
            // nodeIndex:
            ((AETPLexer*)lexer)->integersType = AETPLexer::COUNT;
            if (lexer->getToken(lexerToken) != LexerToken::COUNT) {
              throwBadLexerToken(lexerToken, type);
            }
            nodeIndex = lexerToken.aetpValue.count;
            if (lexer->getToken(lexerToken) == LexerToken::DIM_KEYWORD) {
              if (lexer->getToken(lexerToken) !=
                  LexerToken::COMPOUND_DIMENSION) {
                throwBadLexerToken(lexerToken, type);
              }
              dimension = lexerToken.aetpValue.compoundDimension;
              lexer->getToken(lexerToken);
              if (lexerToken.getType() == LexerToken::EXT_KEYWORD) {
                dimensionIsExternal = true;
              } else if (lexerToken.getType() == LexerToken::INT_KEYWORD) {
                dimensionIsExternal = false;
              } else {
                throwBadLexerToken(lexerToken, type);
              }
            } else if (lexerToken.getType() != LexerToken::ROOT_KEYWORD) {
              throwBadLexerToken(lexerToken, type);
            }
            if (targetType == LexerToken::ASSIGN) {
              haveSeenAssign = true;
              if (dimension != NULL) {
                ((NotifyToken*)token)->addTarget(
                  *(new NotifyToken::AssignTarget(
                    participantId, nodeIndex, *dimension, dimensionIsExternal
                  ))
                );
              } else {
                ((NotifyToken*)token)->addTarget(
                  *(new NotifyToken::AssignTarget(participantId, nodeIndex))
                );
              }
            } else /* targetType == LexerToken::APPLY */ {
              haveSeenApply = true;
              if (dimension != NULL) {
                ((NotifyToken*)token)->addTarget(
                  *(new NotifyToken::ApplyTarget(
                    participantId, nodeIndex, *dimension, dimensionIsExternal
                  ))
                );
              } else {
                ((NotifyToken*)token)->addTarget(
                  *(new NotifyToken::ApplyTarget(participantId, nodeIndex))
                );
              }
            }
            // Assign/ApplyTarget constructor does NOT copy dimensions.
            break;
          case LexerToken::CLEAR:
            // participantId:
            ((AETPLexer*)lexer)->integersType = AETPLexer::ID;
            if (lexer->getToken(lexerToken) != LexerToken::ID) {
              throwBadLexerToken(lexerToken, type);
            }
            participantId = lexerToken.aetpValue.id;
            if (lexer->getToken(lexerToken) == LexerToken::DIM_KEYWORD) {
              if (lexer->getToken(lexerToken) !=
                  LexerToken::COMPOUND_DIMENSION) {
                throwBadLexerToken(lexerToken, type);
              }
              dimension = lexerToken.aetpValue.compoundDimension;
            } else if (lexerToken.getType() != LexerToken::ROOT_KEYWORD) {
              throwBadLexerToken(lexerToken, type);
            }
            if (dimension != NULL) {
              ((NotifyToken*)token)->addTarget(
                *(new NotifyToken::ClearTarget(participantId, *dimension))
              );
            } else {
              ((NotifyToken*)token)->addTarget(
                *(new NotifyToken::ClearTarget(participantId))
              );
            }
            // ClearTarget constructor copies non-NULL dimensions:
            delete dimension;
            break;
          case LexerToken::KICK:
            // participantId:
            ((AETPLexer*)lexer)->integersType = AETPLexer::ID;
            if (lexer->getToken(lexerToken) != LexerToken::ID) {
              throwBadLexerToken(lexerToken, type);
            }
            participantId = lexerToken.aetpValue.id;
            ((NotifyToken*)token)->addTarget(
              *(new NotifyToken::KickTarget(participantId))
            );
            break;
          default:
            throwBadLexerToken(lexerToken, type);
          }
        }
        for (int i = 0; i < nodesSize; i++) {
          Context* node;

          if (haveSeenAssign) {
            node = new Context;
          } else if (haveSeenApply) {
            node = new ContextOp;
          } else {
            throw AEPException(
              "Received NOTIFY with non-zero node count but no assign or "
              "apply targets"
            );
          }
          try {
            lexer->beginInContext();
            node->recogniseNode(*lexer);
            lexer->beginInToken();
          } catch (...) {
            delete node;
            throw;
          }
          ((NotifyToken*)token)->addNode(*node);
        }
      } catch (...) {
        delete token;
        throw;
      }
    }
    if (lexer->getToken(lexerToken) != LexerToken::BREAK) {
      throwBadLexerToken(lexerToken, type);
    }
    break;
  case LexerToken::ACK:
  case LexerToken::DENY:
    {
      long clientSequence;
      long serverSequence;
      string* message = NULL;

      // Server sequence:
      readSequence(lexerToken, serverSequence, "server sequence", type);
      // Client sequence:
      readSequence(lexerToken, clientSequence, "client sequence", type);
      // Message or break:
      if (lexer->getToken(lexerToken) == LexerToken::TEXT) {
        message = lexerToken.aetpValue.text;
        lexer->getToken(lexerToken);
        if (lexer->getToken(lexerToken) != LexerToken::BREAK) {
          throwBadLexerToken(lexerToken, type);
        }
      } else if (lexerToken.getType() != LexerToken::BREAK) {
        throwBadLexerToken(lexerToken, type);
      }
      switch (type) {
      case LexerToken::ACK:
        token = new AckToken(serverSequence, clientSequence, message);
        break;
      case LexerToken::DENY:
        token = new DenyToken(serverSequence, clientSequence, message);
        break;
      case LexerToken::ERROR:
        token = new ErrorToken(serverSequence, clientSequence, message);
        break;
      }
    }
    break;
  case LexerToken::ERROR:
    {
      ostringstream oss;

      oss << "Parsed error token in AETPClient::receive: \""
          << *(lexerToken.value.errorText) << "\"";
      throw AEPException(oss.str());
    }
  default:
    {
      ostringstream oss;

      oss << "INTERNAL ERROR: Parsed bad token type " << lexerToken.getType()
          << " in AETPClient::receive";
      throw AEPException(oss.str(), Log::FATAL);
    }
  }
  return token;
}


void AETPClient::readSequence
(LexerToken& lexerToken, long& dest, const char* sequenceName,
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


void AETPClient::readParticipantId
(LexerToken& lexerToken, long& dest, LexerToken::Type inTokenType)
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


void AETPClient::throwBadLexerToken
(LexerToken& token, LexerToken::Type inTokenType)
{
  ostringstream oss;

  oss << "Unexpected token of type " << token.getTypeString()
      << " in " << LexerToken::getTypeString(inTokenType);
  throw AEPException(oss.str());
}
