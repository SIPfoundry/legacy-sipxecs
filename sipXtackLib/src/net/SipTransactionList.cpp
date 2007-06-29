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
#include <os/OsTask.h>
#include <os/OsEvent.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

//#define TIME_LOG /* turns on timestamping of finding and garbage collecting transactions */
#ifdef TIME_LOG
#  include <os/OsTimeLog.h>
#endif

// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
SipTransactionList::SipTransactionList() :
mTransactions(),
mListMutex(OsMutex::Q_FIFO)
{
}

// Copy constructor
SipTransactionList::SipTransactionList(const SipTransactionList& rSipTransactionList) :
mListMutex(OsMutex::Q_FIFO)
{
}

// Destructor
SipTransactionList::~SipTransactionList()
{
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
    UtlString callId;
    SipTransaction::buildHash(message, isOutgoing, callId);

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
        if(relationship == SipTransaction::MESSAGE_REQUEST ||
            relationship ==  SipTransaction::MESSAGE_PROVISIONAL ||
            relationship ==  SipTransaction::MESSAGE_FINAL ||
            relationship ==  SipTransaction::MESSAGE_NEW_FINAL ||
            relationship ==  SipTransaction::MESSAGE_CANCEL ||
            relationship ==  SipTransaction::MESSAGE_CANCEL_RESPONSE ||
            relationship ==  SipTransaction::MESSAGE_ACK ||
            relationship ==  SipTransaction::MESSAGE_2XX_ACK ||
            relationship ==  SipTransaction::MESSAGE_DUPLICATE)
        {
            break;
        }
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
        relationship =
            SipTransaction::MESSAGE_UNKNOWN;
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
            if (OsSysLog::willLog(FAC_SIP, PRI_WARNING))
            {
                OsSysLog::add(FAC_SIP, PRI_WARNING,
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

#if 0 // 
    UtlString bytes;
    int len;
    message.getBytes(&bytes, &len);
    OsSysLog::add(FAC_SIP, PRI_DEBUG
                  ,"SipTransactionList::findTransactionFor %p %s %s %s"
#                 ifdef TIME_LOG
                  "\n\tTime Log %s"
#                 endif
                  ,&message
                  ,isOutgoing ? "OUTGOING" : "INCOMING"
                  ,transactionFound ? "FOUND" : "NOT FOUND"
                  ,relationshipString(relationship)
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
    SipTransaction** transactionsToBeDeleted = NULL;
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

    int numTransactions = mTransactions.entries();
    if(numTransactions > 0)
    {
        UtlHashBagIterator iterator(mTransactions);
        SipTransaction* transactionFound = NULL;
        long transTime;

        // Pull all of the transactions to be deleted out of the list
        while ((transactionFound = (SipTransaction*) iterator()))
        {
           if(transactionFound->isBusy())
           {
              busyCount++;
           }
           else
           {
            transTime = transactionFound->getTimeStamp();

            // Invites need to be kept longer than other transactions
            if ( transTime < (  transactionFound->isMethod(SIP_INVITE_METHOD)
                              ? oldInviteTransaction
                              : oldTransaction
                              )
                )
            {
                // Remove it from the list
                mTransactions.removeReference(transactionFound);

                // Make sure we have a pointer array to hold it
                if(transactionsToBeDeleted == NULL)
                {
                     transactionsToBeDeleted = new SipTransaction*[numTransactions];
                }

                // Put it in the pointer array
                transactionsToBeDeleted[deleteCount] = transactionFound;
                deleteCount++;

                // Make sure the events waiting for the transaction
                // to be available are signaled before we delete
                // any of the transactions or we end up with
                // incomplete transaction trees (i.e. deleted branches)
                // :TODO: move to the actual deletion loop so we're not holding the lock? -SDL
                transactionFound->signalAllAvailable();
            }
           }
        }
    }

#   ifdef TIME_LOG
    gcTimes.addEvent("scan done");
#   endif

    // We do not need the lock now that the transactions have been
    // removed from the list
    unlock();

    if ( deleteCount || busyCount ) // do not log 'doing nothing when nothing to do', even at debug
    {
       OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransactionList::removeOldTransactions"
                     " deleting %d of %d transactions (%d busy)",
                     deleteCount , numTransactions, busyCount
                     );
    }

    // Delete the transactions in the array
    if (transactionsToBeDeleted)
    {
#      ifdef TIME_LOG
       gcTimes.addEvent("start delete");
#      endif

       for(int txIndex = 0; txIndex < deleteCount; txIndex++)
       {
          delete transactionsToBeDeleted[txIndex];
       }

#      ifdef TIME_LOG
       gcTimes.addEvent("finish delete");
#      endif

       delete[] transactionsToBeDeleted;
    }

#   ifdef TIME_LOG
    UtlString timeString;
    gcTimes.getLogString(timeString);
    OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransactionList::removeOldTransactions "
                  "%s", timeString.data()
                  );
#   endif
}

void SipTransactionList::stopTransactionTimers()
{
    lock();

    int numTransactions = mTransactions.entries();
    if(numTransactions > 0)
    {
        UtlHashBagIterator iterator(mTransactions);
        SipTransaction* transactionFound = NULL;

        while((transactionFound = (SipTransaction*) iterator()))
        {
            transactionFound->stopTimers();
        }
    }

    unlock();
}

void SipTransactionList::deleteTransactionTimers()
{
    lock();

    int numTransactions = mTransactions.entries();
    if(numTransactions > 0)
    {
        UtlHashBagIterator iterator(mTransactions);
        SipTransaction* transactionFound = NULL;

        while((transactionFound = (SipTransaction*) iterator()))
        {
            transactionFound->deleteTimers();
        }
    }

    unlock();
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
                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransactionList::waitUntilAvailable"
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

                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransactionList::waitUntilAvailable"
                              " %p waiting on: %p after %d tries",
                              transaction, waitEvent, numTries);

                OsStatus waitStatus;
                OsTime transBusyTimeout(1, 0);
                int waitTime = 0;
                do
                {
                    if(waitTime > 0)
                         OsSysLog::add(FAC_SIP, PRI_WARNING,
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
                    if (OsSysLog::willLog(FAC_SIP, PRI_WARNING))
                    {
                        UtlString transTree;
                        UtlString waitingTaskName;
                        OsTask* waitingTask = OsTask::getCurrentTask();
                        if(waitingTask) waitingTaskName = waitingTask->getName();
                        transaction->dumpTransactionTree(transTree, FALSE);
                        OsSysLog::add(FAC_SIP, PRI_WARNING,
                                      "SipTransactionList::waitUntilAvailable"
                                      " status: %d wait time: %d transaction: %p "
                                      " task: %s transaction tree: %s",
                                      waitStatus, waitTime, transaction,
                                      waitingTaskName.data(), transTree.data()
                                      );
                    }
                }

                OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransactionList::waitUntilAvailable"
                              " %p done waiting after %d tries",
                              transaction, numTries);
            }
        }
        else
        {
            unlock();
            OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransactionList::waitUntilAvailable"
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
        OsSysLog::add(FAC_SIP, PRI_ERR, "SipTransactionList::markAvailable"
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
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "SipTransactionList::transactionExists"
                      " transaction: %p hash: %s not found",
                      transaction, hash.data());
    }

    return(foundTransaction);
}

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
