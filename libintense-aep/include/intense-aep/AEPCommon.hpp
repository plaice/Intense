// ****************************************************************************
//
// AEPCommon.hpp : AEP 2.0 client and server base, for the abstract/core AEP
// implementation.
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


#ifndef __AEPCOMMON_HPP__
#define __AEPCOMMON_HPP__


namespace intense {


  namespace aep {


#define AEP_DEFAULT_PORT 6969


    class AEPException
      : public std::runtime_error {

      io::Log::Priority priority;

    public:

      AEPException
      (const std::string& what, io::Log::Priority priority_ = io::Log::ERROR)
        : runtime_error(what)
      {}

      io::Log::Priority getPriority
      ()
      {
        return priority;
      }

    };


    class AEPCommon {

    protected:


      // A wrapper for participants that associates them with an AEPServer or
      // an AEPClient:
      class Participant
        : virtual public intense::Participant {

        friend class AEPCommon;

        long id;

      protected:

        io::Log* log;

        io::Log::Priority logLevel;

        Participant
        (io::Log* log_, io::Log::Priority logLevel_)
          : intense::Participant(), id(-1), log(log_), logLevel(logLevel_)
        {}

        Participant
        (long id_, io::Log* log_, io::Log::Priority logLevel_)
          : intense::Participant(), id(id_), log(log_), logLevel(logLevel_)
        {}

      public:

        virtual ~Participant
        ()
        {}

        long getId
        ()
        {
          return id;
        }

        void setLog
        (io::Log* newLog)
        {
          log = newLog;
        }

        io::Log* getLog
        ()
        {
          return log;
        }

        void setLogLevel
        (io::Log::Priority newLogLevel)
        {
          logLevel = newLogLevel;
        }

        io::Log::Priority setLogLevel
        ()
        {
          return logLevel;
        }

      };


      class Token
        : public io::Serialisable {

      public:

        virtual bool handle
        () = 0;

        virtual void aetpPrint
        (std::ostream& os) const = 0;

        virtual void print
        (std::ostream& os, io::Log::Priority logLevel = io::Log::DEBUG)
          const = 0;

        void print
        (io::Log& log, io::Log::Priority logLevel = io::Log::DEBUG) const;

        virtual ~Token
        ()
        {}

      };


      friend class Participant;

      std::map<long, Participant*> participants;

      long nextParticipantId;

      io::Log* log;

      io::Log::Priority logLevel;

      bool errorTolerant;

      pthread_mutexattr_t mutexAttribute;

      pthread_mutex_t mutex;

      pthread_cond_t condition;

      pthread_mutex_t sendMutex;

      pthread_mutex_t receptionMutex;

      AEPCommon
      (io::Log* log_, io::Log::Priority logLevel_, bool errorTolerant_);

      virtual ~AEPCommon
      ();

      void setParticipantId
      (Participant& participant, long id)
      {
        participant.id = id;
      }

      long addParticipant
      (Participant& participant);

      void addParticipantWithId
      (Participant& participant, const char* location);

    public:

      Participant& getParticipant
      (long participantId, const char* location);

      Participant& getParticipant
      (long participantId)
      {
        return getParticipant(participantId, "AEPCommon::getParticipant");
      }

    protected:

      void removeParticipant
      (Participant& participant, const char* location);

      void removeAllParticipants
      (bool deleteParticipants = false);

      void sendLock
      ();

      void sendUnlock
      ();

      void wait
      (pthread_cond_t* condition_);

      void wait
      ();

      virtual void send
      (AEPCommon::Token& token) = 0;

      virtual bool deletesSentTokens
      () = 0;

    public:

      void lock
      ();

      void unlock
      ();

      void signal
      ();

      void setLogLevel
      (io::Log::Priority logLevel_)
      {
        lock();
        logLevel = logLevel_;
        unlock();
      }

    protected:

      static const char* modeTypeStrings[];

    public:

      typedef enum { BINARY, XDR, TEXT } Mode;

      static const char* getModeTypeString
      (Mode mode)
      {
        return modeTypeStrings[mode];
      }

      // Parse AEP URLs of the form aep://some.host.name:port/dim1:dim2:...
      // Don't know if this might be useful in AEPServer, one day - it's for
      // AEPClient.
      static void parseURL
      (const char* url, Mode& mode, std::string& host, int& port,
       CompoundDimension& dimension);

      // Construct an AEP URL, with a possibly-NULL dimension.
      static std::string constructURL
      (Mode& mode, std::string& host, int port, CompoundDimension* dimension);

    };


  }


}


#endif // __AEPCOMMON_HPP__
