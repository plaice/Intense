// ****************************************************************************
//
// AEPServer.java : AEP 2.0 server, for the abstract/core AEP implementation.
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


public abstract class AEPServer
  extends AEPCommon {


  public static class AEther
    extends intense.AEther {

    static final long serialVersionUID = -6653064976724689572L;


    public static class Origin
      implements intense.Origin {

      protected AEther aether;

      protected HashMap<AEPServer, intense.AEther> serverMap;

      protected AEPServer.AsynchronousToken sourceToken;

      protected Origin
      (AEther aether)
      {
        this.aether = aether;
        serverMap = new HashMap<AEPServer, intense.AEther>();
      }

      protected void beginOperation
      (AEPServer.AsynchronousToken sourceToken)
      {
        serverMap.clear();
        this.sourceToken = sourceToken;
      }

      protected void clear
      ()
      {
        serverMap.clear();
      }

      protected void execute
      ()
        throws AEPException
      {
        java.util.Iterator itr = serverMap.keySet().iterator();

        while (itr.hasNext()) {
          AEPServer server = (AEPServer)itr.next();

          AEPClient.NotifyToken token =
            (AEPClient.NotifyToken)(server.getCurrentNotifyToken());

          try {
            Log log = aether.log;
            int logLevel = aether.logLevel;

            if ((log != null)&&(logLevel >= Log.DEBUG)) {
              log.priority(Log.DEBUG).add(
                "AEPServer.AEther.Origin.execute: Sending notification token: "
              );
              token.print(log, (logLevel > Log.DEBUG) ? logLevel : Log.DEBUG);
              log.end();
            }
            server.lockedSend(token);
          } catch (Throwable t) {
            token.shallowClear();
            throw new AEPException(new Exception(t));
          }
        }
      }

      protected void addServerMapEntry
      (AEPServer server, intense.AEther node)
        throws AEPException
      {
        if (!(serverMap.containsKey(server))) {
          serverMap.put(server, node);
          server.setCurrentNotifyToken(
            new AEPClient.NotifyToken(sourceToken.getServerSequence())
          );
        }
      }

      public AEPServer.AsynchronousToken getSourceToken
      ()
        throws AEPException
      {
        if (sourceToken == null) {
          throw new AEPException(
            "INTERNAL ERROR: null source token in " +
            "AEPServer.AEther.Origin.getSourceToken",
            Log.FATAL
          );
        } else {
          return sourceToken;
        }
      }

    }

    protected long serverSequence;

    protected Thread earThread;

    protected boolean running;

    protected ReentrantLock mutex;

    protected Condition condition;

    protected LinkedList<AEPServer.Token> incoming;

    protected Origin origin;

    protected AEPServer.AsynchronousToken accumulated;

    public static final int defaultMaxAccumulatedCount = 1000;

    protected int accumulatedCount;

    protected int maxAccumulatedCount;

    Log log;

    int logLevel;

    private void construct
    (int maxAccumulatedCount, Log log, int logLevel)
    {
      serverSequence = 0;
      earThread = null;
      running = false;
      mutex = new ReentrantLock();
      condition = mutex.newCondition();
      incoming = new LinkedList<AEPServer.Token>();
      origin = new Origin(this);
      this.maxAccumulatedCount = maxAccumulatedCount;
      this.log = log;
      this.logLevel = logLevel;
    }

    public AEther
    ()
    {
      super();
      construct(defaultMaxAccumulatedCount, null, Log.NOTICE);
    }

    public AEther
    (int maxAccumulatedCount)
    {
      super();
      construct(maxAccumulatedCount, null, Log.NOTICE);
    }

    public AEther
    (int maxAccumulatedCount, Log log)
    {
      super();
      construct(maxAccumulatedCount, log, Log.NOTICE);
    }

    public AEther
    (int maxAccumulatedCount, Log log, int logLevel)
    {
      super();
      construct(maxAccumulatedCount, log, logLevel);
    }

    public void append
    (AEPServer.Token token)
    {
      incoming.addLast(token);
    }

    public void signal
    ()
    {
      condition.signal();
    }

    public void condWait
    (Condition waitCondition)
    {
      waitCondition.awaitUninterruptibly();
    }

    public void accumulate
    (AEPServer.AsynchronousToken token)
      throws IOException, IntenseException
    {
      int flags = token.flags;

      if ((flags&AEPServer.preFenceBit) != 0) {
        if ((log != null)&&(logLevel >= Log.DEBUG)) {
          log.priority(Log.DEBUG).add(
            "AEther thread " + Thread.currentThread() +
            ": preFenceBit set in token"
          ).end();
        }
        applyAccumulated();
        accumulated = token;
      } else if (accumulated != null) {
        //    AsynchronousToken* accumulationResult = null;
        AsynchronousToken accumulationResult = accumulated.accumulate(token);

        if (accumulationResult == token) {
          accumulated = accumulationResult;
        } else if (accumulationResult != null) {
          accumulated = accumulationResult;
        } else {
          applyAccumulated();
          accumulated = token;
        }
      } else {
        accumulated = token;
      }
      accumulatedCount++;
      if (((flags&AEPServer.postFenceBit) != 0)||
          (accumulatedCount > maxAccumulatedCount)) {
        if ((log != null)&&(logLevel >= Log.DEBUG)) {
          if ((flags&AEPServer.postFenceBit) != 0) {
            log.priority(Log.DEBUG).add(
              "AEther thread " + Thread.currentThread() +
              ": postFenceBit set in token"
            ).end();
          }
          if (accumulatedCount > maxAccumulatedCount) {
            log.priority(Log.DEBUG).add(
              "AEther thread " + Thread.currentThread() +
              ": maximum accumulated count (" + maxAccumulatedCount +
              ") reached."
            ).end();
          }
        }
        applyAccumulated();
      }
    }

    public void applyAccumulated
    ()
      throws IOException, IntenseException
    {
      if (accumulated != null) {
        if ((log != null)&&(logLevel >= Log.DEBUG)) {
          log.priority(Log.DEBUG).add(
            "AEther thread " + Thread.currentThread() +
            ": applying accumulated asynchronous token: "
          );
          accumulated.print(log, Log.DEBUG);
          log.end();
        }
        accumulated.aetherHandle(this);
        accumulated = null;
        accumulatedCount = 0;
      } else {
        if ((log != null)&&(logLevel >= Log.DEBUG)) {
          log.priority(Log.DEBUG).add(
            "AEther thread " + Thread.currentThread() +
            ": no accumulated asynchronous operations to apply."
          ).end();
        }
      }
    }

    public void lock
    ()
    {
      mutex.lock();
    }

    public Condition newCondition
    ()
    {
      return mutex.newCondition();
    }

    public void unlock
    ()
    {
      mutex.unlock();
    }

    public long nextServerSequence
    ()
    {
      return serverSequence++;
    }

    public void setLogLevel
    (int logLevel)
    {
      lock();
      this.logLevel = logLevel;
      unlock();
    }


    private class EarThread
      extends Thread {

      public void run
      ()
      {
        try {
          main();
        } catch (Throwable ignored) {
        }
      }

    }


    protected void main
    ()
    {
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEther ear thread " + Thread.currentThread() +
          ": starting execution..."
        ).end();
      }
      try {
        lock();
        // Signal constructor:
        signal();
        while (running) {
          if (incoming.size() > 0) {
            AEPServer.Token token = (AEPServer.Token)incoming.removeFirst();
            unlock();
            processToken(token);
            lock();
          } else {
            // Before waiting for more tokens, always apply any outstanding
            // accumulated asynchronous tokens:
            if ((log != null)&&(logLevel >= Log.DEBUG)) {
              log.priority(Log.DEBUG).add(
                "AEther thread: applying outstanding tokens and suspending..."
              ).end();
            }
            applyAccumulated();
            condition.awaitUninterruptibly();
          }
        }
        // Destructor is now waiting to join() this thread.
      } catch (Exception e) {
        if ((log != null)&&(logLevel >= Log.ERROR)) {
          log.priority(Log.ERROR).add(
            "AEther thread caught exception in main: " + e.getMessage() +
            "\nTrace:\n\n" + Log.getTrace(e)
          ).end();
        }
      } catch (Throwable t) {
        if ((log != null)&&(logLevel >= Log.ERROR)) {
          log.priority(Log.ERROR).add(
            "AEther thread caught unknown exception in main" +
            "\nTrace:\n\n" + Log.getTrace(t)
            ).end();
        }
      }
      unlock();
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEther thread " + Thread.currentThread() + ": END OF LINE."
          ).end();
      }
    }

    public void start
    ()
      throws IOException, IntenseException, AEPException
    {
      lock();
      if (running) {
        unlock();
        return;
      }
      running = true;
      earThread = new EarThread();
      try {
        earThread.start();
      } catch (IllegalThreadStateException e) {
        running = false;
        throw new AEPException(
          "Could not create AEPServer.AEther ear thread", e
         );
      }
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPServer.AEther.start created ear thread " + earThread + "."
        ).end();
      }
      unlock();
    }

    public void stop
    ()
      throws InterruptedException
    {
      boolean runningFlag;

      lock();
      runningFlag = running;
      if (running) {
        running = false;
      }
      if (runningFlag) {
        if ((log != null)&&(logLevel >= Log.NOTICE)) {
          log.priority(Log.NOTICE).add(
            "AEPServer.AEther.stop: stop ear thread " + earThread + "..."
          ).end();
        }
      }
      signal();
      unlock();
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPServer.AEther.stop: joining ear thread " + earThread + "..."
        ).end();
      }
      earThread.join();
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPServer.stop: joined ear thread " + earThread + '.'
        ).end();
      }
    }

    protected void processToken
    (AEPServer.Token token)
      throws IOException, IntenseException, AEPException
    {
      long serverSequence = token.getServerSequence();
      long clientSequence = token.getClientSequence();

      if ((log != null)&&(logLevel >= Log.DEBUG)) {
        log.priority(Log.DEBUG).add("AEPServer.AEther: Received token: ");
        token.print(log, (logLevel > Log.DEBUG) ? logLevel : Log.DEBUG);
        log.end();
      }
      try {
        switch (token.getType()) {
        case AEPServer.Token.VOID:
          throw new AEPException(
            "INTERNAL ERROR: Received VOID AEPServer.Token in " +
            "AEPServer.AEther processing of AEPServer.Token",
            Log.FATAL
          );
        case AEPServer.Token.SYNCH:
        case AEPServer.Token.CLIENT_DISCONNECT:
        case AEPServer.Token.JOIN:
        case AEPServer.Token.LEAVE:
        case AEPServer.Token.KICK:
          // Synchronous tokens are implicitly associative pre-fences:
          applyAccumulated();
          token.aetherHandle(this);
          lock();
          ((SynchronousToken)token).resumeServer();
          unlock();
          // AEPServer deletes synchronous tokens:
          break;
        case AEPServer.Token.ASSIGN:
        case AEPServer.Token.APPLY:
        case AEPServer.Token.CLEAR:
          // AEPServer.AEther must decrement the reference count of (and
          // possibly delete) asynchronous tokens, under accumulate somewhere:
          accumulate(((AsynchronousToken)token));
          break;
        default:
          {
            throw new AEPException(
              "INTERNAL ERROR: Got bad AEPServer.Token type int in " +
              "AEPServer.AEther processing of AEPServer.Token: " +
              token.getType(),
              Log.FATAL
            );
          }
        }
      } catch (AEPException e) {
        if ((log != null)&&(logLevel >= e.getPriority())) {
          log.priority(e.getPriority()).add(
            "Caught AEPException in AEPServer.AEther " +
            "processing of AEPServer.Token: " + e.getMessage() +
            "\nTrace:\n\n" + Log.getTrace(e)
          ).end();
        }
      } catch (IntenseException e) {
        if ((log != null)&&(logLevel >= Log.CRITICAL)) {
          log.priority(Log.CRITICAL).add(
            "Caught IntenseException in AEPServer.AEther processing of " +
            "AEPServer.Token: " + e.getMessage() +
            "\nTrace:\n\n" + Log.getTrace(e)
          ).end();
        }
      } catch (Exception e) {
        if ((log != null)&&(logLevel >= Log.CRITICAL)) {
          log.priority(Log.CRITICAL).add(
            "Caught Exception in AEPServer.AEther processing of " +
            "AEPServer.Token: " + e.getMessage() +
            "\nTrace:\n\n" + Log.getTrace(e)
          ).end();
        }
      } catch (Throwable t) {
        if ((log != null)&&(logLevel >= Log.CRITICAL)) {
          log.priority(Log.CRITICAL).add(
            "Caught unknown object in AEPServer.AEther processing of " +
            "AEPServer.Token" +
            "\nTrace:\n\n" + Log.getTrace(t)
          ).end();
        }
      }
    }

  }


  protected static class Participant
    extends intense.AEtherParticipant implements AEPCommon.Participant {

    protected long id;

    protected Log log;

    protected int logLevel;

    protected AEPServer server;

    protected CompoundDimension dimension;

    protected Participant
    (AEPServer server, long id)
    {
      super();
      this.id = id;
      this.log = server.log;
      this.logLevel = Log.NOTICE;
      this.server = server;
      this.dimension = null;
    }

    protected Participant
    (AEPServer server, long id, Log log)
    {
      super();
      this.id = id;
      this.log = log;
      this.logLevel = Log.NOTICE;
      this.server = server;
      this.dimension = null;
    }

    protected Participant
    (AEPServer server, long id, Log log, int logLevel)
    {
      super();
      this.id = id;
      this.log = log;
      this.logLevel = logLevel;
      this.server = server;
      this.dimension = null;
    }

    public void leave
    ()
      throws IntenseException
    {
      super.leave();
    }

    public void join
    (intense.AEther a)
      throws IntenseException
    {
      super.join(a);
    }

    protected AEther.Origin getNotificationOrigin
    (Origin origin, String location)
      throws AEPException
    {
      if (origin == null) {
        throw new AEPException(
          "INTERNAL ERROR: null origin passed as origin in " + location,
          Log.FATAL
        );
      } else {
        try {
          return (AEther.Origin)origin;
        } catch (ClassCastException e) {
          throw new AEPException(
            "INTERNAL ERROR: Non-AEPServer.AEther.Origin passed as " +
            "origin in " + location,
            Log.FATAL
          );
        }
      }
    }

    public void assignNotify
    (Context value, CompoundDimension dim, Origin srcOrigin)
      throws IntenseException
    {
      AEPClient.NotifyToken.AddNodeResult addNodeResult =
        new AEPClient.NotifyToken.AddNodeResult();
      AEther.Origin origin = getNotificationOrigin(
        srcOrigin, "AEPServer.Participant.assignNotify"
      );
      AEPClient.NotifyToken.AssignTarget assignTarget;
      AEPServer.AsynchronousToken sourceToken = origin.getSourceToken();
      intense.AEther aether = (intense.AEther)(this.aether);
      AEPClient.NotifyToken notifyToken;

      if (((sourceToken.flags&AEPServer.preFenceBit) != 0)&&
          ((sourceToken.flags&AEPServer.postFenceBit) != 0)) {
        if (((sourceToken.flags&AEPServer.notifySelfBit) == 0)&&
            (sourceToken.participant == this)) {
          if ((server.log != null)&&(server.logLevel >= Log.DEBUG)) {
            server.log.priority(Log.DEBUG).add(
              "AEPServer.Participant: Skipping assignNotify of " +
              "self (server:" + sourceToken.getServerSequence() + ')'
            ).end();
          }
          return;
        }
        if (((sourceToken.flags&AEPServer.notifyClientBit) == 0)&&
            (sourceToken.server == server)) {
          if ((server.log != null)&&(server.logLevel >= Log.DEBUG)) {
            server.log.priority(Log.DEBUG).add(
              "AEPServer.Participant: Skipping assignNotify of " +
              "client (server:" + sourceToken.getServerSequence() + ')'
            ).end();
          }
          return;
        }
      }
      origin.addServerMapEntry(server, aether);
      notifyToken = (AEPClient.NotifyToken)(server.currentNotifyToken);
      notifyToken.addNode((Context)value, addNodeResult);
      if (dim != null) {
        if (addNodeResult.internalDimension != null) {
          throw new IntenseException(
            "INTERNAL ERROR: Both internal and external dimensions " +
            "are non-null in AEPServer.Participant.assignNotify"
          );
        }
        assignTarget = new AEPClient.NotifyToken.AssignTarget(
          getId(), addNodeResult.nodeIndex, new CompoundDimension(dim), true
        );
      } else if (addNodeResult.internalDimension != null) {
        assignTarget = new AEPClient.NotifyToken.AssignTarget(
          getId(), addNodeResult.nodeIndex,
          addNodeResult.internalDimension, false
        );
      } else {
        assignTarget = new AEPClient.NotifyToken.AssignTarget(
          getId(), addNodeResult.nodeIndex
        );
      }
      notifyToken.addTarget(assignTarget);
    }

    public void applyNotify
    (ContextOp op, CompoundDimension dim, Origin srcOrigin)
      throws IntenseException
    {
      AEPClient.NotifyToken.AddNodeResult addNodeResult =
        new AEPClient.NotifyToken.AddNodeResult();
      AEther.Origin origin = getNotificationOrigin(
        srcOrigin, "AEPServer.Participant.applyNotify"
      );
      AEPClient.NotifyToken.ApplyTarget applyTarget;
      AEPServer.AsynchronousToken sourceToken = origin.getSourceToken();
      intense.AEther aether = this.aether;
      AEPClient.NotifyToken notifyToken;

      if (((sourceToken.flags&AEPServer.preFenceBit) != 0)&&
          ((sourceToken.flags&AEPServer.postFenceBit) != 0)) {
        if (((sourceToken.flags&AEPServer.notifySelfBit) == 0)&&
            (sourceToken.participant == this)) {
          if ((server.log != null)&&(server.logLevel >= Log.DEBUG)) {
            server.log.priority(Log.DEBUG).add(
              "AEPServer.Participant: Skipping applyNotify of " +
              "self (server:" + sourceToken.getServerSequence() + ')'
            ).end();
          }
          return;
        }
        if (((sourceToken.flags&AEPServer.notifyClientBit) == 0)&&
            (sourceToken.server == server)) {
          if ((server.log != null)&&(server.logLevel >= Log.DEBUG)) {
            server.log.priority(Log.DEBUG).add(
              "AEPServer.Participant: Skipping applyNotify of " +
              "client (server:" + sourceToken.getServerSequence() + ')'
            ).end();
          }
          return;
        }
      }
      origin.addServerMapEntry(server, aether);
      notifyToken = (AEPClient.NotifyToken)(server.currentNotifyToken);
      notifyToken.addNode(op, addNodeResult);
      if (dim != null) {
        if (addNodeResult.internalDimension != null) {
          throw new IntenseException(
            "INTERNAL ERROR: Both internal and external dimensions " +
            "are non-null in AEPServer.Participant.applyNotify"
          );
        }
        applyTarget = new AEPClient.NotifyToken.ApplyTarget(
          getId(), addNodeResult.nodeIndex, new CompoundDimension(dim), true
        );
      } else if (addNodeResult.internalDimension != null) {
        applyTarget = new AEPClient.NotifyToken.ApplyTarget(
          getId(), addNodeResult.nodeIndex,
          addNodeResult.internalDimension, false
        );
      } else {
        applyTarget = new AEPClient.NotifyToken.ApplyTarget(
          getId(), addNodeResult.nodeIndex
        );
      }
      notifyToken.addTarget(applyTarget);
    }

    public void clearNotify
    (CompoundDimension dim, Origin srcOrigin)
      throws IntenseException
    {
      AEther.Origin origin = getNotificationOrigin(
        srcOrigin, "AEPServer.Participant.applyNotify"
      );
      AEPClient.NotifyToken.ClearTarget clearTarget;
      AEPServer.AsynchronousToken sourceToken = origin.getSourceToken();
      intense.AEther aether = this.aether;
      AEPClient.NotifyToken notifyToken;

      if (((sourceToken.flags&AEPServer.preFenceBit) != 0)&&
          ((sourceToken.flags&AEPServer.postFenceBit) != 0)) {
        if (((sourceToken.flags&AEPServer.notifySelfBit) == 0)&&
            (sourceToken.participant == this)) {
          if ((server.log != null)&&(server.logLevel >= Log.DEBUG)) {
            server.log.priority(Log.DEBUG).add(
              "AEPServer.Participant: Skipping clearNotify of " +
              "self (server:" + sourceToken.getServerSequence() + ')'
            ).end();
          }
          return;
        }
        if (((sourceToken.flags&AEPServer.notifyClientBit) == 0)&&
            (sourceToken.server == server)) {
          if ((server.log != null)&&(server.logLevel >= Log.DEBUG)) {
            server.log.priority(Log.DEBUG).add(
              "AEPServer.Participant: Skipping clearNotify of " +
              "client (server:" + sourceToken.getServerSequence() + ')'
            ).end();
          }
          return;
        }
      }
      origin.addServerMapEntry(server, aether);
      notifyToken = (AEPClient.NotifyToken)(server.currentNotifyToken);
      if (dim != null) {
        clearTarget = new AEPClient.NotifyToken.ClearTarget(
          getId(), new CompoundDimension(dim)
        );
      } else {
        clearTarget = new AEPClient.NotifyToken.ClearTarget(getId());
      }
      notifyToken.addTarget(clearTarget);
    }

    public void kickNotify
    (Origin origin)
      throws IntenseException
    {
    }

    public boolean isPure
    ()
    {
      return false;
    }

    public long getId
    ()
    {
      return id;
    }    

    public void setId
    (long id)
    {
      this.id = id;
    }    

    public void setLog
    (Log log)
    {
      this.log = log;
    }

    public Log getLog
    ()
    {
      return log;
    }

    public void setLogLevel
    (int newLogLevel)
      throws LogException
    {
      Log.checkPriority(logLevel);
      this.logLevel = logLevel;
    }

    public int getLogLevel
    ()
    {
      return logLevel;
    }

  }


  public abstract static class Token
    extends AEPCommon.Token {

    protected AEPServer server;

    protected long clientSequence;

    protected long serverSequence;

    public static final int VOID = 0;

    public static final int SYNCH = 1;

    public static final int CLIENT_DISCONNECT = 2;

    public static final int JOIN = 3;

    public static final int LEAVE = 4;

    public static final int ASSIGN = 5;

    public static final int APPLY = 6;

    public static final int CLEAR = 7;

    public static final int KICK = 8;

    public static String typeStrings[] = {
      "VOID", "SYNCH", "CLIENT_DISCONNECT", "JOIN", "LEAVE",
      "ASSIGN", "APPLY", "CLEAR", "KICK"
    };

    public static void checkType
    (int type)
      throws AEPException
    {
      if ((type < VOID)||(type > KICK)) {
        throw new AEPException("Invalid AEPServer.Token type " + type);
      }
    }

    String getTypeString
    ()
    {
      return typeStrings[getType()];
    }

    static String getTypeString
    (int type)
    {
      return typeStrings[type];
    }

    Token
    (long clientSequence)
    {
      server = null;
      this.clientSequence = clientSequence;
      this.serverSequence = -1;
    }

    protected static Token factory
    (int type)
      throws AEPException
    {
      switch (type) {
      case VOID:
        return null;
      case SYNCH:
        return new SynchToken();
      case CLIENT_DISCONNECT:
        return new ClientDisconnectToken();
      case JOIN:
        return new JoinToken();
      case LEAVE:
        return new LeaveToken();
      case ASSIGN:
        return new AssignToken();
      case APPLY:
        return new ApplyToken();
      case CLEAR:
        return new ClearToken();
      default:
        throw new AEPException("Bad AEPServer.Token type " + type, Log.FATAL);
      }
    }

    public abstract void aetherHandle
    (AEther aether)
      throws IntenseException, AEPException;

    public static void factorySerialise
    (Token token, DataOutputStream out)
      throws IOException, AEPException
    {
      int type;

      if (token != null) {
        type = ((AEPServer.Token)token).getType();
      } else {
        type = AEPServer.Token.VOID;
      }
      out.writeInt(type);
      if (token != null) {
        ((AEPServer.Token)token).serialise(out);
      }
    }

    public static Token factoryDeserialise
    (DataInputStream in)
      throws IOException, IntenseException, AEPException
    {
      int type;
      AEPServer.Token token;

      type = in.readInt();
      token = AEPServer.Token.factory(type);
      if (token != null) {
	token.deserialise(in);
      }
      return token;
    }

    public void setServer
    (AEPServer server)
    {
      this.server = server;
    }

    public long getClientSequence
    ()
    {
      return clientSequence;
    }

    public long getServerSequence
    ()
    {
      return serverSequence;
    }

  }


  public abstract static class SynchronousToken
    extends Token {

    protected AEPClient.Token reply;

    protected SynchronousToken
    ()
    {
      super(-1);
      reply = null;
    }

    protected SynchronousToken
    (long clientSequence)
    {
      super(clientSequence);
      reply = null;
    }

    protected void resumeServer
    ()
    {
      server.aetherCondition.signal();
    }

    public boolean aetherTransaction
    (String location)
      throws AEPException
    {
      String errorMessage = null;
      boolean successful = false;

      server.aether.lock();
      serverSequence = server.aether.nextServerSequence();
      server.aether.append(this);
      server.aether.signal();
      server.aether.condWait(server.aetherCondition);
      server.aether.unlock();
      if (reply == null) {
        return true;
      } else if (reply.getType() == AEPClient.Token.ACK) {
        successful = true;
      } else if ((reply.getType() != AEPClient.Token.DENY)&&
                 (reply.getType() != AEPClient.Token.ERROR)) {
        reply = null;
        errorMessage = "INTERNAL ERROR: Non-AEPClient.ReplyToken reply in " +
          location;
      }
      if (errorMessage == null) {
        server.lockedAndLoggedSend(
          reply, location, serverSequence, clientSequence
        );
        reply = null;
      } else {
        AEPClient.ErrorToken errorToken = new AEPClient.ErrorToken(
          serverSequence, clientSequence, errorMessage
        );
        if ((server.log != null)&&(server.logLevel >= Log.ERROR)) {
          server.log.priority(Log.ERROR).add(
            errorMessage + " (ss: " + serverSequence +
            ", cs: " + clientSequence + ")"
          ).end();
        }
        try {
          server.lockedAndLoggedSend(
            errorToken, location, serverSequence, clientSequence
          );
        } catch (Throwable ignored) {
        }
        throw new AEPException(errorMessage, Log.FATAL);
      }
      return successful;
    }
  }


  public static class SynchToken
    extends SynchronousToken {

    public SynchToken
    ()
    {
      super(-1);
    }

    public SynchToken
    (long clientSequence)
    {
      super(clientSequence);
    }

    public int getType
    ()
    {
      return SYNCH;
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      os.writeLong(clientSequence);
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, AEPException
    {
      clientSequence = is.readLong();
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      String location = "SynchToken.handle";

      aetherTransaction(location);
      return true;
    }

    public void aetherHandle
    (AEther aether)
      throws IntenseException, AEPException
    {
      reply = new AEPClient.AckToken(serverSequence, clientSequence);
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print("SYNCH " + clientSequence + ";\n\n");
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print("AEPServer.SynchToken (cs: " + clientSequence + ")");
    }

  }


  public static class ClientDisconnectToken
    extends SynchronousToken {

    public ClientDisconnectToken
    ()
    {
      super(-1);
    }

    public ClientDisconnectToken
    (long clientSequence)
    {
      super(clientSequence);
    }

    public int getType
    ()
    {
      return CLIENT_DISCONNECT;
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      os.writeLong(clientSequence);
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, AEPException
    {
      clientSequence = is.readLong();
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      String location = "ClientDisconnectToken.handle";

      // So the destructor doesn't try to cancel us, and just joins us:
      server.lock();
      server.running = false;
      server.unlock();
      aetherTransaction(location);
      return false;
    }

    public void aetherHandle
    (AEther aether)
      throws IntenseException, AEPException
    {
      server.leaveAllParticipants();
      if (clientSequence >= 0) {
        reply = new AEPClient.AckToken(serverSequence, clientSequence);
      } else {
        reply = null;
      }
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print("DISCONNECT " + clientSequence + ";\n\n");
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print("AEPServer.ClientDisconnectToken (cs: " + clientSequence + ")");
    }

  }


  public static class JoinToken
    extends SynchronousToken {

    protected long participantId;

    protected CompoundDimension dimension;

    protected boolean notify;

    protected Participant participant;

    public JoinToken
    ()
    {
      super(-1);
      participantId = -1;
      dimension = null;
      participant = null;
      notify = true;
    }

    public JoinToken
    (long clientSequence, long participantId, boolean notify,
     CompoundDimension dimension)
    {
      super(clientSequence);
      this.participantId = participantId;
      if (dimension != null) {
        this.dimension = new CompoundDimension(dimension);
      } else {
        this.dimension = null;
      }
      this.notify = notify;
    }

    public int  getType
    ()
    {
      return JOIN;
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      os.writeLong(clientSequence);
      os.writeLong(participantId);
      if (!notify) {
        os.writeInt(0);
      } else {
        os.writeInt(1);
      }
      if (dimension == null) {
        os.writeInt(0);
      } else {
        os.writeInt(1);
        dimension.serialise(os);
      }
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, IntenseException, AEPException
    {
      int hasDimension;
      int notifyInt;

      clientSequence = is.readLong();
      participantId = is.readLong();
      notifyInt = is.readInt();
      hasDimension = is.readInt();
      notify = (notifyInt == 1);
      if (hasDimension == 0) {
        dimension = null;
      } else {
        dimension = new CompoundDimension();
        dimension.deserialise(is);
      }
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      String location = "AEPServer.JoinToken.handle";

      // META
      server.log.priority(Log.DEBUG2).add("In JoinToken.handle()").end();
      participant = new Participant(server, participantId);
      try {
        server.addParticipantWithId(participant, location);
      } catch (AEPException e) {
        AEPClient.ErrorToken errorToken = new AEPClient.ErrorToken(
          -1, clientSequence, e.getMessage()
        );
        // Ignore send errors (they are logged):
        try {
          if (server.log != null) {
            server.log.priority(Log.DEBUG2).add("SENDING AN ERROR TOKEN: ");
            errorToken.print(server.log, Log.DEBUG2);
            server.log.end();
          }
          server.lockedAndLoggedSend(errorToken, location, -1, clientSequence);
        } catch (Throwable ignored) {
        }
        throw e;
      }
      try {
        if (!aetherTransaction(location)) {
          server.removeParticipant(participant, location);
          participant = null;
        }
      } catch (Throwable t) {
        server.removeParticipant(participant, location);
        participant = null;
      }
      return true;
    }

    public void aetherHandle
    (AEther aether)
      throws IntenseException, AEPException
    {
      intense.AEther targetNode = null;

      if (dimension == null) {
        targetNode = aether;
        participant.join(targetNode);
      } else {
        targetNode = (intense.AEther)aether.value(dimension);
        participant.join(targetNode);
        participant.dimension = dimension;
        dimension = null;
      }
      if (notify) {
        AEPClient.NotifyToken notifyToken =
          new AEPClient.NotifyToken(serverSequence);
        Context value = new Context();

        value.assign(targetNode);
        notifyToken.addNode(value);
        notifyToken.addTarget(new AEPClient.NotifyToken.AssignTarget(
          participantId, 0
        ));
        server.lockedAndLoggedSend(
          notifyToken, "JoinToken.aetherHandle notification",
          serverSequence, clientSequence
        );
      }
      reply = new AEPClient.AckToken(serverSequence, clientSequence);
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print("JOIN " + clientSequence + ' ' + participantId);
      if (notify) {
        os.print(" NOTIFY");
      }
      if (dimension != null) {
        os.print(" DIM " + dimension.canonical());
      }
      os.print(";\n\n");
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print(
        "AEPServer.JoinToken (cs: " + clientSequence + ", p:" +
        participantId
      );
      if (dimension != null) {
        os.print(", d:" + dimension.canonical());
      }
      os.print(')');
    }

  }


  public static class LeaveToken
    extends SynchronousToken {

    protected long participantId;

    protected Participant participant;

    public LeaveToken
    ()
    {
      super(-1);
      participantId = -1;
      participant = null;
    }

    public LeaveToken
    (long clientSequence, long participantId)
    {
      super(clientSequence);
      this.participantId = participantId;
      this.participant = null;
    }

    public int getType
    ()
    {
      return LEAVE;
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      os.writeLong(clientSequence);
      os.writeLong(participantId);
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, AEPException
    {
      clientSequence = is.readLong();
      participantId = is.readLong();
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      AEPClient.AckToken replyToken;
      String location = "AEPServer.LeaveToken.handle";

      try {
        participant =
          (Participant)server.getParticipant(participantId, location);
      } catch (AEPException e) {
        AEPClient.ErrorToken errorToken = new AEPClient.ErrorToken(
          -1, clientSequence, e.getMessage()
        );
        // Ignore send errors (they are logged):
        try {
          server.lockedAndLoggedSend(errorToken, location, -1, clientSequence);
        } catch (Throwable ignored) {
        }
        throw e;
      }
      if (aetherTransaction(location)) {
        server.removeParticipant(participant, location);
      }
      return true;
    }

    public void aetherHandle
    (AEther aether)
      throws IntenseException, AEPException
    {
      participant.leave();
      reply = new AEPClient.AckToken(serverSequence, clientSequence);
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print("LEAVE " + clientSequence + ' ' + participantId + ";\n\n");
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print(
        "AEPServer.LeaveToken (cs: " + clientSequence + ", p:" +
        participantId + ')'
      );
    }

  };


  /**
   *  For now, we do not allow participant-originated (IRC-like) kicking.  This
   *  is an internal token for use by AEPServer.AEther.  Eventually, when
   *  clusterable participant identifiers (probably just
   *  server-id/participant-id pairs) get worked in, this will not be an issue:
   */
  public static class KickToken
    extends SynchronousToken {

    protected long participantId;

    public KickToken
    ()
    {
      super(-1);
      participantId = -1;
    }

    public KickToken
    (long clientSequence, long participantId)
    {
      super(clientSequence);
      this.participantId = participantId;
    }

    public int getType
    ()
    {
      return KICK;
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      throw new AEPException(
        "INTERNAL ERROR: KickToken.serialise called", Log.FATAL
      );
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, AEPException
    {
      throw new AEPException(
        "INTERNAL ERROR: KickToken.deserialise called", Log.FATAL
      );
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      throw new AEPException(
        "INTERNAL ERROR: KickToken.handle called", Log.FATAL
      );
    }

    public void aetherHandle
    (AEther aether)
      throws IntenseException, AEPException
    {
      throw new AEPException(
        "INTERNAL ERROR: KickToken.aetherHandle called", Log.FATAL
      );
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      throw new AEPException(
        "INTERNAL ERROR: KickToken.aetpPrint called", Log.FATAL
      );
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      throw new IOException("INTERNAL ERROR: KickToken.print called");
    }

  }


  // AsynchronousToken flag bits:

  /**
   * If set, this bit indicates that the token cannot be applied to previous
   * outstanding asynchronous tokens, prior to application to the aether;
   * rather, they will be applied to the aether first:
   */
  public static final int preFenceBit = 0x01;

  /**
   * If set, this token will be applied to the aether as soon as it has been
   * accumulated, and no subsequence asynchronous operations will be applied to
   * it.
   */
  public static final int postFenceBit = 0x02;

  /**
   * Notify the originating participant, during application of this operation
   * (only valid if both preFenceBit and postFenceBit are set).
   */
  public static final int notifySelfBit = 0x04;

  /**
   * Notify participants in the same client as the the originating participant,
   * during application of this operation (only valid if both preFenceBit and
   * postFenceBit are set):
   */
  public static final int notifyClientBit = 0x08;


  public abstract static class AsynchronousToken
    extends Token {

    protected ReentrantLock mutex;

    protected int flags;

    protected CompoundDimension dimension;

    protected Participant participant;

    protected intense.AEther targetNode;

    protected AsynchronousToken
    ()
    {
      super(-1);
      mutex = new ReentrantLock();
      dimension = null;
      participant = null;
      targetNode = null;
      flags = notifySelfBit&notifyClientBit;
    }

    protected AsynchronousToken
    (CompoundDimension dimension, long clientSequence, int flags)
    {
      super(clientSequence);
      mutex = new ReentrantLock();
      if (dimension != null) {
        this.dimension = new CompoundDimension(dimension);
      } else {
        this.dimension = null;
      }
      participant = null;
      targetNode = null;
      this.flags = flags;
    }

    public void lock
    ()
    {
      mutex.lock();
    }

    public void unlock
    ()
    {
      mutex.lock();
    }

    public void aetherTransaction
    (String location)
      throws IOException, IntenseException, AEPException
    {
      server.aether.lock();
      serverSequence = server.aether.nextServerSequence();
      server.sendLock();
      server.aether.append(this);
      server.aether.signal();
      server.aether.unlock();
      // server sendLock still held
      try {
        AEPClient.AckToken ackToken = new AEPClient.AckToken(
          serverSequence, clientSequence
        );

        server.send(ackToken);
      } catch (Exception e) {
        if ((server.log != null)&&(server.logLevel >= Log.ERROR)) {
          server.log.priority(Log.ERROR).add(
            "AEPServer ear thread caught exception during sending of ack " +
            "(ss: " + serverSequence + ", cs: " + clientSequence +
            "): " + e.getMessage()
          ).end();
        }
        throw new AEPException(
          "Caught exception in AEPServer.AsynchronousToken.aetherTransaction",
          e
        );
      } finally {
        server.sendUnlock();
      }
    }

    intense.AEther getTargetNode
    ()
      throws IntenseException
    {
      if (targetNode == null) {
        if (participant != null) {
          if (dimension != null) {
            targetNode =
              (intense.AEther)participant.getNode().value(dimension);
          } else {
            targetNode = participant.getNode();
          }
        } else if (server != null) {
          if (dimension != null) {
            targetNode = (AEther)server.aether.value(dimension);
          } else {
            targetNode = server.aether;
          }
        } else {
          throw new AEPException(
            "Got an asynch token with a null participant and server: " + this,
            Log.FATAL
          );
        }
      }
      return targetNode;
    }

    public void setTargetNode
    (AEther aether)
    {
      targetNode = aether;
    }

    // Returns a pointer to the result of accumulation.  If null,
    // accumulation was not possible (and this token must be applied prior
    // to the argument token).  If the return value is this, then the
    // argument token was accumulated into this token and must be deleted.
    // If the return value is &token, then the argument token was used for
    // the result of accumulation, and this token must be deleted.  If
    // the return value is the address of another AsynchronousToken, then
    // both this token and the argument must be deleted, and the result of
    // accumulation is in the returned token.
    public abstract AsynchronousToken accumulate
    (AsynchronousToken token)
      throws IntenseException;

  }

  public static class AssignToken
    extends AsynchronousToken {

    protected Context context;
    
    protected long participantId;

    public AssignToken
    ()
    {
      super();
      participantId = -1;
      context = null;
    }

    public AssignToken
    (long clientSequence, long participantId, Context context,
     CompoundDimension dimension)
      throws IntenseException
    {
      super(dimension, clientSequence, 0);
      this.participantId = participantId;
      this.context = new Context(context);
    }

    public AssignToken
    (long clientSequence, long participantId, Context context,
     CompoundDimension dimension, int flags)
      throws IntenseException
    {
      super(dimension, clientSequence, flags);
      this.participantId = participantId;
      this.context = new Context(context);
    }

    public int getType
    ()
    {
      return ASSIGN;
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      os.writeLong(clientSequence);
      os.writeLong(participantId);
      context.serialise(os);
      if (dimension == null) {
        os.writeInt(0);
      } else {
        os.writeInt(1);
        dimension.serialise(os);
      }
      os.writeInt(flags);
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, IntenseException, AEPException
    {
      int hasDimension;

      clientSequence = is.readLong();
      participantId = is.readLong();
      context = new Context();
      context.deserialise(is);
      hasDimension = is.readInt();
      if (hasDimension == 0) {
        dimension = null;
      } else {
        dimension = new CompoundDimension();
        dimension.deserialise(is);
      }
      flags = is.readInt();
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      String location = "AssignToken.handle";

      if (participantId > 0) {
        try {
          participant =
            (Participant)server.getParticipant(participantId, location);
        } catch (AEPException e) {
          AEPClient.ErrorToken errorToken = new AEPClient.ErrorToken(
            -1, clientSequence, e.getMessage()
          );

          // Ignore send errors (they are logged):
          try {
            server.lockedAndLoggedSend(
              errorToken, location, -1, clientSequence
            );
          } catch (Throwable ignored) {
          }
          throw e;
        }
      }
      aetherTransaction(location);
      return true;
    }

    public void aetherHandle
    (AEther aether)
      throws IntenseException, AEPException
    {
      aether.origin.beginOperation(this);
      getTargetNode().assign(context, aether.origin);
      aether.origin.execute();
    }

    public AsynchronousToken accumulate
    (AsynchronousToken token)
      throws IntenseException
    {
      CompoundDimension path = new CompoundDimension();
      int ancestry = getTargetNode().getAncestry(token.getTargetNode(), path);

      switch (token.getType()) {
      case ASSIGN:
        {
          AssignToken assignToken = (AssignToken)token;

          if (ancestry <= 0) {
            // [path:<c1>]<c2>:
            // <c1><c2>:
            return token;
          } else {
            // <c1>[path:<c2>]:
            context.value(path).assign(assignToken.context);
            serverSequence = assignToken.serverSequence;
            return this;
          }
        }
      case APPLY:
        {
          ApplyToken applyToken = (ApplyToken)token;

          if (ancestry == 0) {
            // <c1>[o2]:
            context.apply(applyToken.op);
            serverSequence = applyToken.serverSequence;
            return this;
          } else if (ancestry < 0) {
            // [path:<c1>][o2]:
            ApplyToken returnValue = new ApplyToken();

            returnValue.op = new ContextOp();
            ((ContextOp)returnValue.op.value(path)).apply(context);
            returnValue.op.apply(applyToken.op);
            returnValue.targetNode = applyToken.targetNode;
            returnValue.serverSequence = applyToken.serverSequence;
            return returnValue;
          } else {
            // <c1>[path:[o2]]:
            context.value(path).apply(applyToken.op);
            serverSequence = applyToken.serverSequence;
            return this;
          }
        }
      case CLEAR:
        {
          ClearToken clearToken = (ClearToken)token;

          if (ancestry <= 0) {
            // <c1><>:
            // [path:<c1>]<>:
            return token;
          } else {
            // <c1>[path:<>]:
            context.value(path).clear();
            return this;
          }
        }
      default:
        {
          throw new AEPException(
            "Attempt to accumulate token of type " + token.getTypeString() +
            " into AssignToken: " + token,
            Log.FATAL
          );
        }
      }
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print("ASSIGN " + clientSequence + ' ' + participantId);
      if (dimension != null) {
        os.print(" DIM " + dimension.canonical());
      }
      if ((preFenceBit&flags) != 0) {
        os.print(" PREFENCE");
      }
      if ((postFenceBit&flags) != 0) {
        os.print(" POSTFENCE");
      }
      if ((notifySelfBit&flags) != 0) {
        os.print(" SELF");
      }
      if ((notifyClientBit&flags) != 0) {
        os.print(" CLIENT");
      }
      os.print("\nCONTEXT\n" + context + ";\n\n");
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print(
        "AEPServer.AssignToken (cs: " + clientSequence + ", p:" + participantId
      );
      if (dimension != null) {
        os.print(", d:" + dimension.canonical());
      }
      if ((preFenceBit&flags) != 0) {
        os.print(", preFence");
      }
      if ((postFenceBit&flags) != 0) {
        os.print(", postFence");
      }
      if ((notifySelfBit&flags) != 0) {
        os.print(", self-notify");
      }
      if ((notifyClientBit&flags) != 0) {
        os.print(", client-notify");
      }
      if (logLevel >= Log.DEBUG1) {
        os.print(", context:" + ((AssignToken)this).context);
      }
      os.print(')');
    }

  }


  public static class ApplyToken
    extends AsynchronousToken {

    protected long participantId;

    protected ContextOp op;

    public ApplyToken
    ()
    {
      super();
      participantId = -1;
      op = null;
    }

    public ApplyToken
    (long clientSequence, long participantId, ContextOp op,
     CompoundDimension dimension)
      throws IntenseException
    {
      super(dimension, clientSequence, 0);
      this.participantId = participantId;
      this.op = new ContextOp(op);
    }

    public ApplyToken
    (long clientSequence, long participantId, ContextOp op,
     CompoundDimension dimension, int flags)
      throws IntenseException
    {
      super(dimension, clientSequence, flags);
      this.participantId = participantId;
      this.op = new ContextOp(op);
    }

    public int getType
    ()
    {
      return APPLY;
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      os.writeLong(clientSequence);
      os.writeLong(participantId);
      op.serialise(os);
      if (dimension == null) {
        os.writeInt(0);
      } else {
        os.writeInt(1);
        dimension.serialise(os);
      }
      os.writeInt(flags);
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, IntenseException, AEPException
    {
      int hasDimension;

      clientSequence = is.readLong();
      participantId = is.readLong();
      op = new ContextOp();
      op.deserialise(is);
      hasDimension = is.readInt();
      if (hasDimension == 0) {
        dimension = null;
      } else {
        dimension = new CompoundDimension();
        dimension.deserialise(is);
      }
      flags = is.readInt();
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      String location = "ApplyToken.handle";

      if (participantId > 0) {
        try {
          participant =
            (Participant)server.getParticipant(participantId, location);
        } catch (AEPException e) {
          AEPClient.ErrorToken errorToken = new AEPClient.ErrorToken(
            -1, clientSequence, e.getMessage()
          );

          // Ignore send errors (they are logged):
          try {
            server.lockedAndLoggedSend(
              errorToken, location, -1, clientSequence
            );
          } catch (Throwable ignored) {
          }
          throw e;
        }
      }
      aetherTransaction(location);
      return true;
    }

    public void aetherHandle
    (AEther aether)
      throws IntenseException, AEPException
    {
      aether.origin.beginOperation(this);
      getTargetNode().apply(op, aether.origin);
      aether.origin.execute();
    }

    public AsynchronousToken accumulate
    (AsynchronousToken token)
      throws IntenseException
    {
      CompoundDimension path = new CompoundDimension();
      int ancestry = getTargetNode().getAncestry(token.getTargetNode(), path);

      switch (token.getType()) {
      case ASSIGN:
        {
          AssignToken assignToken = (AssignToken)token;

          if (ancestry <= 0) {
            // [path:[o1]]<c2>:
            // [o1]<c2>:
            return token;
          } else {
            // [o1][path:<c2>]:
            ((ContextOp)op.value(path)).apply(assignToken.context);
            serverSequence = assignToken.serverSequence;
            return this;
          }
        }
      case APPLY:
        {
          ApplyToken applyToken = (ApplyToken)token;

          if (ancestry == 0) {
            // [o1][o2]:
            op.apply(applyToken.op);
            serverSequence = applyToken.serverSequence;
            return this;
          } else if (ancestry < 0) {
            // [path:[o1]][o2]:
            ApplyToken returnValue = new ApplyToken();

            returnValue.op = new ContextOp();
            ((ContextOp)returnValue.op.value(path)).apply(op);
            returnValue.op.apply(applyToken.op);
            returnValue.targetNode = applyToken.targetNode;
            returnValue.serverSequence = applyToken.serverSequence;
            return returnValue;
          } else {
            // [o1][path:[o2]]:
            ((ContextOp)op.value(path)).apply(applyToken.op);
            serverSequence = applyToken.serverSequence;
            return this;
          }
        }
      case CLEAR:
        {
          ClearToken clearToken = (ClearToken)token;

          if (ancestry <= 0) {
            // [o1]<>:
            // [path:[o1]]<>:
            return token;
          } else {
            // [o1][path:<>]:
            ((ContextOp)op.value(path)).clear();
            ((ContextOp)op.value(path)).setClearBaseFlag();
            ((ContextOp)op.value(path)).setClearDimsFlag();
            return this;
          }
        }
      default:
        {
          throw new AEPException(
            "Attempt to accumulate token of type " + token.getTypeString() +
            " into ApplyToken: " + token,
            Log.FATAL
          );
        }
      }
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print("APPLY " + clientSequence + ' ' + participantId);
      if (dimension != null) {
        os.print(" DIM " + dimension.canonical());
      }
      if ((preFenceBit&flags) != 0) {
        os.print(" PREFENCE");
      }
      if ((postFenceBit&flags) != 0) {
        os.print(" POSTFENCE");
      }
      if ((notifySelfBit&flags) != 0) {
        os.print(" SELF");
      }
      if ((notifyClientBit&flags) != 0) {
        os.print(" CLIENT");
      }
      os.print("\nOP\n" + op + ";\n\n");
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print(
        "AEPServer.ApplyToken (cs: " + clientSequence + ", p:" +
        participantId
      );
      if (dimension != null) {
        os.print(", d:" + dimension.canonical());
      }
      if ((preFenceBit&flags) != 0) {
        os.print(", prefence");
      }
      if ((postFenceBit&flags) != 0) {
        os.print(", postfence");
      }
      if ((notifySelfBit&flags) != 0) {
        os.print(", self-notify");
      }
      if ((notifyClientBit&flags) != 0) {
        os.print(", client-notify");
      }
      if (logLevel >= Log.DEBUG1) {
        os.print(", op:" + ((ApplyToken)this).op.canonical());
      }
      os.print(')');
    }

  }


  public static class ClearToken
    extends AsynchronousToken {

    protected long participantId;

    public ClearToken
    ()
    {
      super();
      participantId = -1;
    }

    public ClearToken
    (long clientSequence, long participantId, CompoundDimension dimension)
    {
      super(dimension, clientSequence, 0);
      this.participantId = participantId;
    }

    public ClearToken
    (long clientSequence, long participantId, CompoundDimension dimension,
     int flags)
    {
      super(dimension, clientSequence, flags);
      this.participantId = participantId;
    }

    public int getType
    ()
    {
      return CLEAR;
    }

    public void serialise
    (DataOutputStream os)
      throws IOException, AEPException
    {
      os.writeLong(clientSequence);
      os.writeLong(participantId);
      if (dimension == null) {
        os.writeInt(0);
      } else {
        os.writeInt(1);
        dimension.serialise(os);
      }
      os.writeInt(flags);
    }

    public void deserialise
    (DataInputStream is)
      throws IOException, IntenseException, AEPException
    {
      int hasDimension;

      clientSequence = is.readLong();
      participantId = is.readLong();
      hasDimension = is.readInt();
      if (hasDimension == 0) {
        dimension = null;
      } else {
        dimension = new CompoundDimension();
        dimension.deserialise(is);
      }
      flags = is.readInt();
    }

    public boolean handle
    ()
      throws IOException, IntenseException, AEPException
    {
      String location = "ClearToken.handle";

      if (participantId > 0) {
        try {
          participant =
            (Participant)server.getParticipant(participantId, location);
        } catch (AEPException e) {
          AEPClient.ErrorToken errorToken = new AEPClient.ErrorToken(
            -1, clientSequence, e.getMessage()
          );

          // Ignore send errors (they are logged):
          try {
            server.lockedAndLoggedSend(
              errorToken, location, -1, clientSequence
            );
          } catch (Throwable ignored) {
          }
          throw e;
        }
      }
      aetherTransaction(location);
      return true;
    }

    public void aetherHandle
    (AEther aether)
      throws IntenseException, AEPException
    {
      aether.origin.beginOperation(this);
      getTargetNode().clear(aether.origin);
      aether.origin.execute();
    }

    public AsynchronousToken accumulate
    (AsynchronousToken token)
      throws IntenseException
    {
      CompoundDimension path = new CompoundDimension();
      int ancestry = getTargetNode().getAncestry(token.getTargetNode(), path);

      switch (token.getType()) {
      case ASSIGN:
        {
          AssignToken assignToken = (AssignToken)token;

          if (ancestry <= 0) {
            // [path:<>]<c2>:
            // <><c2>:
            return token;
          } else {
            // <>[path:<c2>]:
            AssignToken returnValue = new AssignToken();

            returnValue.context = new Context();
            returnValue.context.value(path).assign(assignToken.context);
            returnValue.targetNode = assignToken.targetNode;
            returnValue.serverSequence = assignToken.serverSequence;
            return returnValue;
          }
        }
      case APPLY:
        {
          ApplyToken applyToken = (ApplyToken)token;

          if (ancestry == 0) {
            // <>[o2]:
            ApplyToken returnValue = new ApplyToken();

            returnValue.op = new ContextOp();
            returnValue.op.setClearBaseFlag();
            returnValue.op.setClearDimsFlag();
            returnValue.op.apply(applyToken.op);
            returnValue.targetNode = applyToken.targetNode;
            returnValue.serverSequence = applyToken.serverSequence;
            return returnValue;
          } else if (ancestry < 0) {
            // [path:<>][o2]:
            ApplyToken returnValue = new ApplyToken();

            returnValue.op = new ContextOp();
            ((ContextOp)returnValue.op.value(path)).setClearBaseFlag();
            ((ContextOp)returnValue.op.value(path)).setClearDimsFlag();
            returnValue.op.apply(applyToken.op);
            returnValue.targetNode = applyToken.targetNode;
            returnValue.serverSequence = applyToken.serverSequence;
            return returnValue;
          } else {
            // <>[path:[o2]]:
            ApplyToken returnValue = new ApplyToken();

            returnValue.op = new ContextOp();
            returnValue.op.setClearBaseFlag();
            returnValue.op.setClearDimsFlag();
            returnValue.op.apply(applyToken.op);
            returnValue.targetNode = applyToken.targetNode;
            returnValue.serverSequence = applyToken.serverSequence;
            return returnValue;
          }
        }
      case CLEAR:
        {
          ClearToken clearToken = (ClearToken)token;

          if (ancestry <= 0) {
            // <><>:
            // [path:<>]<>:
            return token;
          } else {
            // <>[path:<>]:
            return this;
          }
        }
      default:
        {
          throw new AEPException(
            "Attempt to accumulate token of type " + token.getTypeString() +
            " into ClearToken: " + token,
            Log.FATAL
          );
        }
      }
    }

    public void aetpPrint
    (PrintStream os)
      throws IOException, AEPException
    {
      os.print("CLEAR " + clientSequence + ' ' + participantId);
      if (dimension != null) {
        os.print(" DIM " + dimension.canonical());
      }
      if ((preFenceBit&flags) != 0) {
        os.print(" PREFENCE");
      }
      if ((postFenceBit&flags) != 0) {
        os.print(" POSTFENCE");
      }
      if ((notifySelfBit&flags) != 0) {
        os.print(" SELF");
      }
      if ((notifyClientBit&flags) != 0) {
        os.print(" CLIENT");
      }
      os.print(";\n\n");
    }

    public void print
    (PrintStream os, int logLevel)
      throws IOException
    {
      os.print(
        "AEPServer.ClearToken (cs: " + clientSequence + ", p:" +
        participantId
      );
      if (dimension != null) {
        os.print(", d:" + dimension.canonical());
      }
      if ((preFenceBit&flags) != 0) {
        os.print(", preFence");
      }
      if ((postFenceBit&flags) != 0) {
        os.print(", postFence");
      }
      if ((notifySelfBit&flags) != 0) {
        os.print(", self-notify");
      }
      if ((notifyClientBit&flags) != 0) {
        os.print(", client-notify");
      }
      os.print(')');
    }

  }


  protected AEther aether;

  protected Thread earThread;

  protected Condition aetherCondition;

  protected boolean running;

  protected AEPCommon.Token currentNotifyToken;

  protected AEPCommon.Token getCurrentNotifyToken
  ()
    throws AEPException
  {
    if (currentNotifyToken == null) {
      throw new AEPException(
        "INTERNAL ERROR: NULL token in AEPServer.getCurrentNotificationToken",
        Log.FATAL
      );
    }
    return currentNotifyToken;
  }

  protected void setCurrentNotifyToken
  (AEPCommon.Token newToken)
  {
    currentNotifyToken = newToken;
  }

  protected abstract void send
  (AEPClient.Token token)
    throws IOException, AEPException;

  protected abstract Token receive
  ()
    throws IOException, IntenseException, AEPException;

  protected AEPServer
  (AEther aether, Log log, int logLevel, boolean errorTolerant)
  {
    super(log, logLevel, errorTolerant);
    this.aether = aether;
    this.aetherCondition = aether.newCondition();
  }


  protected class EarThread
    extends Thread {

    public void run
    ()
    {
      try {
        earMain();
      } catch (Throwable ignored) {
      }
    }

  }


  protected void earMain
  ()
  {
    boolean runningFlag;

    try {
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPServer ear thread " + Thread.currentThread() +
          ": starting execution..."
        ).end();
      }
      try {
        aep();
      } catch (Exception e) {
        if ((log != null)&&(logLevel >= Log.ERROR)) {
          log.priority(Log.ERROR).add(
            "AEPServer ear thread caught exception in earMain: " +
            e.getMessage() + "\nTrace:\n\n" + Log.getTrace(e)
          ).end();
        }
      } catch (Throwable t) {
        if ((log != null)&&(logLevel >= Log.ERROR)) {
          log.priority(Log.ERROR).add(
            "AEPServer ear thread caught unknown exception in earMain" +
            "\nTrace:\n\n" + Log.getTrace(t)
          ).end();
        }
      }
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPServer ear thread " + Thread.currentThread() + ": END OF LINE."
        ).end();
      }
      lock();
      runningFlag = running;
      if (runningFlag) {
        running = false;
        // stop() will not cancel, just join:
        unlock();
      } else {
        // someone has cancelled us (via stop()):
        unlock();
        // META: TESTCANCEL()... blah blah blah the end.
        //      pthread_testcancel();
      }
    } catch (Throwable ignored) {
    }
  }

  protected boolean processToken
  (Token token)
    throws IOException
  {
    boolean keepRunning = false;

    // AEPExceptions thrown during reception may not terminate the session, if
    // the server is error tolerant:
    token.setServer(this);
    if ((log != null)&&(logLevel >= Log.DEBUG)) {
      log.priority(Log.DEBUG).add("AEPServer: Received token: ");
      token.print(log, (logLevel > Log.DEBUG) ? logLevel : Log.DEBUG);
      log.end();
    }
    try {
      switch (token.getType()) {
      case AEPServer.Token.VOID:
        throw new AEPException(
          "INTERNAL ERROR: Received VOID AEPServer.Token in " +
          "AEPServer processing of AEPServer.Token",
          Log.FATAL
        );
      case AEPServer.Token.SYNCH:
      case AEPServer.Token.CLIENT_DISCONNECT:
      case AEPServer.Token.JOIN:
      case AEPServer.Token.LEAVE:
        keepRunning = token.handle();
        break;
      case AEPServer.Token.ASSIGN:
      case AEPServer.Token.APPLY:
      case AEPServer.Token.CLEAR:
        keepRunning = token.handle();
        break;
      default:
        {
          throw new AEPException(
            "INTERNAL ERROR: Got bad AEPServer.Token type int in " +
            "AEPServer processing of AEPServer.Token: " + token.getType(),
            Log.FATAL
          );
        }
      }
    } catch (AEPException e) {
      if ((log != null)&&(logLevel >= e.getPriority())) {
      log.priority(e.getPriority()).add(
        "Caught AEPException in AEPServer processing of AEPServer.Token: " +
        e.getMessage() + "\nTrace:\n\n" + Log.getTrace(e)
      ).end();
      }
      if ((errorTolerant)&&(e.getPriority() > Log.FATAL)) {
        keepRunning = true;
      }
    } catch (IntenseException e) {
      if ((log != null)&&(logLevel >= Log.CRITICAL)) {
        log.priority(Log.CRITICAL).add(
          "Caught IntenseException in AEPServer processing of " +
          "AEPServer.Token: " + e.getMessage() +
          "\nTrace:\n\n" + Log.getTrace(e)
        ).end();
      }
    } catch (Exception e) {
      if ((log != null)&&(logLevel >= Log.CRITICAL)) {
        log.priority(Log.CRITICAL).add(
          "Caught std.exception in AEPServer processing of " +
          "AEPServer.Token: " + e.getMessage() +
          "\nTrace:\n\n" + Log.getTrace(e)
        ).end();
      }
    } catch (Throwable t) {
      if ((log != null)&&(logLevel >= Log.CRITICAL)) {
        log.priority(Log.CRITICAL).add(
          "Caught unknown object in AEPServer processing of " +
          "AEPServer.Token" +
          "\nTrace:\n\n" + Log.getTrace(t)
        ).end();
      }
    }
    return keepRunning;
  }

  /**
   * Essentially instream->peek() in stream server, and condition wait in
   * shared server.  Returns true if lock is held.
   */
  protected abstract boolean blockForAvailableData
  ()
    throws IOException;

  protected void lockedSend
  (AEPClient.Token token)
    throws IOException, IntenseException, AEPException
  {
    sendLock();
    try {
      send(token);
    } finally {
      sendUnlock();
    }
  }

  protected void lockedAndLoggedSend
  (AEPClient.Token token, String location, long serverSequence,
   long clientSequence)
    throws AEPException
  {
    try {
      try {
        if ((log != null)&&(logLevel >= Log.DEBUG1)) {
          log.priority(Log.DEBUG1).add("AEPServer sending: ");
          token.print(log, logLevel);
          log.end();
        }
        lockedSend(token);
      } catch (Exception e) {
        if ((log != null)&&(logLevel >= Log.ERROR)) {
          log.priority(Log.ERROR).add(
            "AEPServer ear thread caught exception during sending of reply " +
            "(ss: " + serverSequence + ", cs: " + clientSequence +
            ") in " + location + ": " + e.getMessage()
          ).end();
        }
        throw e;
      }
    } catch (Exception e) {
      throw new AEPException(e);
    }
  }

  protected void leaveAllParticipants
  ()
    throws IntenseException
  {
    java.util.Iterator itr = participants.keySet().iterator();

    while (itr.hasNext()) {
      Participant participant = (Participant)participants.get(itr.next());

      participant.leave();
    }
    participants.clear();
  }

  protected void aep
  ()
    throws IOException, IntenseException, AEPException
  {
    boolean running = true;
    Token token = null;

    while (running) {
      if ((log != null)&&(logLevel >= Log.DEBUG2)) {
        log.priority(Log.DEBUG2).add(
          "AEPServer.aep: blocking for available data..."
        ).end();
      }
      if (!blockForAvailableData()) {
        receptionMutex.lock();
      }
      if ((log != null)&&(logLevel >= Log.DEBUG2)) {
        log.priority(Log.DEBUG2).add(
          "AEPServer.aep: data available; reading token..."
        ).end();
      }
      try {
        token = receive();
        if (token == null) {
          throw new AEPException(
            "INTERNAL ERROR: Received null AEPServer.Token"
          );
        }
        receptionMutex.unlock();
      } catch (Exception e) {
        if ((log != null)&&(logLevel >= Log.ERROR)) {
          log.priority(Log.ERROR).add(
            "AEPServer ear thread caught exception: " + e.getMessage()
          ).end();
        }
        running = false;
        receptionMutex.unlock();
      }
      if (running) {
        running = processToken(token);
      }
    }
  }

  /**
   * Start this server.
   */
  public void start
  ()
    throws IOException, IntenseException, AEPException
  {
    lock();
    if (running) {
      unlock();
      return;
    }
    earThread = new EarThread();
    try {
      earThread.start();
    } catch (IllegalThreadStateException e) {
      throw new AEPException(
        "Could not create AEPServer ear thread", e
      );
    }
    if ((log != null)&&(logLevel >= Log.NOTICE)) {
      log.priority(Log.NOTICE).add(
        "AEPServer.start created ear thread " + earThread + "."
      ).end();
    }
    running = true;
    unlock();
  }

  /**
   * Stop this server.
   */
  public void stop
  ()
      throws InterruptedException
  {
    boolean runningFlag;

    lock();
    runningFlag = running;
    if (running) {
      running = false;
    }
    // We can't lock later on in stop(), since the ear is using deferred
    // cancellation and we won't be able to cancel it if it has called
    // server.lock():
    if (runningFlag) {
      if ((log != null)&&(logLevel >= Log.NOTICE)) {
        log.priority(Log.NOTICE).add(
          "AEPServer.stop: stop ear thread " + earThread + "..."
        ).end();
      }
      try {
        closeConnection();
      } catch (Throwable t) {
        log.priority(Log.CRITICAL).add(
          "AEPServer.stop: unable to close connection\nTrace:" +
          Log.getTrace(t)
        ).end();
      }
    }
    unlock();
    if ((log != null)&&(logLevel >= Log.NOTICE)) {
      log.priority(Log.NOTICE).add(
        "AEPServer.stop: joining ear thread " + earThread + "..."
      ).end();
    }
    earThread.join();
    if ((log != null)&&(logLevel >= Log.NOTICE)) {
      log.priority(Log.NOTICE).add(
        "AEPServer.stop: joined ear thread " + earThread + '.'
      ).end();
    }
  }

  /**
   * Get a name for this server.
   */
  public abstract String getName
  ();

  protected abstract void closeConnection
  ()
    throws IOException;

}
