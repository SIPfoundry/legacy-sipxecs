//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include <utl/UtlString.h>
#include <utl/UtlHashBagIterator.h>

#include <net/SipTransactionList.h>
#include <net/SipTransaction.h>
#include <net/SipMessage.h>
#include <net/SipUserAgent.h>
#include <os/OsTask.h>
#include <os/OsEvent.h>
#include <os/OsDateTime.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

//#define TIME_LOG /* turns on timestamping of finding and garbage collecting transactions */
//#define TRANSACTION_MATCH_DEBUG // enable only for transaction match debugging - log is confusing otherwise

#ifdef TIME_LOG
#  include <os/OsTimeLog.h>
#endif

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipTransactionList::SipTransactionList(SipUserAgent* pSipUserAgent) :
mTransactions(),
mListMutex(OsMutex::Q_FIFO),
mpSipUserAgent(pSipUserAgent)
{
  //
  // Run the garbage collector
  //
  _pGarbageCollectionThread = 0;
  _abortGarbageCollection = false;
}

// Copy constructor
SipTransactionList::SipTransactionList(const SipTransactionList& rSipTransactionList) :
mListMutex(OsMutex::Q_FIFO)
{
}

// Destructor
SipTransactionList::~SipTransactionList()
{
    abortGarbageCollection();
    mTransactions.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
SipTransactionList&
SipTransactionList::operator=(const SipTransactionList& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

void SipTransactionList::addTransaction(SipTransaction* transaction,
                                        UtlBoolean lockList)
{
    if(lockList) lock();

    mTransactions.insert(transaction);

    if(lockList) unlock();
}

//: Find a transaction for the given message
SipTransaction*
SipTransactionList::findTransactionFor(const SipMessage& message,
                                       UtlBoolean isOutgoing,
                                       enum SipTransaction::messageRelationship& relationship)
{
    SipTransaction* transactionFound = NULL;
    SipTransaction* transaction2xxFound = NULL;
    enum SipTransaction::messageRelationship relationship2xx = SipTransaction::MESSAGE_UNKNOWN;
    UtlString callId;
    SipTransaction::buildHash(message, isOutgoing, callId);

    //
    // Call garbage collection before we further process existence of a transaction.
    //
    {
    boost::mutex::scoped_lock lock(_garbageCollectionMutex, boost::try_to_lock);
    if (lock)
    {
      mpSipUserAgent->garbageCollection();
    }
    //else { // some other thread is already doing garbageCollection, no need to wait here}
    }

    lock();

    // See if the message knows its transaction
    // DO NOT TOUCH THE CONTENTS of this transaction as it may no
    // longer exist.  It can only be used as an ID for the transaction.
    SipTransaction* messageTransaction = message.getSipTransaction();

    UtlString matchTransaction(callId);

    UtlHashBagIterator iterator(mTransactions, &matchTransaction);

    relationship = SipTransaction::MESSAGE_UNKNOWN;
#   ifdef TIME_LOG
    OsTimeLog findTimes;
    findTimes.addEvent("start");
#   endif
    while ((transactionFound = (SipTransaction*) iterator()))
    {
        // If the message knows its SIP transaction
        // and the found transaction pointer does not match, skip the
        // expensive relationship calculation
        // The messageTransaction MUST BE TREATED AS OPAQUE
        // as it may have been deleted.
        if(   messageTransaction && transactionFound != messageTransaction )
        {
#          ifdef TIME_LOG
           findTimes.addEvent("mismatch");
#          endif
           continue;
        }

        // If the transaction has never sent the original rquest
        // it should never get a match for any messages.
        if(   messageTransaction == NULL // this message does not point to this TX
           && ((transactionFound->getState()) == SipTransaction::TRANSACTION_LOCALLY_INIITATED)
           )
        {
#           ifdef TIME_LOG
            findTimes.addEvent("unsent");
#           endif
            continue;
        }

        relationship = transactionFound->whatRelation(message, isOutgoing);

        // Since 2xx ACK transactions always get a new Via branch, we have to
        // make sure there isn't another error transaction with a better match (branch included)
        //
        // NOTE: Adding this code makes obvious that ACK's for 2xx responses always(??? at least on initial invite)
        // match more than one transaction tree, the original tx tree in the forking proxy function and the
        // consequent tx tree in the auth proxy function.  This is a result of the fix for XECS-414, which
        // allows matching a transaction for a 2xx ACK without matching branch id's.
        // This 2xx-ACK-match-except-branch result combined with the failuer to find a complete 2xx-ACK_match-with-branch,
        // means we can be assured that this ACK should be sent upstream. The ACK is then treated as a new transaction
        // and the EXISTENCE of the matching transaction is used to navigate through the code.
        // The CONTENTS of the matched transaction are not important except for the matching itself.
        // ACKs for error responses will always match ONLY one transaction, since the branch id must also match.
        if(relationship ==  SipTransaction::MESSAGE_2XX_ACK ||
           relationship ==  SipTransaction::MESSAGE_2XX_ACK_PROXY )
        {
            if (transaction2xxFound)
            {
                UtlString bytes;
                ssize_t len;
                message.getBytes(&bytes, &len);
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG
                              ,"SipTransactionList::findTransactionFor"
                              " more than one 2xx match for message %p(%p) %s "
                              " previous match (%p) %s"
                              " current match (%p) %s"
                              ,&message
                              ,messageTransaction
                              ,isOutgoing ? "OUTGOING" : "INCOMING"
                              ,transaction2xxFound
                              ,SipTransaction::relationshipString(relationship2xx)
                              ,transactionFound
                              ,SipTransaction::relationshipString(relationship)
                              );
                relationship2xx = relationship;
                transaction2xxFound = transactionFound;
            }
            else
            {
                relationship2xx = relationship;
                transaction2xxFound = transactionFound;

                UtlString bytes;
                ssize_t len;
                message.getBytes(&bytes, &len);
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG
                              ,"SipTransactionList::findTransactionFor"
                              " 2xx match for message %p(%p) %s "
                              " current match (%p) %s"
                              ,&message
                              ,messageTransaction
                              ,isOutgoing ? "OUTGOING" : "INCOMING"
                              ,transactionFound
                              ,SipTransaction::relationshipString(relationship)
                              );
            }
            continue;
        }
        if(relationship ==  SipTransaction::MESSAGE_ACK )
        {
                UtlString bytes;
                ssize_t len;
                message.getBytes(&bytes, &len);
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG
                              ,"SipTransactionList::findTransactionFor"
                              " ACK match for message %p(%p) %s "
                              " current match (%p) %s"
                              " previous match (%p) %s"
                              ,&message
                              ,messageTransaction
                              ,isOutgoing ? "OUTGOING" : "INCOMING"
                              ,transactionFound
                              ,SipTransaction::relationshipString(relationship)
                              ,transaction2xxFound
                              ,SipTransaction::relationshipString(relationship2xx)
                              );
        }
        if(relationship == SipTransaction::MESSAGE_REQUEST ||
            relationship ==  SipTransaction::MESSAGE_PROVISIONAL ||
            relationship ==  SipTransaction::MESSAGE_FINAL ||
            relationship ==  SipTransaction::MESSAGE_NEW_FINAL ||
            relationship ==  SipTransaction::MESSAGE_CANCEL ||
            relationship ==  SipTransaction::MESSAGE_CANCEL_RESPONSE ||
            relationship ==  SipTransaction::MESSAGE_ACK ||
            relationship ==  SipTransaction::MESSAGE_DUPLICATE)
        {
            break;
        }
    }

    //
    if((transactionFound == NULL) && transaction2xxFound)
    {
        relationship = relationship2xx;
        transactionFound = transaction2xxFound;
    }

