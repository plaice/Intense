// ****************************************************************************
//
// IntenseAEPTest.hpp : Base for tests of libintense-aep.
//
// Copyright 2004 Paul Swoboda.
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


class IntenseAEPTest
  : public Test {

protected:

  class TestParticipant
    : public intense::aep::AEPClient::Participant {

    std::string name;

    std::ostringstream out;

    intense::Context localContext;

  public:

    TestParticipant
    (char*_name, intense::io::Log* log = NULL,
     intense::io::Log::Priority logLevel = intense::io::Log::DEBUG2)
      : intense::aep::AEPClient::Participant(log, logLevel), name(_name),
        out(), localContext()
    {
    }

    TestParticipant
    (intense::aep::AEPClient& client, const char *_name,
     intense::io::Log* log = NULL,
     intense::io::Log::Priority logLevel = intense::io::Log::DEBUG2)
      : intense::aep::AEPClient::Participant(client, log, logLevel),
        name(_name), out(), localContext()
    {
    }

    void assignNotify
    (const intense::Context& newValue, const intense::CompoundDimension* dim,
     intense::Origin* origin);

    void applyNotify
    (const intense::ContextOp& o, const intense::CompoundDimension* dim,
     intense::Origin* origin);

    void clearNotify
    (const intense::CompoundDimension* dim, intense::Origin* origin);

    void kickNotify
    (intense::Origin* origin);

    void checkContext
    (int testId, const char* expectedCanonical);

    std::string getName
    ()
    {
      return name;
    }

  };


  class ClientServerPair {

  protected:

    std::string name;

    intense::aep::AEPClient* client;

    intense::aep::AEPServer* server;

    intense::io::Log* log;

    std::map<long, TestParticipant*> participantMap;

    ClientServerPair
    (const char* name_, intense::io::Log& log_)
      : name(name_), client(NULL), server(NULL), log(&log_),
        participantMap()
    {}

    void setClient
    (intense::aep::AEPClient& client_)
    {
      client = &client_;
    }

    void setServer
    (intense::aep::AEPServer& server_)
    {
      server = &server_;
    }

  public:

    virtual ~ClientServerPair
    ();

    void disconnect
    ();

    TestParticipant& newParticipant
    (long id);

    void removeParticipant
    (long id);

    TestParticipant& participantRef
    (long id);

    TestParticipant& operator[]
    (long id)
    {
      return participantRef(id);
    }

  };


  class SharedClientServerPair
    : public ClientServerPair {

  protected:

    virtual void cleanupConnections
    ()
    {}

  public:

    SharedClientServerPair
    (const char* name, intense::aep::AEPServer::AEther& aether,
     intense::io::Log& log,
     intense::io::Log::Priority logLevel = intense::io::Log::DEBUG2);

    virtual ~SharedClientServerPair
    ()
    {}

  };


  class StreamClientServerPair
    : public ClientServerPair {

  protected:

    int serverInPipes[2];

    int clientInPipes[2];

    intense::io::FileInStream* serverIn;

    intense::io::FileOutStream* serverOut;

    intense::io::FileInStream* clientIn;

    intense::io::FileOutStream* clientOut;

  public:

    StreamClientServerPair
    (const char* name, intense::io::Log& log);

    virtual ~StreamClientServerPair
    ();

  };


  class BinaryClientServerPair
    : public StreamClientServerPair {

  public:

    BinaryClientServerPair
    (const char* name, intense::aep::AEPServer::AEther& aether,
     intense::io::Log& log,
     intense::io::Log::Priority logLevel = intense::io::Log::DEBUG2);

    virtual ~BinaryClientServerPair
    ()
    {}

  };


  class XDRClientServerPair
    : public StreamClientServerPair {

  public:

    XDRClientServerPair
    (const char* name, intense::aep::AEPServer::AEther& aether,
     intense::io::Log& log,
     intense::io::Log::Priority logLevel = intense::io::Log::DEBUG2);

    virtual ~XDRClientServerPair
    ()
    {}

  };


  class AETPClientServerPair
    : public StreamClientServerPair {

  public:

    AETPClientServerPair
    (const char* name, intense::aep::AEPServer::AEther& aether,
     intense::io::Log& log,
     intense::io::Log::Priority logLevel = intense::io::Log::DEBUG2);

    virtual ~AETPClientServerPair
    ()
    {}

  };


  intense::io::Log* log;

  std::vector<ClientServerPair*> pairs;

public:

  IntenseAEPTest
  (const char* execName)
    : Test(execName), log(NULL), pairs()
  {
  }

  void setLog
  (intense::io::Log& log_)
  {
    log = &log_;
  }

  virtual ~IntenseAEPTest
  ();

  void deletePairs
  ();

protected:

  void aepTest
  ();

  virtual bool permitMultiThreadedTest
  ()
  {
    return false;
  }

  virtual void usage
  ();

};
