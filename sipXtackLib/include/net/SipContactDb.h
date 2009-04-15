//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _SipContactDb_h_
#define _SipContactDb_h_

// SYSTEM INCLUDES
//#include <...>
#include <utl/UtlString.h>
#include <utl/UtlHashMap.h>
#include <os/OsMutex.h>
#include <os/OsSocket.h>

// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS



// FORWARD DECLARATIONS

class SipContactDb
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */
    SipContactDb();

    virtual
    ~SipContactDb();

    /**
     * Inserts a contact into the contact table.  Fails if there is
     * already an entry with the same port and IP address.
     * If the ID of the incoming CONTACT_ADDRESS is less that 1,
     * which it should be, then this method will assign
     * a contact id.
     * @param contact Reference to a contact structure, which will be
     *        copied, and the copy will be added to the DB.
     */   
    const bool addContact(CONTACT_ADDRESS& contact);

    /**
     * Removes a contact record from the DB.  
     *
     * @param id Key value (the contact id) used to find
     *        a matching record for deletion.
     */
    const bool deleteContact(const CONTACT_ID id);
    
    /** 
     * Finds a contact in the DB, by CONTACT_ID.
     *
     * @param id The CONTACT_ID of the record to find.
     */
    CONTACT_ADDRESS* find(CONTACT_ID id);

    /**
     * Finds the first contact by a given contact type
     */
    CONTACT_ADDRESS* findByType(CONTACT_TYPE type) ;

    /*
     * Find the local contact from a contact id.
     */
    CONTACT_ADDRESS* getLocalContact(CONTACT_ID id) ;

    /** 
     * Finds a contact in the DB, by IP address.
     *
     * @param id The IP Address of the record to find.
     */    
    CONTACT_ADDRESS* find(const UtlString szIpAddress, const int port);
    
    /**
     * Populates a CONTACT_ADDRESS array with all of the contacts
     * stored in this DB.
     *
     * @param contacts Pre-allocated array of CONTACT_ADDRESS pointers.
              Should be allocated using the MAX_IP_ADDRESSES for the size.
     * @param actualNum The number of contacts.
     */
    void getAll(CONTACT_ADDRESS* contacts[], int& actualNum) const;
    
    
    /**
     * Populates a CONTACT_ADDRESS array with all of the contacts
     * stored in this DB that match a particular adapter name.
     *
     * @param contacts Pre-allocated array of CONTACT_ADDRESS pointers.
              Should be allocated using the MAX_IP_ADDRESSES for the size.
     * @param szAdapter Adapter name for which to look-up contacts.
     * @param actualNum The number of contacts.
     */
    void getAllForAdapter(const CONTACT_ADDRESS* contacts[],
                                    const char* szAdapter,
                                    int& actualNum,
                                    CONTACT_TYPE typeFilter = ALL) const;
                                    
    const bool getRecordForAdapter(CONTACT_ADDRESS& contact,
                                             const char* szAdapter,
                                             const CONTACT_TYPE typeFilter) const;
    

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */


/* ============================ INQUIRY =================================== */


/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:



/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /** Disabled copy constructor */
    SipContactDb(const SipContactDb& rSipContactDb);

    //** Disabled assignment operator */
    SipContactDb& operator=(const SipContactDb& rhs);
    
    /** Checks this database for a duplicate record by key */
    const bool isDuplicate(const CONTACT_ID id);
    
    /** Checks this database for a duplicate record by ipAddress and port */
    const bool isDuplicate(const UtlString& ipAddress, const int port);
    
    /**
     * Given a contact record containing an ID which is set
     * to a value less than 1, this method will generate a contact 
     * ID.
     * 
     * @param contact Reference to the CONTACT_ADDRESS object to be
     *        modified.
     */
    const bool assignContactId(CONTACT_ADDRESS& contact);

    /** hash map storage for contact information, keyed by Contact Record ID */
    UtlHashMap mContacts;   

    int mNextContactId;
    
    mutable OsMutex mLock;
};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipContactDb_h_