#   ifdef TIME_LOG
    if ( transactionFound )
    {
       findTimes.addEvent("found");
    }
    else
    {
       findTimes.addEvent("unfound");
    }
#   endif

    UtlBoolean isBusy = FALSE;
    if(transactionFound == NULL)
    {
        relationship = SipTransaction::MESSAGE_UNKNOWN;
    }
    else
    {
        isBusy = transactionFound->isBusy();
        if(!isBusy)
        {
            transactionFound->markBusy();
        }
    }

    unlock();
    if(transactionFound && isBusy)
    {
#       ifdef TIME_LOG
        findTimes.addEvent("checking");
#       endif

        // If we cannot lock it, it does not exist
        if(!waitUntilAvailable(transactionFound, callId))
        {
            if (Os::Logger::instance().willLog(FAC_SIP, PRI_WARNING))
            {
                Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                              "SipTransactionList::findTransactionFor"
                              " %p not available relation: %s",
                              transactionFound, SipTransaction::relationshipString(relationship));
            }
#           ifdef TIME_LOG
            findTimes.addEvent("notavail");
#           endif

            transactionFound = NULL;
        }
    }
#   ifdef TIME_LOG
    findTimes.addEvent("done");
    UtlString findTimeLog;
    findTimes.getLogString(findTimeLog);
