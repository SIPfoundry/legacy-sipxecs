//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipTransactionList_h_
#define _SipTransactionList_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlHashBag.h>

#include <net/SipTransaction.h>

#include <os/OsDefs.h>
#include <os/OsMutex.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class SipTransaction;
class SipMessage;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class SipTransactionList {
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    SipTransactionList();
    //:Default constructor

    virtual
    ~SipTransactionList();
    //:Destructor

/* ============================ MANIPULATORS ============================== */

    void addTransaction(SipTransaction* transaction,
                        UtlBoolean lockList = TRUE);
    //: Adds a transaction to the list
    // Note: does not make a copy

    SipTransaction* findTransactionFor(const SipMessage& message,
                                       UtlBoolean isOutgoing,
                                       enum SipTransaction::messageRelationship& relationship);
    //: Find a transaction for the given message
    // Note: the caller should first lock the list and keep
    // it locked until all access of the transaction is complete

    UtlBoolean transactionExists(const SipTransaction* transaction,
                                const UtlString& hash);
    //: Used to confirm a transaction is still good and has not been deleted
    // Note: this should only be used inside of a lock to prevent any sort
    // of delete race.

    UtlBoolean waitUntilAvailable(SipTransaction* transaction,
                                 const UtlString& hash);
    //: waits until the transaction is available and them marks it as busy
    // Note: be sure to check the return.  If returns false the transaction
    // is not locked or available.  Most likely as it got removed from
    // the list (e.g. deleted)

    void markAvailable(SipTransaction& transaction);
    //: Marks the transaction as available

    void removeOldTransactions(long oldTransaction,
                               long oldTcpTransaction);
    //: Remove transactions not accessed after given time

    void stopTransactionTimers();

    void deleteTransactionTimers();

/* ============================ ACCESSORS ================================= */

    void toString(UtlString& string);

    void toStringWithRelations(UtlString& string,
                               SipMessage& message,
                               UtlBoolean isOutGoing);

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:
    void lock();
    //: Locks the list for iteration, reading or writing

    void unlock();
    //: Unlock

/* //////////////////////////// PRIVATE /////////////////////////////////// */
    private:
    SipTransactionList(const SipTransactionList& rSipTransactionList);
    //:Copy constructor (disabled)

    SipTransactionList& operator=(const SipTransactionList& rhs);
    //:Assignment operator

    UtlHashBag mTransactions;
    OsMutex mListMutex;

};

/* ============================ INLINE METHODS ============================ */

#endif
