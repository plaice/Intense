// ****************************************************************************
//
// AEPClient.hpp : AEP 2.0 client, for the abstract/core AEP implementation.
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


#ifndef __AEPCLIENT_HPP__
#define __AEPCLIENT_HPP__


namespace intense {


  namespace aep {


    class AEPClient
      : public AEPCommon {

    public:


      class Participant
        : public AEPCommon::Participant {

        friend class AEPClient;

        AEPClient* client;

      protected:

        Participant
        (io::Log* log_ = NULL, io::Log::Priority logLevel_ = io::Log::NOTICE)
          : AEPCommon::Participant(log, logLevel), client(NULL)
        {}

        Participant
        (AEPClient& client_, io::Log* log = NULL,
         io::Log::Priority logLevel = io::Log::NOTICE)
          : AEPCommon::Participant(
              ((log == NULL) ? client_.log : log), logLevel
            ),
            client(&client_)
        {}

      public:

        long long join
        (AEPClient& newClient, bool notify,
         const CompoundDimension* dimension = NULL,
         pthread_cond_t* optionalBlockCondition = NULL);

        long long join
        (AEPClient& newClient, bool notify, const char* dimension,
         pthread_cond_t* optionalBlockCondition = NULL);

        long long join
        (bool notify, const CompoundDimension* dimension = NULL,
         pthread_cond_t* optionalBlockCondition = NULL);

        long long join
        (bool notify, const char* dimension,
         pthread_cond_t* optionalBlockCondition = NULL);

        virtual bool isPure
        ()
        {
          return false;
        }

      private:

        long long join
        (AEPClient* newClient, bool notify, const CompoundDimension* dimension,
         pthread_cond_t* blockCondition);

      public:

        long long leave
        (pthread_cond_t* blockCondition = NULL);

        long long assign
        (const Context& context)
        {
          return assign(context, (const CompoundDimension*)NULL, 0, NULL);
        }

        virtual long long assign
        (const Context& context, const CompoundDimension* dim)
        {
          return assign(context, dim, 0, NULL);
        }

        virtual long long assign
        (const Context& context, const CompoundDimension* dim, int flags)
        {
          return assign(context, dim, flags, NULL);
        }

        virtual long long assign
        (const Context& context, const char* dim)
        {
          return assign(context, dim, 0, NULL);
        }

        virtual long long assign
        (const Context& context, const char* dim, int flags)
        {
          return assign(context, dim, flags, NULL);
        }

        virtual long long assign
        (const Context& context, const char* dim, int flags,
         pthread_cond_t* blockCondition);

        virtual long long assign
        (const Context& context, const CompoundDimension* dim, int flags,
         pthread_cond_t* blockCondition);

        virtual long long apply
        (const ContextOp& op)
        {
          return apply(op, (const CompoundDimension*)NULL, 0, NULL);
        }

        virtual long long apply
        (const ContextOp& op, const CompoundDimension* dim)
        {
          return apply(op, dim, 0, NULL);
        }

        virtual long long apply
        (const ContextOp& op, const CompoundDimension* dim, int flags)
        {
          return apply(op, dim, flags, NULL);
        }

        virtual long long apply
        (const ContextOp& op, const char* dim)
        {
          return apply(op, dim, 0, NULL);
        }

        virtual long long apply
        (const ContextOp& op, const char* dim, int flags)
        {
          return apply(op, dim, flags, NULL);
        }

        virtual long long apply
        (const ContextOp& op, const char* dim, int flags,
         pthread_cond_t* blockCondition);

        virtual long long apply
        (const ContextOp& op, const CompoundDimension* dim, int flags,
         pthread_cond_t* blockCondition);

        virtual long long clear
        ()
        {
          return clear((const CompoundDimension*)NULL, 0, NULL);
        }

        virtual long long clear
        (const CompoundDimension* dim)
        {
          return clear(dim, 0, NULL);
        }

        virtual long long clear
        (const CompoundDimension* dim, int flags)
        {
          return clear(dim, flags, NULL);
        }

        virtual long long clear
        (const char* dim)
        {
          return clear(dim, 0, NULL);
        }

        virtual long long clear
        (const char* dim, int flags)
        {
          return clear(dim, flags, NULL);
        }

        virtual long long clear
        (const char* dim, int flags, pthread_cond_t* blockCondition);

        virtual long long clear
        (const CompoundDimension* dim, int flags,
         pthread_cond_t* blockCondition);

        virtual long long synch
        ()
        {
          return synch((pthread_cond_t*)NULL);
        }

        virtual long long synch
        (pthread_cond_t* blockCondition);

        virtual void synch
        (long long serverSequence)
        {
          return synch(serverSequence, NULL);
        }

        virtual void synch
        (long long serverSequence, pthread_cond_t* blockCondition);

      private:

        void checkValidClientAndJoined
        (const char* location);

      };


      class Token
        : public AEPCommon::Token {

        friend class AEPClient;

      protected:

        AEPClient* client;

        long long serverSequence;

      public:

        typedef enum {
          VOID, CLIENT_TERMINATE, SERVER_DISCONNECT, NOTIFY, ACK, DENY, ERROR
        } Type;

        static const char* typeStrings[];

        virtual Type getType
        () = 0;

        const char* getTypeString
        ()
        {
          return typeStrings[getType()];
        }

        static const char* getTypeString
        (Type type)
        {
          return typeStrings[type];
        }

        Token
        (long long serverSequence_)
          : client(NULL), serverSequence(serverSequence_)
        {}

        virtual ~Token
        ()
        {}

      protected:

        static Token* factory
        (Type type);

      public:

        static void factorySerialise
        (const Token* token, io::BaseSerialiser& baseSerialiser,
         std::ostream& os);

        static Token* factoryDeserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        void setClient
        (AEPClient& client_)
        {
          client = &client_;
        }

        long long getServerSequence
        ()
        {
          return serverSequence;
        }

      };


      class ClientTerminateToken
        : public Token {

        friend class AEPClient;

        std::string* message;

      public:

        ClientTerminateToken
        (std::string& message_)
          : Token(-1), message(&message_)
        {}

        ~ClientTerminateToken
        ()
        {
          delete message;
        }

        Type getType
        ()
        {
          return CLIENT_TERMINATE;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        bool handle
        ();

        void aetpPrint
        (std::ostream& os) const;

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      class ServerDisconnectToken
        : public Token {

        friend class AEPClient;

      public:

        ServerDisconnectToken
        ()
          : Token(-1)
        {}

        ServerDisconnectToken
        (long long serverSequence_)
          : Token(serverSequence_)
        {}

        ~ServerDisconnectToken
        ()
        {}

        Type getType
        ()
        {
          return SERVER_DISCONNECT;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        bool handle
        ();

        void aetpPrint
        (std::ostream& os) const;

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      class NotifyToken
        : public Token, public Origin {

        friend class AEPClient;

      public:

        class Target
          : public Serialisable {

        public:

          typedef enum {
            ASSIGN, APPLY, CLEAR, KICK
          } Type;

          static const char* typeStrings[];

          long long participantId;

          virtual Type getType
          () = 0;

          const char* getTypeString
          ()
          {
            return typeStrings[getType()];
          }

          static const char* getTypeString
          (Type type)
          {
            return typeStrings[type];
          }

          virtual ~Target
          ();

          void serialise
          (io::BaseSerialiser& baseSerialiser, std::ostream& os) const = 0;

          void deserialise
          (io::BaseSerialiser& baseSerialiser, std::istream& is) = 0;

          virtual void print
          (std::ostream& os, io::Log::Priority logLevel, NotifyToken& token)
            const = 0;

          virtual void aetpPrint
          (std::ostream& os) const = 0;

          virtual void handle
          (NotifyToken& token) = 0;

          static Target* factory
          (Type type);

          static void factorySerialise
          (const Target* target, io::BaseSerialiser& baseSerialiser,
           std::ostream& os);

          static Target* factoryDeserialise
          (io::BaseSerialiser& baseSerialiser, std::istream& is);

        protected:

          Target
          (long long participantId_)
            : participantId(participantId_)
          {}

        private:

          Target
          ()
          {}

        };


        class AssignTarget
          : public Target {

          int contextNodeIndex;

          CompoundDimension* dimension;

          bool dimensionIsExternal;

          AEPCommon::Token* assignToken;

        public:

          AssignTarget
          ()
            : Target(-1), contextNodeIndex(-1), dimension(NULL),
              dimensionIsExternal(false), assignToken(NULL)
          {}

          AssignTarget
          (long long participantId, int contextNodeIndex_)
            : Target(participantId), contextNodeIndex(contextNodeIndex_),
              dimension(NULL), dimensionIsExternal(false), assignToken(NULL)
          {}

          AssignTarget
          (long long participantId, int contextNodeIndex_,
           CompoundDimension& dimension_, bool dimensionIsExternal_)
            : Target(participantId), contextNodeIndex(contextNodeIndex_),
              // Copy pointer directly - it's freshly allocated in addNode():
              dimension(&dimension_),
              dimensionIsExternal(dimensionIsExternal_), assignToken(NULL)
          {}

          virtual ~AssignTarget
          ();

          void setAssignToken
          (AEPCommon::Token& assignToken);

          void serialise
          (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

          void deserialise
          (io::BaseSerialiser& baseSerialiser, std::istream& is);

          void print
          (std::ostream& os, io::Log::Priority logLevel, NotifyToken& token)
            const;

          virtual void aetpPrint
          (std::ostream& os) const;

          void handle
          (NotifyToken& token);

          virtual Type getType
          ()
          {
            return ASSIGN;
          }

        };


        class ApplyTarget
          : public Target {

          int opNodeIndex;

          CompoundDimension* dimension;

          bool dimensionIsExternal;

          AEPCommon::Token* applyToken;

        public:

          ApplyTarget
          ()
            : Target(-1), opNodeIndex(-1), dimension(NULL),
              dimensionIsExternal(false), applyToken(NULL)
          {}

          ApplyTarget
          (long long participantId, int opNodeIndex_)
            : Target(participantId), opNodeIndex(opNodeIndex_),
              dimension(NULL), dimensionIsExternal(false), applyToken(NULL)
          {}

          ApplyTarget
          (long long participantId, int opNodeIndex_,
           CompoundDimension& dimension_, bool dimensionIsExternal_)
            : Target(participantId), opNodeIndex(opNodeIndex_),
              // Copy pointer directly - it's freshly allocated in addNode():
              dimension(&dimension_),
              dimensionIsExternal(dimensionIsExternal_), applyToken(NULL)
          {}

          virtual ~ApplyTarget
          ();

          void setApplyToken
          (AEPCommon::Token& applyToken);

          void serialise
          (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

          void deserialise
          (io::BaseSerialiser& baseSerialiser, std::istream& is);

          void print
          (std::ostream& os, io::Log::Priority logLevel, NotifyToken& token)
            const;

          virtual void aetpPrint
          (std::ostream& os) const;

          void handle
          (NotifyToken& token);

          virtual Type getType
          ()
          {
            return APPLY;
          }

        };


        class ClearTarget
          : public Target {

          CompoundDimension* dimension;

        public:

          ClearTarget
          ()
            : Target(-1), dimension(NULL)
          {}

          ClearTarget
          (long long participantId)
            : Target(participantId), dimension(NULL)
          {}

          ClearTarget
          (long long participantId, CompoundDimension& dimension_)
            : Target(participantId),
              dimension(new CompoundDimension(dimension_))
          {}

          virtual ~ClearTarget
          ();

          void serialise
          (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

          void deserialise
          (io::BaseSerialiser& baseSerialiser, std::istream& is);

          void print
          (std::ostream& os, io::Log::Priority logLevel, NotifyToken& token)
            const;

          virtual void aetpPrint
          (std::ostream& os) const;

          void handle
          (NotifyToken& token);

          virtual Type getType
          ()
          {
            return CLEAR;
          }

        };


        class KickTarget
          : public Target {

        public:

          KickTarget
          ()
            : Target(-1)
          {}

          KickTarget
          (long long participantId)
            : Target(participantId)
          {}

          virtual ~KickTarget
          ()
          {}

          void serialise
          (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

          void deserialise
          (io::BaseSerialiser& baseSerialiser, std::istream& is);

          void print
          (std::ostream& os, io::Log::Priority logLevel, NotifyToken& token)
            const;

          virtual void aetpPrint
          (std::ostream& os) const;

          void handle
          (NotifyToken& token);

          virtual Type getType
          ()
          {
            return KICK;
          }

        };


      protected:

        std::vector<Target*> targets;

        std::vector<Context*> nodes;

      public:

        NotifyToken
        ()
          : Token(-1), targets(), nodes()
        {}

        NotifyToken
        (long long serverSequence_)
          : Token(serverSequence_), targets(), nodes()
        {}

        Context& getContext
        (int nodeIndex, CompoundDimension* dimension);

        ContextOp& getContextOp
        (int nodeIndex, CompoundDimension* dimension);

        void addTarget
        (Target& target);

        // For AEPServer::Participant:
        void addNode
        (Context& node, int& nodeIndex, CompoundDimension*& internalDimension);

        // For AETPClient:
        void addNode
        (Context& node);

      public:

        virtual ~NotifyToken
        ();

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

        virtual void shallowClear
        ();

        Type getType
        ()
        {
          return NOTIFY;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        bool handle
        ();

        void aetpPrint
        (std::ostream& os) const;

      };


      class ReplyToken
        : public Token {

        friend class AEPClient;

      protected:

        long long clientSequence;

        std::string* message;

      public:

        ReplyToken
        ()
          : Token(-1), clientSequence(-1), message(NULL)
        {}

        ReplyToken
        (long long serverSequence_, long clientSequence_, std::string* message_)
          : Token(serverSequence_), clientSequence(clientSequence_),
            message(message_)
        {}

        ~ReplyToken
        ()
        {
          delete message;
        }

        Type getType
        ()
        {
          return ACK;
        }

        long long getClientSequence
        ()
        {
          return clientSequence;
        }

        std::string* getMessage
        ()
        {
          return message;
        }

        bool handle
        ();

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      class AckToken
        : public ReplyToken {

        friend class AEPClient;

      public:

        AckToken
        ()
          : ReplyToken()
        {}

        AckToken
        (long long serverSequence, long long clientSequence,
         std::string* message = NULL)
          : ReplyToken(serverSequence, clientSequence, message)
        {}

        ~AckToken
        ()
        {}

        Type getType
        ()
        {
          return ACK;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        void aetpPrint
        (std::ostream& os) const;

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      class DenyToken
        : public ReplyToken {

        friend class AEPClient;

      public:

        DenyToken
        ()
          : ReplyToken()
        {}

        DenyToken
        (long long serverSequence, long long clientSequence,
         std::string* message = NULL)
          : ReplyToken(serverSequence, clientSequence, message)
        {}

        ~DenyToken
        ()
        {}

        Type getType
        ()
        {
          return DENY;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        void aetpPrint
        (std::ostream& os) const;

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      class ErrorToken
        : public ReplyToken {

        friend class AEPClient;

      public:

        ErrorToken
        (std::string* message_)
          : ReplyToken(-1, -1, message)
        {}

        ErrorToken
        (long long serverSequence, long long clientSequence,
         std::string* message = NULL)
          : ReplyToken(serverSequence, clientSequence, message)
        {}

        ~ErrorToken
        ()
        {}

        Type getType
        ()
        {
          return ERROR;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const
        {}

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is)
        {}

        void aetpPrint
        (std::ostream& os) const;

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


    protected:


      class SequenceBinderMap;


      class SequenceBinder {

        friend class SequenceBinderMap;

        long long sequence;

        bool terminate;

        ReplyToken* token;

        bool allocatedCondition;

        pthread_cond_t* condition;

      public:

        SequenceBinder
        (long long sequence, bool terminate, pthread_cond_t* condition);

        ~SequenceBinder
        ();

      };


      class SequenceBinderMap
        : protected std::map<long long, SequenceBinder*> {

        AEPClient* client;

      public:

        SequenceBinderMap
        (AEPClient& client_)
          : std::map<long long, SequenceBinder*>(), client(&client_)
        {}

        // client->lock() must be acquired prior to call, and is reacquired,
        // after call:
        ReplyToken* wait
        (long long sequence, bool terminate = false,
         pthread_cond_t* condition = NULL);

        // client->lock() must be acquired prior to call.  Returns a bool
        // indicating whether the client ear is to continue running:
        bool resumeAll
        ();

        // client->lock() must be acquired prior to call.  Returns a bool
        // indicating whether the client ear is to continue running:
        bool resume
        (ReplyToken& token);

        // client->lock() must be acquired prior to call.  Returns a bool
        // indicating whether the client ear is to continue running:
        bool resumeUntil
        (long long sequence);

      };


      friend class Token;
      friend class ServerDisconnectToken;
      friend class NotifyToken;
      friend class AckToken;
      friend class DenyToken;
      friend class ErrorToken;
      friend class Participant;

    protected:

      // Clients keep their own sequence; Servers get their sequence from their
      // AEther.
      long long sequence;

      long long serverSequence;

      SequenceBinderMap clientSequenceBinderMap;

      SequenceBinderMap serverSequenceBinderMap;

      bool stopped;

      pthread_t earThread;

      bool usingReceiverThread;

      pthread_t receiverThread;

      int maxReceiveSize;

      std::queue<Token*>* receiverQueue;

      pthread_mutex_t receiverQueueMutex;

      pthread_cond_t receiverQueueCondition;

      // Assumes lock has been acquired:
      long long nextSequence
      ()
      {
        return sequence++;
      }

      // Essentially instream->peek() in stream client, and condition wait in
      // shared client.  Returns true if lock is held:
      virtual bool blockForAvailableData
      () = 0;

      virtual Token* receive
      () = 0;

      AEPClient
      (io::Log* log, io::Log::Priority logLevel, bool errorTolerant,
       bool useReceiverThread, int maxReceiveSize);

      ReplyToken* waitForReply
      (long long clientSequence, const char* location, bool terminate,
       pthread_cond_t* optionalBlockCondition);

      static void earCleanupHandler
      (void* clientAsVoidPointer);

      static void* earMain
      (void* clientAsVoidPointer);

      static void receiverCleanupHandler
      (void* clientAsVoidPointer);

      static void* receiverMain
      (void* clientAsVoidPointer);

      bool processToken
      (Token* token);

      // Assumes client has been locked and unlocks it before returning:
      long long transaction
      (AEPCommon::Token* outgoing_, bool allowDeny, bool terminate,
       pthread_cond_t* blockCondition);

    public:

      virtual ~AEPClient
      ();

      // Direct operations on the remote aether, with no use of participants.
      // These have the same effect as assign, apply and clear operations on
      // a root-node participant:

      virtual long long assign
      (const Context& context, const CompoundDimension* dim = NULL,
       int flags = 0, pthread_cond_t* blockCondition = NULL);

      virtual long long apply
      (const ContextOp& op, const CompoundDimension* dim = NULL,
       int flags = 0, pthread_cond_t* blockCondition = NULL);

      virtual long long clear
      (const CompoundDimension* dim = NULL, int flags = 0,
       pthread_cond_t* blockCondition = NULL);

      virtual long long synch
      (pthread_cond_t* blockCondition);

      virtual void synch
      (long long serverSequence, pthread_cond_t* blockCondition);

      long long getServerSequence
      ()
      {
        return serverSequence;
      }

      virtual long long disconnect
      ()
      {
        return disconnect(NULL);
      }

      virtual long long disconnect
      (pthread_cond_t* blockCondition);

      void start
      ();

      void stop
      ();

    };


    class AEPStreamClient
      : public AEPClient {

    protected:

      std::istream* in;

      std::ostream* out;

      AEPStreamClient
      (std::istream& in_, std::ostream& out_, io::Log* log,
       io::Log::Priority logLevel, bool errorTolerant, bool useReceiverThread,
       int maxReceiveSize)
        : AEPClient(
            log, logLevel, errorTolerant, useReceiverThread, maxReceiveSize
          ),
          in(&in_), out(&out_)
      {}

      virtual bool blockForAvailableData
      ();

      virtual bool deletesSentTokens
      ()
      {
        return true;
      }

    public:

      virtual ~AEPStreamClient
      ();

      static AEPStreamClient* connect
      (std::istream& is, std::ostream& os, Mode mode, io::Log* log,
       io::Log::Priority logLevel);

      static AEPStreamClient* connect
      (std::istream& in, std::ostream& out, Mode mode,
       Participant& participant, const CompoundDimension* dimension,
       pthread_cond_t* optionalBlockCondition,
       io::Log* log, io::Log::Priority logLevel);

      static AEPStreamClient* connect
      (std::istream& in, std::ostream& out, Mode mode,
       Participant& participant, const char* compoundDimensionString,
       pthread_cond_t* optionalBlockCondition,
       io::Log* log, io::Log::Priority logLevel);

      // Connects to the given (aep://host:port/full:compound:dimension) AEP
      // URL, using the given AEPClient::Participant.
      static AEPStreamClient* urlConnect
      (const char* aepURL, io::FileStream& destStream,
       Participant& participant, pthread_cond_t* optionalBlockCondition,
       io::Log* log, io::Log::Priority logLevel);

      // Connects to the given (aep://host:port/ignored:dimension) AEP
      // URL (ignoring any dimension component of the URL):
      static AEPStreamClient* urlConnect
      (const char* aepURL, io::FileStream& destStream, io::Log* log,
       io::Log::Priority logLevel);

      static void streamConnect
      (io::FileStream& destInStream, std::string& host, int port);

    };


    class AEPSerialClient
      : public AEPStreamClient {

    protected:

      io::BaseSerialiser* serialiser;

      AEPSerialClient
      (std::istream& in, std::ostream& out, io::BaseSerialiser& serialiser_,
       io::Log* log = NULL, io::Log::Priority logLevel = io::Log::NOTICE,
       bool errorTolerant = true, bool useReceiverThread = false,
       int maxReceiveSize = 0);

      virtual ~AEPSerialClient
      ();

      void send
      (AEPCommon::Token& token);

      Token* receive
      ();

    };


    class AEPBinaryClient
      : public AEPSerialClient {

    public:

      AEPBinaryClient
      (std::istream& in, std::ostream& out, io::Log* log = NULL,
       io::Log::Priority logLevel = io::Log::NOTICE, bool errorTolerant = true,
       bool useReceiverThread = false, int maxReceiveSize = 0);

      virtual ~AEPBinaryClient
      ();

    };


    class AEPXDRClient
      : public AEPSerialClient {

    public:

      AEPXDRClient
      (std::istream& in, std::ostream& out, io::Log* log = NULL,
       io::Log::Priority logLevel = io::Log::NOTICE, bool errorTolerant = true,
       bool useReceiverThread = false, int maxReceiveSize = 0);

      virtual ~AEPXDRClient
      ();

    };


    class AETPClient
      : public AEPStreamClient {

      friend class AETPLexer;

      AETPLexer* lexer;

    protected:

      void send
      (AEPCommon::Token& token);

      Token* receive
      ();

    public:

      AETPClient
      (std::istream& in, std::ostream& out, io::Log* log = NULL,
       io::Log::Priority logLevel = io::Log::NOTICE, bool errorTolerant = true,
       bool useReceiverThread = false, int maxReceiveSize = 0);

      virtual ~AETPClient
      ();

    protected:


      class LexerToken
        : public Context::Token {

        friend class AETPClient;

        static const char* typeStrings[];

      public:

        typedef enum {
          // ContextOp::Token types (preserving enum sequence):
          VOID, DIMENSION, BASEVALUE, DASH, DASHDASH, DASHDASHDASH, PLUS,
          LANGLE, RANGLE, LSQUARE, RSQUARE, ERROR, ENDOFFILE,
          // Extra AETPClient::LexerToken types:
          BREAK, SERVER_DISCONNECT, NOTIFY, ASSIGN, APPLY, CLEAR, KICK, ACK,
          DENY, COUNT, ID_KEYWORD, ID, SEQUENCE, TEXT, DIM_KEYWORD,
          ROOT_KEYWORD, COMPOUND_DIMENSION, EXT_KEYWORD, INT_KEYWORD
        } Type;

        virtual const char** getTypeStrings
        ()
        {
          return typeStrings;
        }

        virtual const char* getTypeString
        ()
        {
          return typeStrings[type];
        }

        static const char* getTypeString
        (Type type)
        {
          return typeStrings[type];
        }

      protected:

        // Instead of extending the value union in Context::Token, we'll
        // add 8 bytes to these tokens:
        union {
          int count;
          long long id;
          long long sequence;
          CompoundDimension* compoundDimension;
          std::string* text;
        } aetpValue;

      public:

        LexerToken
        ()
          : Context::Token()
        {}

        ~LexerToken
        ()
        {}

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
          aetpValue.count = count;
          return type = COUNT;
        }

        int setIdKeyword
        ()
        {
          return type = ID_KEYWORD;
        }

        int setId
        (long long id)
        {
          aetpValue.id = id;
          return type = ID;
        }

        int setSequence
        (long long sequence)
        {
          aetpValue.sequence = sequence;
          return type = SEQUENCE;
        }

        int setText
        (const char* text)
        {
          aetpValue.text = new std::string(text);
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
        (CompoundDimension& compoundDimension)
        {
          aetpValue.compoundDimension = &compoundDimension;
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


    private:

      void readSequence
      (LexerToken& lexerToken, long long& dest, const char* sequenceName,
       LexerToken::Type inTokenType);

      void readParticipantId
      (LexerToken& lexerToken, long long& dest, LexerToken::Type inTokenType);

      void throwBadLexerToken
      (LexerToken& token, LexerToken::Type inTokenType);

    };


    class AEPSharedServer;


    class AEPSharedClient
      : public AEPClient {

      friend class AEPSharedServer;

      AEPSharedServer* server;

      std::queue<Token*> incoming;

      pthread_cond_t receptionCondition;

      AEPSharedClient
      (io::Log* log = NULL, io::Log::Priority logLevel = io::Log::NOTICE,
       bool errorTolerant = true, bool useReceiverThread = false,
       int maxReceiveSize = 0);

    public:

      virtual ~AEPSharedClient
      ();

      AEPSharedClient
      (AEPSharedServer& server_, io::Log* log = NULL,
       io::Log::Priority logLevel = io::Log::NOTICE, bool errorTolerant = true,
       bool useReceiverThread = false, int maxReceiveSize = 0);

      static AEPSharedClient* connect
      (AEPSharedServer& server, io::Log* log = NULL,
       io::Log::Priority logLevel = io::Log::NOTICE, bool errorTolerant = true,
       bool useReceiverThread = false, int maxReceiveSize = 0);

    protected:

      void attach
      (AEPSharedServer& server);

      void send
      (AEPCommon::Token& token);

      void append
      (Token& token);

      Token* receive
      ();

      virtual bool blockForAvailableData
      ();

      virtual bool deletesSentTokens
      ()
      {
        return false;
      }

    };


  }


}


#endif // __AEPCLIENT_HPP__