#   endif

#ifdef TRANSACTION_MATCH_DEBUG
    UtlString bytes;
    ssize_t len;
    message.getBytes(&bytes, &len);
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG
                  ,"SipTransactionList::findTransactionFor %p(%p) %s %s(%p) %s"
#                 ifdef TIME_LOG
                  "\n\tTime Log %s"
#                 endif
                  ,&message
                  ,messageTransaction
                  ,isOutgoing ? "OUTGOING" : "INCOMING"
                  ,transactionFound ? "FOUND" : "NOT FOUND"
                  ,transactionFound
                  ,SipTransaction::relationshipString(relationship)
#                 ifdef TIME_LOG
                  ,findTimeLog.data()
#                 endif
                  );

#endif

    return(transactionFound);
}

void SipTransactionList::removeOldTransactions(long oldTransaction,
                                               long oldInviteTransaction)
{
    std::vector<SipTransaction*> transactionsToBeDeleted;
    int deleteCount = 0;
    int busyCount = 0;

#   ifdef TIME_LOG
    OsTimeLog gcTimes;
    gcTimes.addEvent("start");
#   endif

    lock();
#   ifdef TIME_LOG
    gcTimes.addEvent("locked");
#   endif

    OsTime time;
    OsDateTime::getCurTimeSinceBoot(time);
    long bootime = time.seconds();

    int numTransactions = mTransactions.entries();
    if(numTransactions > 0)
    {
        UtlHashBagIterator iterator(mTransactions);
        SipTransaction* transactionFound = NULL;
        long transTime;

        transactionsToBeDeleted.reserve(numTransactions);
        // Pull all of the transactions to be deleted out of the list
        while ((transactionFound = (SipTransaction*) iterator()))
        {
           if(transactionFound->isBusy())
           {
              busyCount++;
           }
           else if (!transactionFound->isMarkedForDeletion())
           {
            transTime = transactionFound->getTimeStamp();

            // Invites need to be kept longer than other transactions
            if ( transTime < (  transactionFound->isMethod(SIP_INVITE_METHOD)
                              ? oldInviteTransaction
                              : oldTransaction
                              )
                )
            {
#ifdef TRANSACTION_MATCH_DEBUG
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                              "SipTransactionList::removeOldTransactions "
                              " removing %p",  transactionFound );
#endif

                // Put it in the pointer array
                transactionsToBeDeleted.push_back(transactionFound);
                deleteCount++;

                transactionFound->markForDeletion();
                
                // Make sure the events waiting for the transaction
                // to be available are signaled before we delete
                // any of the transactions or we end up with
                // incomplete transaction trees (i.e. deleted branches)
                // :TODO: move to the actual deletion loop so we're not holding the lock? -SDL
                transactionFound->signalAllAvailable();
            }
            else if (
              transactionFound->isMethod(SIP_INVITE_METHOD)
              && transactionFound->getState() >= SipTransaction::TRANSACTION_COMPLETE
              && bootime - transTime > 32
              && !transactionFound->isMarkedForDeletion()
              && !transactionFound->getTopMostParent())
            {
              UtlSListIterator busyIter(transactionFound->childTransactions());
              SipTransaction* childTransaction = NULL;
              bool isChildBusy = false;

              while ((childTransaction = (SipTransaction*) busyIter()))
              {
                if (childTransaction->isBusy() || transactionFound->getState() < SipTransaction::TRANSACTION_COMPLETE)
                {
                  isChildBusy = true;
                  break;
                }
              }

              if (!isChildBusy)
              {
                //
                // Remove all child transactions
                //
                UtlSListIterator iterator(transactionFound->childTransactions());
                childTransaction = NULL;
                while ((childTransaction = (SipTransaction*) iterator()))
                {
                  if (!childTransaction->isMarkedForDeletion())
                  {
                    transactionsToBeDeleted.push_back(childTransaction);
                    childTransaction->signalAllAvailable();
                    childTransaction->markForDeletion();
                    deleteCount++;
                  }
                }
                //
                // Finally, delete the parent if it is not marked previously as
                // a child transaction ready for deletion.
                //
                transactionsToBeDeleted.push_back(transactionFound);
                deleteCount++;
                transactionFound->signalAllAvailable();
                transactionFound->markForDeletion();
              }
            }
          }
        }
    }

