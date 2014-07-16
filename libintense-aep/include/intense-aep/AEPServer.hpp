// ****************************************************************************
//
// AEPServer.hpp : AEP 2.0 server, for the abstract/core AEP implementation.
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


#ifndef __AEPSERVER_HPP__
#define __AEPSERVER_HPP__


namespace intense {


  namespace aep {


    class AEPServer
      : public AEPCommon {

    protected:

      class Participant;

    public:

      class Token;
      class JoinToken;
      class LeaveToken;
      class AsynchronousToken;
      class AssignToken;
      class ApplyToken;
      class ClearToken;
      class KickToken;

    public:


      // AEther with a server sequence provider - this implementation keeps
      // valueRefFactory from intense::AEther, so that only the root node is
      // an AEPServer::AEther - children are intense::AEthers.
      class AEther
        : public intense::AEther {

        friend class AEPServer;
        friend class Participant;
        friend class AssignToken;
        friend class ApplyToken;
        friend class ClearToken;
        friend class KickToken;

      public:


        // A participant-effects accumulator for asynchonous aether operations
        // (assign, apply, clear) as well as kick.  For a full description,
        // see AEPServer.cpp (search for AEPServer::AEther::Origin):
        class Origin
          : public intense::Origin {

          friend class AEther;
          friend class Participant;
          friend class AssignToken;
          friend class ApplyToken;
          friend class ClearToken;
          friend class KickToken;

          AEther* aether;

          struct LessThanAEPServerPtr {

            bool operator()
              (const AEPServer* s1, const AEPServer* s2)
            {
              return (long)s1 < (long)s2;
            }

          };

          typedef std::map<AEPServer*, AEther*, LessThanAEPServerPtr>
          AEPServerMap;

          // Maps AEPServer pointers to the minimal node required for that
          // server, for the current operation (based on
          // propagate-first/in-order operation application from libintense):
          AEPServerMap serverMap;

          AEPServer::AsynchronousToken* sourceToken;

        protected:

          Origin
          (AEther& aether_)
            : aether(&aether_), serverMap()
          {}

          void beginOperation
          (AEPServer::AsynchronousToken& sourceToken);

          void clear
          ();

          void execute
          ();

          virtual ~Origin
          ()
          {}

          void addServerMapEntry
          (AEPServer* server, AEther* node);

        public:

          AEPServer::AsynchronousToken& getSourceToken
          ();

        };


        long serverSequence;

        pthread_t thread;

        bool running;

        pthread_mutex_t mutex;

        pthread_cond_t condition;

        std::queue<AEPServer::Token*> incoming;

        Origin origin;

        AEPServer::AsynchronousToken* accumulated;

        int accumulatedCount;

        int maxAccumulatedCount;

        const static int defaultMaxAccumulatedCount = 1000;

        io::Log* log;

        io::Log::Priority logLevel;

      public:

        AEther
        (int maxAccumulatedCount_ = defaultMaxAccumulatedCount,
         io::Log* log_ = NULL, io::Log::Priority logLevel_ = io::Log::ERROR);

        virtual ~AEther
        ();

        void append
        (AEPServer::Token& token);

        void signal
        ();

        void wait
        (pthread_cond_t& waitCondition);

        void accumulate
        (AEPServer::AsynchronousToken& token);

        virtual void applyAccumulated
        ();

        void lock
        ();

        void unlock
        ();

        virtual long nextServerSequence
        ()
        {
          return serverSequence++;
        }

        void setLogLevel
        (io::Log::Priority logLevel_)
        {
          pthread_mutex_lock(&mutex);
          logLevel = logLevel_;
          pthread_mutex_unlock(&mutex);
        }

      protected:

        static void cleanupHandler
        (void* aetherAsVoidPointer);

        static void* main
        (void* aetherAsVoidPointer);

        void processToken
        (AEPServer::Token& token);

      };


    protected:


      class Participant
        : public AEPCommon::Participant,
          public intense::AEtherParticipant {

        friend class AEPServer;
        friend class AEther;
        friend class JoinToken;
        friend class LeaveToken;

        AEPServer* server;

        CompoundDimension* dimension;

      public:

        virtual ~Participant
        ();

      protected:

        Participant
        (AEPServer& server_, long id, io::Log* log = NULL,
         io::Log::Priority logLevel = io::Log::NOTICE)
          : AEPCommon::Participant(
              id, ((log == NULL) ? server_.log : log), logLevel
            ),
            server(&server_), dimension(NULL)
        {}

        virtual void leave
        ();

        virtual void join
        (intense::AEther& a);

        virtual long assign
        (const Context& context, const CompoundDimension* dim);

        AEther::Origin& getNotificationOrigin
        (Origin* origin, const char* location);

        virtual void assignNotify
        (const Context& value, const CompoundDimension* dim, Origin* origin);

        virtual long apply
        (const ContextOp& op, const CompoundDimension* dim);

        virtual void applyNotify
        (const ContextOp& o, const CompoundDimension* dim, Origin* origin);

        virtual long clear
        (const CompoundDimension* dim);

        virtual void clearNotify
        (const CompoundDimension* dim, Origin* origin);

        virtual bool isPure
        ()
        {
          return false;
        }

        virtual void kickNotify
        (Origin* origin);

      };


    public:


      class Token
        : public AEPCommon::Token {

        friend class AEPServer;
        friend class AEther;

        // META FIX ME
        //      protected:
      public:

        AEPServer* server;

        long clientSequence;

        long serverSequence;

      public:

        typedef enum {
          VOID, SYNCH, CLIENT_DISCONNECT, JOIN, LEAVE, ASSIGN, APPLY, CLEAR,
          KICK
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
        (long clientSequence_)
          : server(NULL), clientSequence(clientSequence_), serverSequence(-1)
        {}

        virtual ~Token
        ()
        {}

      protected:

        static Token* factory
        (Type type);

        virtual void aetherHandle
        (AEther& aether) = 0;

      public:

        static void factorySerialise
        (const Token* token, io::BaseSerialiser& baseSerialiser,
         std::ostream& os);

        static Token* factoryDeserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        void setServer
        (AEPServer& server_)
        {
          server = &server_;
        }

        long getClientSequence
        ()
        {
          return clientSequence;
        }

        long getServerSequence
        ()
        {
          return serverSequence;
        }

      };


      class SynchronousToken
        : public Token {

      protected:

        AEPClient::Token* reply;

        SynchronousToken
        ()
          : Token(-1), reply(NULL)
        {}

        SynchronousToken
        (long clientSequence)
          : Token(clientSequence), reply(NULL)
        {}

      public:

        virtual ~SynchronousToken
        ()
        {
          delete reply;
        }

        void resumeServer
        ();

        bool aetherTransaction
        (const char* location);

      };


      class SynchToken
        : public SynchronousToken {

        friend class AEPServer;
        friend class AEther;

      public:

        SynchToken
        ()
          : SynchronousToken(-1)
        {}

        SynchToken
        (long clientSequence_)
          : SynchronousToken(clientSequence_)
        {}

        ~SynchToken
        ()
        {}

        Type getType
        ()
        {
          return SYNCH;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        bool handle
        ();

        virtual void aetherHandle
        (AEther& aether);

        void aetpPrint
        (std::ostream& os) const;

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      class ClientDisconnectToken
        : public SynchronousToken {

        friend class AEPServer;
        friend class AEther;

      public:

        ClientDisconnectToken
        ()
          : SynchronousToken(-1)
        {}

        ClientDisconnectToken
        (long clientSequence_)
          : SynchronousToken(clientSequence_)
        {}

        ~ClientDisconnectToken
        ()
        {}

        Type getType
        ()
        {
          return CLIENT_DISCONNECT;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        bool handle
        ();

        virtual void aetherHandle
        (AEther& aether);

        void aetpPrint
        (std::ostream& os) const;

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      class JoinToken
        : public SynchronousToken {

        friend class AEPServer;
        friend class AEther;

        long participantId;

        bool notify;

        CompoundDimension* dimension;

        Participant* participant;

      public:

        JoinToken
        ()
          : SynchronousToken(-1), participantId(-1), notify(true),
            dimension(NULL), participant(NULL)
        {}

        JoinToken
        (long clientSequence_, long participantId_,
         bool notify_, CompoundDimension* dimension_)
          : SynchronousToken(clientSequence_), participantId(participantId_),
            notify(notify_),
            dimension(
              (dimension_ != NULL) ? new CompoundDimension(*dimension_)
                                   : NULL
            )
        {}

        virtual ~JoinToken
        ();

        Type getType
        ()
        {
          return JOIN;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        bool handle
        ();

        virtual void aetherHandle
        (AEther& aether);

        void aetpPrint
        (std::ostream& os) const;

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      class LeaveToken
        : public SynchronousToken {

        friend class AEPServer;
        friend class AEther;

        long participantId;

        Participant* participant;

      public:

        LeaveToken
        ()
          : SynchronousToken(-1), participantId(-1), participant(NULL)
        {}

        LeaveToken
        (long clientSequence_, long participantId_)
          : SynchronousToken(clientSequence_), participantId(participantId_)
        {}

        virtual ~LeaveToken
        ()
        {}

        Type getType
        ()
        {
          return LEAVE;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        bool handle
        ();

        virtual void aetherHandle
        (AEther& aether);

        void aetpPrint
        (std::ostream& os) const;

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      // For now, we do not allow participant-originated (IRC-like) kicking.
      // This is an internal token for use by AEPServer::AEther.  Eventually,
      // when clusterable participant identifiers (probably just
      // server-id/participant-id pairs) get worked in, this will not be an
      // issue:
      class KickToken
        : public SynchronousToken {

        friend class AEPServer;

        long participantId;

      public:

        KickToken
        ()
          : SynchronousToken(-1), participantId(-1)
        {}

        KickToken
        (long clientSequence_, long participantId_)
          : SynchronousToken(clientSequence_), participantId(participantId_)
        {}

        virtual ~KickToken
        ()
        {}

        Type getType
        ()
        {
          return KICK;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        bool handle
        ();

        virtual void aetherHandle
        (AEther& aether);

        void aetpPrint
        (std::ostream& os) const;

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      // AsynchronousToken flag bits:

      // If set, this bit indicates that the token cannot be applied to
      // previous outstanding asynchronous tokens, prior to application to the
      // aether; rather, they will be applied to the aether first:
      const static int preFenceBit = 0x01;

      // If set, this token will be applied to the aether as soon as it has
      // been accumulated, and no subsequence asynchronous operations will be
      // applied to it:
      const static int postFenceBit = 0x02;

      // Notify the originating participant, during application of this
      // operation (only valid if both preFenceBit and postFenceBit are set):
      const static int notifySelfBit = 0x04;

      // Notify participants in the same client as the the originating
      // participant, during application of this operation (only
      // valid if both preFenceBit and postFenceBit are set):
      const static int notifyClientBit = 0x08;


      class AsynchronousToken
        : public Token {

        friend class AEPServer;
        friend class AEther;
        friend class Participant;

      protected:

        // META: Conditionally use spinlocks, instead...
        pthread_mutex_t mutex;

        int refCount;

        int flags;

      protected:

        CompoundDimension* dimension;

        Participant* participant;

        intense::AEther* targetNode;

        AsynchronousToken
        (int initialRefCount = 1);

        AsynchronousToken
        (CompoundDimension* dimension_, long clientSequence, int flags_,
         int initialRefCount = 1);

        virtual ~AsynchronousToken
        ();

        void lock
        ();

        void unlock
        ();

      public:

        void aetherTransaction
        (const char* location);

        intense::AEther& getTargetNode
        ();

        void setTargetNode
        (AEther& aether)
        {
          targetNode = &aether;
        }

        // Returns a pointer to the result of accumulation.  If NULL,
        // accumulation was not possible (and this token must be applied prior
        // to the argument token).  If the return value is this, then the
        // argument token was accumulated into this token and must be deleted.
        // If the return value is &token, then the argument token was used for
        // the result of accumulation, and this token must be deleted.  If
        // the return value is the address of another AsynchronousToken, then
        // both this token and the argument must be deleted, and the result of
        // accumulation is in the returned token.
        virtual AsynchronousToken* accumulate
        (AsynchronousToken& token) = 0;

        static void decrementRefCount
        (AsynchronousToken& token);

        static void incrementRefCount
        (AsynchronousToken& token);

      };


      class AssignToken
        : public AsynchronousToken {

        friend class AEPServer;
        friend class ApplyToken;
        friend class ClearToken;

      protected:

        Context* context;

        long participantId;

      public:

        AssignToken
        (int initialRefCount = 1)
          : AsynchronousToken(initialRefCount), participantId(-1),
            context(NULL)
        {}

        AssignToken
        (long clientSequence, long participantId_,
         const Context& context_, CompoundDimension* dimension, int flags = 0,
         int initialRefCount = 1)
          : AsynchronousToken(
              dimension, clientSequence, flags, initialRefCount
            ),
            participantId(participantId_), context(new Context(context_))
        {}

        AssignToken
        (long clientSequence, long participantId_,
         const Context* context_, CompoundDimension* dimension, int flags = 0,
         int initialRefCount = 1)
          : AsynchronousToken(
              dimension, clientSequence, flags, initialRefCount
            ),
            participantId(participantId_), context((Context*)context_)
        {}

        virtual ~AssignToken
        ();

        Type getType
        ()
        {
          return ASSIGN;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        bool handle
        ();

        virtual void aetherHandle
        (AEther& aether);

        AsynchronousToken* accumulate
        (AsynchronousToken& token);

        void aetpPrint
        (std::ostream& os) const;

        void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      class ApplyToken
        : public AsynchronousToken {

        friend class AEPServer;
        friend class AssignToken;
        friend class ClearToken;

      protected:

        long participantId;

        ContextOp* op;

      public:

        ApplyToken
        (int initialRefCount = 1)
          : AsynchronousToken(initialRefCount), participantId(-1), op(NULL)
        {}

        ApplyToken
        (long clientSequence, long participantId_,
         const ContextOp& op_, CompoundDimension* dimension, int flags = 0,
         int initialRefCount = 1)
          : AsynchronousToken(
              dimension, clientSequence, flags, initialRefCount
            ),
            participantId(participantId_), op(new ContextOp(op_))
        {}

        ApplyToken
        (long clientSequence, long participantId_,
         const ContextOp* op_, CompoundDimension* dimension, int flags = 0,
         int initialRefCount = 1)
          : AsynchronousToken(
              dimension, clientSequence, flags, initialRefCount
            ),
            participantId(participantId_), op((ContextOp*)op_)
        {}

        virtual ~ApplyToken
        ();

        Type getType
        ()
        {
          return APPLY;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        bool handle
        ();

        virtual void aetherHandle
        (AEther& aether);

        AsynchronousToken* accumulate
        (AsynchronousToken& token);

        void aetpPrint
        (std::ostream& os) const;

        void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


      class ClearToken
        : public AsynchronousToken {

        friend class AEPServer;
        friend class AssignToken;
        friend class ApplyToken;

      protected:

        long participantId;

      public:

        ClearToken
        ()
          : AsynchronousToken(), participantId(-1)
        {}

        ClearToken
        (long clientSequence, long participantId_,
         CompoundDimension* dimension, int flags = 0, int initialRefCount = 1)
          : AsynchronousToken(
              dimension, clientSequence, flags, initialRefCount
            ),
            participantId(participantId_)
        {}

        virtual ~ClearToken
        ()
        {}

        Type getType
        ()
        {
          return CLEAR;
        }

        void serialise
        (io::BaseSerialiser& baseSerialiser, std::ostream& os) const;

        void deserialise
        (io::BaseSerialiser& baseSerialiser, std::istream& is);

        bool handle
        ();

        virtual void aetherHandle
        (AEther& aether);

        AsynchronousToken* accumulate
        (AsynchronousToken& token);

        void aetpPrint
        (std::ostream& os) const;

        void print
        (std::ostream& os, io::Log::Priority logLevel) const;

      };


    protected:

      friend class SynchronousToken;
      friend class ClientDisconnectToken;
      friend class JoinToken;
      friend class LeaveToken;
      friend class AssignToken;
      friend class ApplyToken;
      friend class ClearToken;
      friend class Participant;
      friend class AEther;
      friend class AEther::Origin;

      AEther* aether;

      pthread_t earThread;

      bool running;

      AEPCommon::Token* currentNotifyToken;

      AEPCommon::Token& getCurrentNotifyToken
      ();

      void setCurrentNotifyToken
      (AEPCommon::Token* newToken);

      virtual Token* receive
      () = 0;

      AEPServer
      (AEther& aether_, io::Log* log, io::Log::Priority logLevel,
       bool errorTolerant);

      static void earCleanupHandler
      (void* serverAsVoidPointer);

      static void* earMain
      (void* serverAsVoidPointer);

      bool processToken
      (Token& token);

      // Essentially instream->peek() in stream server, and condition wait in
      // shared server.  Returns true if lock is held:
      virtual bool blockForAvailableData
      () = 0;

      virtual void lockedSend
      (AEPClient::Token& token);

      void lockedAndLoggedSend
      (AEPClient::Token& token, const char* location,
       long serverSequence, long clientSequence);

      void leaveAllParticipants
      ();

    public:

      virtual ~AEPServer
      ();

      void aep
      ();

      void start
      ();

      void stop
      ();

      void stop
      (pthread_t externalEarThread);

      virtual const char* getName
      () = 0;

    };


    class AEPStreamServer
      : public AEPServer {

    protected:

      std::istream* in;

      std::ostream* out;

      AEPStreamServer
      (AEther& aether, std::istream& in_, std::ostream& out_, io::Log* log,
       io::Log::Priority logLevel, bool errorTolerant);

    public:

      // Accept returns a non-NULL AEPStreamServer pointer to a connected
      // AEPStreamServer, or throws an AEPException.

      static AEPStreamServer* accept
      (AEther& aether, std::istream& is, std::ostream& os, io::Log* log = NULL,
       io::Log::Priority logLevel = io::Log::NOTICE,
       bool errorTolerant = true);

      virtual bool blockForAvailableData
      ();

      virtual bool deletesSentTokens
      ()
      {
        return true;
      }

    protected:

      static void throwBadInput
      (std::ostream& os, const char* badInput);

    };


    class AEPSerialServer
      : public AEPStreamServer {

      friend class AEPStreamServer;

    protected:

      io::BaseSerialiser* serialiser;

      void send
      (AEPCommon::Token& token);

      Token* receive
      ();

      AEPSerialServer
      (AEther& aether, std::istream& in, std::ostream& out,
       io::BaseSerialiser& serialiser_, io::Log* log = NULL,
       io::Log::Priority logLevel = io::Log::NOTICE,
       bool errorTolerant = true);

      virtual ~AEPSerialServer
      ();

    };


    class AEPBinaryServer
      : public AEPSerialServer {

      friend class AEPStreamServer;

    public:

      AEPBinaryServer
      (AEther& aether, std::istream& in, std::ostream& out,
       io::Log* log = NULL, io::Log::Priority logLevel = io::Log::NOTICE,
       bool errorTolerant = true);

      virtual ~AEPBinaryServer
      ();

      const char* getName
      ();

    };


    class AEPXDRServer
      : public AEPSerialServer {

      friend class AEPStreamServer;

    protected:

      io::XDRBaseSerialiser serialiser;

    public:

      AEPXDRServer
      (AEther& aether, std::istream& in, std::ostream& out,
       io::Log* log = NULL, io::Log::Priority logLevel = io::Log::NOTICE,
       bool errorTolerant = true);

      virtual ~AEPXDRServer
      ();

      const char* getName
      ();

    };


    class AETPServer
      : public AEPStreamServer {

      friend class AETPLexer;
      friend class AEPStreamServer;

      AETPLexer* lexer;

    protected:

      void send
      (AEPCommon::Token& token);

      Token* receive
      ();

    public:

      AETPServer
      (AEther& aether, std::istream& in, std::ostream& out, io::Log* log = NULL,
       io::Log::Priority logLevel = io::Log::NOTICE, bool errorTolerant = true);

      virtual ~AETPServer
      ();

      const char* getName
      ();


      class LexerToken
        : public Context::Token {

        friend class AETPServer;

        static const char* typeStrings[];

      public:

        typedef enum {
          // ContextOp::Token types (preserving enum sequence):
          VOID, DIMENSION, BASEVALUE, DASH, DASHDASH, DASHDASHDASH, PLUS,
          LANGLE, RANGLE, LSQUARE, RSQUARE, ERROR, ENDOFFILE,
          // Extra AETPServer::LexerToken types:
          BREAK, SYNCH, CLIENT_DISCONNECT, JOIN, LEAVE, ASSIGN, APPLY, CLEAR,
          SELF, CLIENT, PREFENCE, POSTFENCE, ID, SEQUENCE, TEXT, DIM_KEYWORD,
          COMPOUND_DIMENSION, CONTEXT_KEYWORD, OP_KEYWORD, NOTIFY
        } Type;

        const char** getTypeStrings
        ()
        {
          return typeStrings;
        }

        const char* getTypeString
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
          int id;
          long sequence;
          CompoundDimension* compoundDimension;
          std::string* text;
          std::string* errorText;
        } aetpValue;

      public:

        LexerToken
        ()
          : Context::Token()
        {}

        virtual ~LexerToken
        ()
        {}

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
        (int id)
        {
          aetpValue.id = id;
          return type = ID;
        }

        int setSequence
        (long sequence)
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

        int setCompoundDimension
        (CompoundDimension& compoundDimension)
        {
          aetpValue.compoundDimension = &compoundDimension;
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

      };


    private:

      void readSequence
      (LexerToken& lexerToken, long& dest, const char* sequenceName,
       LexerToken::Type inTokenType);

      void readParticipantId
      (LexerToken& lexerToken, long& dest, LexerToken::Type inTokenType);

      void throwBadLexerToken
      (LexerToken& token, LexerToken::Type inTokenType);

    };


    class AEPSharedClient;


    class AEPSharedServer
      : public AEPServer {

      friend class AEPSharedClient;

      AEPSharedClient* client;

      std::queue<Token*> incoming;

      pthread_cond_t receptionCondition;

    public:

      AEPSharedServer
      (AEther& aether, io::Log* log = NULL,
       io::Log::Priority logLevel = io::Log::NOTICE,
       bool errorTolerant = true);

      virtual ~AEPSharedServer
      ();

      const char* getName
      ();

    protected:

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


#endif // __AEPSERVER_HPP__