#   ifdef TIME_LOG
    gcTimes.addEvent("scan done");
#   endif   

    if ( deleteCount || busyCount || numTransactions > 10000 ) // do not log 'doing nothing when nothing to do', even at debug
    {
       Os::Logger::instance().log(FAC_SIP, PRI_NOTICE,
                     "SipTransactionList::removeOldTransactions"
                     " deleting %d of %d transactions (%d busy)",
                     deleteCount , numTransactions, busyCount
                     );
    }

    // Delete the transactions in the array
    if (!transactionsToBeDeleted.empty())
    {
#      ifdef TIME_LOG
       gcTimes.addEvent("start delete");
#      endif

       for(std::vector<SipTransaction*>::iterator iter = transactionsToBeDeleted.begin(); iter != transactionsToBeDeleted.end(); iter++)
       {
         mTransactions.removeReference(*iter);
          delete *iter;
       }

#      ifdef TIME_LOG
       gcTimes.addEvent("finish delete");
#      endif

    }

#   ifdef TIME_LOG
    UtlString timeString;
    gcTimes.getLogString(timeString);
    Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransactionList::removeOldTransactions "
                  "%s", timeString.data()
                  );
#   endif

    unlock();
}

void SipTransactionList::stopTransactionTimers()
{
#ifdef TIME_LOG
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipTransactionList::stopTransactionTimers entered");
#endif

   // It is possible that there are a very large number of transactions,
   // so we don't want to hold the lock while processing all of them.
   // So we make a list of the addresses of all the SipTransactions
   // and then process them afterward.

   lock();

   int numTransactions = mTransactions.entries();
   SipTransaction** transactionsToBeProcessed =
      new SipTransaction*[numTransactions];

   if (numTransactions > 0)
   {
      UtlHashBagIterator iterator(mTransactions);
      SipTransaction* transactionFound;
      int count = 0;

      while ((transactionFound = dynamic_cast <SipTransaction*> (iterator())))
      {
         transactionsToBeProcessed[count++] = transactionFound;
      }
   }

   unlock();

   // Now process each transaction in turn.
   for (int i = 0; i < numTransactions; i++)
   {
      lock();

      SipTransaction* transaction = transactionsToBeProcessed[i];
      // Verify (within a critical section) that this transaction is
      // still in mTransactions.
      if (mTransactions.findReference(transaction))
      {
         transaction->stopTimers();
      }

      unlock();

      // Let any threads that are waiting for mListMutex run.
      OsTask::yield();
   }

   // Delete list of transactions.
   delete[] transactionsToBeProcessed;

#ifdef TIME_LOG
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipTransactionList::stopTransactionTimers exited %d entries",
                 numTransactions);
#endif
}

void SipTransactionList::deleteTransactionTimers()
{
#ifdef TIME_LOG
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipTransactionList::deleteTransactionTimers entered");
#endif

   // It is possible that there are a very large number of transactions,
   // so we don't want to hold the lock while processing all of them.
   // So we make a list of the addresses of all the SipTransactions
   // and then process them afterward.

   lock();

   int numTransactions = mTransactions.entries();
   SipTransaction** transactionsToBeProcessed =
      new SipTransaction*[numTransactions];

   if (numTransactions > 0)
   {
      UtlHashBagIterator iterator(mTransactions);
      SipTransaction* transactionFound;
      int count = 0;

      while ((transactionFound = dynamic_cast <SipTransaction*> (iterator())))
      {
         transactionsToBeProcessed[count++] = transactionFound;
      }
   }

   unlock();

   // Now process each transaction in turn.
   for (int i = 0; i < numTransactions; i++)
   {
      lock();

      SipTransaction* transaction = transactionsToBeProcessed[i];
      // Verify (within a critical section) that this transaction is
      // still in mTransactions.
      if (mTransactions.findReference(transaction))
      {
         transaction->deleteTimers();
      }

      unlock();

      // Let any threads that are waiting for mListMutex run.
      OsTask::yield();
   }

   // Delete list of transactions.
   delete[] transactionsToBeProcessed;

#ifdef TIME_LOG
   Os::Logger::instance().log(FAC_SIP, PRI_DEBUG,
                 "SipTransactionList::deleteTransactionTimers exited %d entries",
                 numTransactions);
#endif
}

void SipTransactionList::toString(UtlString& string)
{
    lock();

    string.remove(0);

    UtlHashBagIterator iterator(mTransactions);
    SipTransaction* transactionFound = NULL;
    UtlString oneTransactionString;

    while((transactionFound = (SipTransaction*) iterator()))
    {
        transactionFound->toString(oneTransactionString, FALSE);
        string.append(oneTransactionString);
        oneTransactionString.remove(0);
    }

    unlock();
}

void SipTransactionList::toStringWithRelations(UtlString& string,
                                               SipMessage& message,
                                               UtlBoolean isOutGoing)
{
    lock();

    string.remove(0);

    UtlHashBagIterator iterator(mTransactions);
    SipTransaction* transactionFound = NULL;
    UtlString oneTransactionString;
    SipTransaction::messageRelationship relation;

    while((transactionFound = (SipTransaction*) iterator()))
    {
        relation = transactionFound->whatRelation(message, isOutGoing);
        string.append(SipTransaction::relationshipString(relation));
        string.append(" ");


        transactionFound->toString(oneTransactionString, FALSE);
        string.append(oneTransactionString);
        oneTransactionString.remove(0);

        string.append("\n");
    }

    unlock();
}

void SipTransactionList::lock()
{
    mListMutex.acquire();
}

void SipTransactionList::unlock()
{
    mListMutex.release();
}

UtlBoolean SipTransactionList::waitUntilAvailable(SipTransaction* transaction,
                                                  const UtlString& hash)
{
    UtlBoolean exists;
    UtlBoolean busy = FALSE;
    int numTries = 0;

    do
    {
        numTries++;

        lock();
        exists = transactionExists(transaction, hash);

        if(exists)
        {
            busy =  transaction->isBusy();
            if(!busy)
            {
                transaction->markBusy();
                unlock();
                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransactionList::waitUntilAvailable"
                              " %p locked after %d tries",
                              transaction, numTries);
            }
            else
            {
                // We set an event to be signaled when a
                // transaction is released.
                OsEvent* waitEvent = new OsEvent;
                transaction->notifyWhenAvailable(waitEvent);

                // Must unlock while we wait or there is a deadlock
                unlock();

                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransactionList::waitUntilAvailable"
                              " %p waiting on: %p after %d tries",
                              transaction, waitEvent, numTries);

                OsStatus waitStatus;
                OsTime transBusyTimeout(1, 0);
                int waitTime = 0;
                do
                {
                    if(waitTime > 0)
                         Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                                       "SipTransactionList::waitUntilAvailable"
                                       " %p still waiting: %d",
                                       transaction, waitTime);

                    waitStatus = waitEvent->wait(transBusyTimeout);
                    waitTime+=1;
                }
                while(waitStatus != OS_SUCCESS && waitTime < 2);

                // If we were never signaled, then we signal the
                // event so the other side knows that it has to
                // free up the event
                if(waitEvent->signal(-1) == OS_ALREADY_SIGNALED)
                {
                    delete waitEvent;
                    waitEvent = NULL;
                }

                // If we bailed out before the event was signaled
                // pretend the transaction does not exist.
                if(waitStatus != OS_SUCCESS)
                {
                    exists = FALSE;
                }

                if(waitTime > 1)
                {
                    if (Os::Logger::instance().willLog(FAC_SIP, PRI_WARNING))
                    {
                        UtlString transTree;
                        UtlString waitingTaskName;
                        OsTask* waitingTask = OsTask::getCurrentTask();
                        if(waitingTask) waitingTaskName = waitingTask->getName();
                        transaction->dumpTransactionTree(transTree, FALSE);
                        Os::Logger::instance().log(FAC_SIP, PRI_WARNING,
                                      "SipTransactionList::waitUntilAvailable"
                                      " status: %d wait time: %d transaction: %p "
                                      " task: %s transaction tree: %s",
                                      waitStatus, waitTime, transaction,
                                      waitingTaskName.data(), transTree.data()
                                      );
                    }
                }

                Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransactionList::waitUntilAvailable"
                              " %p done waiting after %d tries",
                              transaction, numTries);
            }
        }
        else
        {
            unlock();
            Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransactionList::waitUntilAvailable"
                          " %p gone after %d tries",
                          transaction, numTries);
        }
    }
    while(exists && busy);

    return(exists && !busy);
}

void SipTransactionList::markAvailable(SipTransaction& transaction)
{
    lock();

    if(!transaction.isBusy())
    {
        UtlString transactionString;
        transaction.toString(transactionString, FALSE);
        Os::Logger::instance().log(FAC_SIP, PRI_ERR, "SipTransactionList::markAvailable"
                      " transaction not locked: %s",
            transactionString.data());
    }
    else
    {
        transaction.markAvailable();
    }

    unlock();
}

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

UtlBoolean SipTransactionList::transactionExists(const SipTransaction* transaction,
                                                const UtlString& hash)
{
    UtlBoolean foundTransaction = FALSE;
    SipTransaction* aTransaction = NULL;
    UtlString matchTransaction(hash);
    UtlHashBagIterator iterator(mTransactions, &matchTransaction);

    while ((aTransaction = (SipTransaction*) iterator()))
    {
        if(aTransaction == transaction)
        {
            foundTransaction = TRUE;
            break;
        }
    }

    if(!foundTransaction)
    {
        Os::Logger::instance().log(FAC_SIP, PRI_DEBUG, "SipTransactionList::transactionExists"
                      " transaction: %p hash: %s not found",
                      transaction, hash.data());
    }

    return(foundTransaction);
}


void SipTransactionList::runGarbageCollection()
{
  if (!_pGarbageCollectionThread)
  {
    _pGarbageCollectionThread = new boost::thread(boost::bind(&SipTransactionList::runGarbageCollection, this));
    return;
  }

  while(!_abortGarbageCollection)
  {
    OsTask::delay(DEFAULT_GARBAGE_COLLECTOR_INTERVAL);
    boost::lock_guard<boost::mutex> lock(_garbageCollectionMutex);
    mpSipUserAgent->garbageCollection();
  }
}

void SipTransactionList::abortGarbageCollection()
{
  if (_pGarbageCollectionThread)
  {
    _abortGarbageCollection = true;
    _pGarbageCollectionThread->join();
    delete _pGarbageCollectionThread;
    _pGarbageCollectionThread = 0;
  }
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
