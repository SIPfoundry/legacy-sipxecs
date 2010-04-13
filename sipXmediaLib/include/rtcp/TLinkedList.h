//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TLinkedList_h
#define _TLinkedList_h

#include "rtcp/RtcpConfig.h"

// Includes
#include "BaseClass.h"
#include "TLink.h"


/*|><|************************************************************************
*
* Class Name:   CTLinkedList
*
* Inheritance:
*
* Methods:
*
* Attributes:
*
* Description:  CTLinkedList is a general purpose class template used to manage
*               doubly link lists of entries.  This linked list class shall
*               implement a customized allocator which shall retrieve memory
*               for a fixed number of items and shall reuse this memory through
*               maintaining a free list.  The services provided shall allow
*               links to be added, removed, and released.
*
*
*
************************************************************************|<>|*/
template <class TENTRY>
class CTLinkedList
{

  public:       // Public Member Functions


/*|><|************************************************************************
*
* Method Name:  CTLinkedList - Constructor
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Description:   The constructor shall handle any initialization which is
*                not subject to failure.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      CTLinkedList(void);


/*|><|************************************************************************
*
* Method Name:  CTLinkedList - Destructor
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Description:  The linked list destructor shall remove all entries from
*               the linked list returning the associated memory to
*               memory pool.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      virtual ~CTLinkedList(void);



/*|><|************************************************************************
*
* Method Name:  GetCount
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      unsigned long
*
* Description:  Returns the number of entries currently on the list.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      virtual unsigned long GetCount(void);

/*|><|************************************************************************
*
* Method Name:  AddEntry
*
*
* Inputs:       TENTRY tEntry
*
* Outputs:      None
*
* Returns:      bool
*
* Description:  Add an entry to the linked list.  The type of entry is
*               generated at compile time in response to the template
*               type provided.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      virtual bool AddEntry(TENTRY tEntry);


/*|><|************************************************************************
*
* Method Name:  GetFirstEntry
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Description:  Returns the first entry on the FIFO linked list.  A NULL entry
*               shall be returned if their are no entries on the list.  Entries
*               are not removed from the linked list as the result of this call.
*
* Usage Notes:  GetFirstEntry() and GetNextEntry() provide the user with the
*               ability to iterate through a linked list.  GetFirstEntry()
*               shall always reset the iterator to the head of the list while
*               successive calls to GetNextEntry() will cause the iterator to
*               move down the list.
*
*
*
************************************************************************|<>|*/
     virtual TENTRY GetFirstEntry(void);

/*|><|************************************************************************
*
* Method Name:  GetNextEntry
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Description:  Returns the next entry on the FIFO linked list.  A NULL entry
*               shall be returned if the end of the list has been encountered.
*               Entries are not removed from the linked list as the result of
*               this call.
*
* Usage Notes:  GetFirstEntry() and GetNextEntry() provide the user with the
*               ability to iterate through a linked list.  GetFirstEntry()
*               shall always reset the iterator to the head of the list while
*               successive calls to GetNextEntry() will cause the iterator to
*               move down the list.
*
*
************************************************************************|<>|*/
    virtual TENTRY GetNextEntry(void);

/*|><|************************************************************************
*
* Method Name:  RemoveFirstEntry
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Description:  Removes the first entry on the FIFO linked list.  A NULL entry
*               shall be returned if their are no entries on the list.
*
* Usage Notes:  RemoveFirstEntry() and RemoveNextEntry() provide the user with
*               the ability to iterate through a linked list.
*               RemoveFirstEntry() shall always reset the iterator to the head
*               of the list while successive calls to RemoveNextEntry() will
*               cause the iterator to move down the list.
*
*
*
************************************************************************|<>|*/
     virtual TENTRY RemoveFirstEntry(void);

/*|><|************************************************************************
*
* Method Name:  RemoveNextEntry
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Description:  Removes the next entry on the FIFO linked list.  A NULL entry
*               shall be returned if the end of the list has been encountered.
*
* Usage Notes:  RemoveFirstEntry() and RemoveNextEntry() provide the user with
*               the ability to iterate through a linked list.
*               RemoveFirstEntry() shall always reset the iterator to the head
*               of the list while successive calls to RemoveNextEntry() will
*               cause the iterator to move down the list.
*
*
************************************************************************|<>|*/
    virtual TENTRY RemoveNextEntry(void);


/*|><|************************************************************************
*
* Method Name:  GetEntry
*
*
* Inputs:       TENTRY tEntry
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Description:  Find an entry on the linked list matching the address passed.
*               If the entry is found, the address of the entry shall be
*               returned; otherwise, a NULL entry shall be returned.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
     virtual TENTRY GetEntry(TENTRY tEntry);

/*|><|************************************************************************
*
* Method Name:  GetEntry
*
*
* Inputs:       bool (*Comparitor)(TENTRY, void *),
*               void *
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Description:  Get an entry on the linked list with a primary lookup key
*               matching that passed.  If the entry is found, the address of
*               the entry shall be returned; otherwise, a NULL entry shall
*               be returned.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
     virtual TENTRY GetEntry(bool (*Comparitor)(TENTRY, void *),void *);

/*|><|************************************************************************
*
* Method Name:  RemoveEntry
*
*
* Inputs:       TENTRY tEntry
*
* Outputs:      None
*
* Returns:      TENTRY tEntry
*
* Description:  Remove an entry from the linked list matching the address
*               passed.  If the entry is found, the address of the entry
*               shall be returned; otherwise, a NULL entry shall be returned.
*
* Usage Notes:  The memory associated with the removed Entry shall not be
*               deallocated as a result of this call.
*
*
************************************************************************|<>|*/
     virtual TENTRY RemoveEntry(TENTRY tEntry);


/*|><|************************************************************************
*
* Method Name:  RemoveEntry
*
*
* Inputs:       bool (*Comparitor)(TENTRY, void *),void *
*
* Outputs:      None
*
* Returns:      TENTRY tEntry
*
* Description:  Remove an entry from the linked list with a primary lookup key
*               matching that passed.  If the entry is found, the address of
*               the entry shall be returned; otherwise, a NULL entry shall be
*               returned.
*
* Usage Notes:  The memory associated with the removed Entry shall not be
*               deallocated as a result of this call.
*
*
************************************************************************|<>|*/
     virtual TENTRY RemoveEntry(bool (*Comparitor)(TENTRY, void *),void *);

/*|><|************************************************************************
*
* Method Name:  RemoveAllEntries
*
*
* Inputs:       TENTRY tEntry
*
* Outputs:      None
*
* Returns:      void
*
* Description:  Remove all entries from a linked list matching the address
*               passed.
*
* Usage Notes:  The memory associated with the removed Entry shall not be
*               deallocated as a result of this call.
*
*
************************************************************************|<>|*/
     virtual void RemoveAllEntries(TENTRY tEntry);


/*|><|************************************************************************
*
* Method Name:  RemoveAllEntries
*
*
* Inputs:       bool (*Comparitor)(TENTRY, void *),void *
*
* Outputs:      None
*
* Returns:      void
*
* Description:  Remove all entries from the linked list with a primary lookup
*               key matching that passed.  If the entry is found, the address
*               of the entry shall be returned; otherwise, a NULL entry shall
*               be returned.
*
* Usage Notes:  The memory associated with the removed Entry shall not be
*               deallocated as a result of this call.
*
*
************************************************************************|<>|*/
     virtual void RemoveAllEntries(bool (*Comparitor)(TENTRY, void *),void *);


  private:  // Private Member Functions

/*|><|************************************************************************
*
* Method Name:  AddLink
*
*
* Inputs:       CTLink<TENTRY> *ptLink
*
* Outputs:      None
*
* Returns:      None
*
* Description:  Add a link to the linked list adjusting the forward and back
*               pointers of adjoining link as well as those of the head, tail,
*               and iterator.
*
* Usage Notes:
*
*
*
************************************************************************|<>|*/
    void AddLink(CTLink<TENTRY> *ptLink);

/*|><|************************************************************************
*
* Method Name:  RemoveLink
*
*
* Inputs:       CTLink<TENTRY> *ptLink
*
* Outputs:      None
*
* Returns:      TENTRY tEntry
*
* Description:  Remove a link from the linked list adjusting the forward and
*               back pointers of adjoining link as well as those of the head,
*               tail, and iterator.
*
* Usage Notes:  The memory associated with the removed link shall be
*               deallocated.  The entry contained within the deallocated link
*               shall remain intact with its pointer returned to the caller.
*
*
*
************************************************************************|<>|*/
    TENTRY RemoveLink(CTLink<TENTRY> *ptLink);

/*|><|************************************************************************
*
* Method Name:  ResetIterator
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      CTLink<TENTRY> *
*
* Description:  A private member which resets the internal iterator to the
*               first position within the FIFO
*
* Usage Notes:
*
************************************************************************|<>|*/
    CTLink<TENTRY> * ResetIterator(void);


/*|><|************************************************************************
*
* Method Name:  AdvanceIterator
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      CTLink<TENTRY> *
*
* Description:  A private member which advances the internal iterator to the
*               next position within the FIFO
*
* Usage Notes:
*
************************************************************************|<>|*/
    CTLink<TENTRY> *AdvanceIterator(void);



  private:  // Private Data Members

/*|><|************************************************************************
*
* Attribute Name:   m_dwCount
*
* Type:             unsigned long
*
* Description:      A count of the number of entries currently on the list.
*
************************************************************************|<>|*/
      unsigned long m_dwCount;

/*|><|************************************************************************
*
* Attribute Name:   m_ptHead
*
* Type:             CTLink<TENTRY> *
*
* Description:  This is a pointer to the head of the list.  The head shall
*               be set to NULL when the list is empty or shall point to the
*               first entry on the list.
*
************************************************************************|<>|*/
      CTLink<TENTRY> *m_ptHead;

/*|><|************************************************************************
*
* Attribute Name:   m_ptTail
*
* Type:             CTLink<TENTRY> *
*
* Description:      This is a pointer to the tail of the list. The tail shall
*                   be set to NULL when the list is empty or shall point to
*                   the last entry on the list.
*
************************************************************************|<>|*/
      CTLink<TENTRY> *m_ptTail;


/*|><|************************************************************************
*
* Attribute Name:   m_ptIterator
*
* Type:             CTLink<TENTRY> *
*
* Description:      This is a pointer to the iterator of the list. The iterator
*                   shall be set to NULL when the list is empty.  It shall
*                   point to the head of the list at list creation or following
*                   a call to GetFirstEntry().  It shall be advanced to the
*                   next entry within the list upon each call to GetNextEntry.
*
************************************************************************|<>|*/
      CTLink<TENTRY> *m_ptIterator;

/*|><|************************************************************************
*
* Attribute Name:   m_csSynchronized
*
* Type:             CRITICAL_SECTION
*
* Description:      This is a critical section used to synchronize thread
*                   access to this class.  This shall prevent several
*                   contending threads from executing interleaving linked
*                   list operations.
*
************************************************************************|<>|*/
      CRITICAL_SECTION m_csSynchronized;
};



/*|><|************************************************************************
*
* Method Name:  CTLinkedList - Constructor
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
CTLinkedList<TENTRY>::CTLinkedList(void)
            : m_dwCount(0), m_ptHead(NULL), m_ptTail(NULL), m_ptIterator(NULL)
#ifndef WIN32
            , m_csSynchronized(NULL)
#endif
{

//  Initialize Critical Section
    InitializeCriticalSection (&m_csSynchronized);

}


/*|><|************************************************************************
*
* Method Name:  CTLinkedList - Destructor
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:  Traverses a doubly linked list from the bottom up deleting
*               all list elements which remain at the time of destruction.
*
* Caveats:
*
*
************************************************************************|<>|*/
template <class TENTRY>
CTLinkedList<TENTRY>::~CTLinkedList(void)
{

//  Declarations
    CTLink<TENTRY> *ptLink;
    TENTRY tEntry;

//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Reset List Pointer
    ptLink = ResetIterator();

//  While Links remain
    while (ptLink)
    {
//      Delete Link
        tEntry = ptLink->GetEntry();
        delete ptLink;

//      Advance to next link
        ptLink = AdvanceIterator();
    }

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

//  Delete the Critical Section
    DeleteCriticalSection (&m_csSynchronized);

}


/*|><|************************************************************************
*
* Method Name:  GetCount
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      unsigned long
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
unsigned long CTLinkedList<TENTRY>::GetCount(void)
{

    return(m_dwCount);

}

/*|><|************************************************************************
*
* Method Name:  AddEntry
*
*
* Inputs:       TENTRY tEntry
*
* Outputs:      None
*
* Returns:      bool
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
bool CTLinkedList<TENTRY>::AddEntry(TENTRY tEntry)
{

//  Declarations
    CTLink<TENTRY> *ptLink;

//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Create new Link object.  If a failure occurs, return FALSE
    if(!(ptLink = new CTLink<TENTRY>(tEntry)))
    {
//      Leave Synchronized Area
        LeaveCriticalSection (&m_csSynchronized);

        return(FALSE);
    }

    AddLink(ptLink);

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

    return(TRUE);


}


/*|><|************************************************************************
*
* Method Name:  GetFirstEntry
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Logic Notes:
*
* Caveats:      Both GetFirst and GetNext entry maintain a single context.
*               Undesired results will occur if two or more users of a linked
*               list attempt to use its iterator simultaneously.
*
************************************************************************|<>|*/
template <class TENTRY>
TENTRY CTLinkedList<TENTRY>::GetFirstEntry(void)
{

//  Declarations
    CTLink<TENTRY> *ptLink;

//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Reset the list iterator to the tail where the first entry in resides.
    ptLink = ResetIterator();

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

//  If the Iterator is equal to NULL, then the list is empty
    if(!ptLink)
        return(NULL);

//  Return entry
    return(ptLink->GetEntry());
}

/*|><|************************************************************************
*
* Method Name:  GetNextEntry
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Logic Notes:
*
* Caveats:      Both GetFirst and GetNext entry maintain a single context.
*               Undesired results will occur if two or more users of a linked
*               list attempt to use its iterator simultaneously.
*
************************************************************************|<>|*/
template <class TENTRY>
TENTRY CTLinkedList<TENTRY>::GetNextEntry(void)
{

//  Declarations
    CTLink<TENTRY> *ptLink;

//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Advance List Iterator
    ptLink = AdvanceIterator();

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

//  If the Iterator is equal to NULL, then the list is empty or the iterator
//  has reached the end of the list
    if(!ptLink)
        return(NULL);


    return(ptLink->GetEntry());

}

/*|><|************************************************************************
*
* Method Name:  RemoveFirstEntry
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Logic Notes:
*
* Caveats:      Both RemoveFirst and RemoveNext entry maintain a single
*               context.  Undesired results will occur if two or more users
*               of a linked list attempt to use its iterator simultaneously.
*               Undesirable effects will also occur if Get and Remove
*               First/Next Entry calls are interleaved.  Both sets of calls
*               operate off the same iterator.
*
************************************************************************|<>|*/
template <class TENTRY>
TENTRY CTLinkedList<TENTRY>::RemoveFirstEntry(void)
{

//  Declarations
    CTLink<TENTRY> *ptLink;
    TENTRY tEntry;

//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Reset the list iterator to the tail where the first entry in resides.
    ptLink = ResetIterator();


//  If the Iterator is equal to NULL, then the list is empty
    if(!ptLink)
    {
//      Leave Synchronized Area
        LeaveCriticalSection (&m_csSynchronized);

        return(NULL);
    }


//  Remove entry
    tEntry = RemoveLink(ptLink);

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

    return(tEntry);

}

/*|><|************************************************************************
*
* Method Name:  RemoveNextEntry
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Logic Notes:
*
* Caveats:  Both RemoveFirst and RemoveNext entry maintain a single context.
*           Undesired results will occur if two or more users of a linked list
*           attempt to use its iterator simultaneously.  Undesirable effects
*           will also occur if Get and Remove First/Next Entry calls are
*           interleaved.  Both sets of calls operate off the same iterator.
*
************************************************************************|<>|*/
template <class TENTRY>
TENTRY CTLinkedList<TENTRY>::RemoveNextEntry(void)
{

//  Declarations
    CTLink<TENTRY> *ptLink;
    TENTRY ptEntry;

//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Advance List Iterator
    ptLink = AdvanceIterator();

//  If the Iterator is equal to NULL, then the list is empty or the iterator
//  has reached the end of the list
    if(!ptLink)
    {
//      Leave Synchronized Area
        LeaveCriticalSection (&m_csSynchronized);

        return(NULL);
    }

//  Remove entry
    ptEntry = RemoveLink(ptLink);

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

    return(ptEntry);

}


/*|><|************************************************************************
*
* Method Name:  GetEntry
*
*
* Inputs:       TENTRY tEntry
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
TENTRY CTLinkedList<TENTRY>::GetEntry(TENTRY tEntry)
{
//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Initialize link pointer
    CTLink<TENTRY> *ptLink = m_ptTail;

//  Iterate through the list looking for the matching entry
    while(ptLink)
    {
//      Check for matching entry address
        if(tEntry == ptLink->GetEntry())
        {
//          Leave Synchronized Area
            LeaveCriticalSection (&m_csSynchronized);

            return(tEntry);
        }

//      Advance the pointer and try again
        ptLink = ptLink->GetPrevious();
    }

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

    return(NULL);

}

/*|><|************************************************************************
*
* Method Name:  GetEntry
*
*
* Inputs:       bool (*Comparitor)(TENTRY, void *),void *
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
TENTRY CTLinkedList<TENTRY>::GetEntry(
                    bool (*Comparitor)(TENTRY, void *),void *pvLookupKey)
{
//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Initialize link pointer
    CTLink<TENTRY> *ptLink = m_ptTail;
    TENTRY tEntry;

//  Iterate through the list looking for the matching entry
    while(ptLink)
    {
//      Get entry from link
        tEntry = ptLink->GetEntry();

//      Check for matching entry address
        if((*Comparitor)(tEntry, pvLookupKey))
        {
//          Leave Synchronized Area
            LeaveCriticalSection (&m_csSynchronized);

            return(tEntry);
        }

//      Advance the pointer and try again
        ptLink = ptLink->GetPrevious();
    }

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

    return(NULL);

}

/*|><|************************************************************************
*
* Method Name:  RemoveEntry
*
*
* Inputs:       TENTRY tEntry
*
* Outputs:      None
*
* Returns:      bool
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
TENTRY CTLinkedList<TENTRY>::RemoveEntry(TENTRY tEntry)
{
//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Initialize link pointer
    CTLink<TENTRY> *ptLink = m_ptTail;

//  Iterate through the list looking for the matching entry
    while(ptLink)
    {
//      Check for matching entry address
        if(tEntry == ptLink->GetEntry())
        {
            tEntry = RemoveLink(ptLink);

//          Leave Synchronized Area
            LeaveCriticalSection (&m_csSynchronized);

            return(tEntry);
        }


//      Advance the pointer and try again
        ptLink = ptLink->GetPrevious();
    }

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

    return(NULL);

}


/*|><|************************************************************************
*
* Method Name:  RemoveEntry
*
*
* Inputs:       bool (*Comparitor)(TENTRY, void *),
*               void *pvLookupKey
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
TENTRY CTLinkedList<TENTRY>::RemoveEntry(
                      bool (*Comparitor)(TENTRY, void *),void *pvLookupKey)
{
//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Initialize link pointer
    CTLink<TENTRY> *ptLink = m_ptTail;
    TENTRY tEntry;

//  Iterate through the list looking for the matching entry
    while(ptLink)
    {
//      Get entry from link
        tEntry = ptLink->GetEntry();

//      Check for matching entry address
        if((*Comparitor)(tEntry, pvLookupKey))
        {
            tEntry = RemoveLink(ptLink);

//          Leave Synchronized Area
            LeaveCriticalSection (&m_csSynchronized);

            return(tEntry);
        }

//      Advance the pointer and try again
        ptLink = ptLink->GetPrevious();
    }

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

    return(NULL);

}

/*|><|************************************************************************
*
* Method Name:  RemoveAllEntries
*
*
* Inputs:       TENTRY tEntry
*
* Outputs:      None
*
* Returns:      void
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
void CTLinkedList<TENTRY>::RemoveAllEntries(TENTRY tEntry)
{
//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Initialize link pointer
    CTLink<TENTRY> *ptLink = m_ptTail;

//  Iterate through the list looking for the matching entry
    while(ptLink)
    {
//      Check for matching entry address
        if(ptLink->GetEntry())
        {
            CTLink<TENTRY> *ptNextLink = ptLink->GetPrevious();
            RemoveLink(ptLink);
            ptLink = ptNextLink;
            continue;
        }


//      Advance the pointer and try again
        ptLink = ptLink->GetPrevious();
    }

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

    return;

}


/*|><|************************************************************************
*
* Method Name:  RemoveAllEntries
*
*
* Inputs:       bool (*Comparitor)(TENTRY, void *),
*               void *pvLookupKey
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
void CTLinkedList<TENTRY>::RemoveAllEntries(
                      bool (*Comparitor)(TENTRY, void *),void *pvLookupKey)
{
//  Enter Synchronized Area
    EnterCriticalSection (&m_csSynchronized);

//  Initialize link pointer
    CTLink<TENTRY> *ptLink = m_ptTail;
    TENTRY tEntry;

//  Iterate through the list looking for the matching entry
    while(ptLink)
    {
//      Get entry from link
        tEntry = ptLink->GetEntry();

//      Check for matching entry address
        if((*Comparitor)(tEntry, pvLookupKey))
        {
            CTLink<TENTRY> *ptNextLink = ptLink->GetPrevious();
            RemoveLink(ptLink);
            ptLink = ptNextLink;
            continue;
        }

//      Advance the pointer and try again
        ptLink = ptLink->GetPrevious();
    }

//  Leave Synchronized Area
    LeaveCriticalSection (&m_csSynchronized);

    return;

}


/*|><|************************************************************************
*
* Method Name:  AddLink
*
*
* Inputs:       CTLink<TENTRY> *ptLink
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
void CTLinkedList<TENTRY>::AddLink(CTLink<TENTRY> *ptLink)
{

//  Add to the head of the Linked List
    ptLink->SetPrevious(NULL);
    ptLink->SetNext(m_ptHead);

//  If the head points to an entry, set that entry's back pointer
//  to the newly added entry
    if(m_ptHead)
        m_ptHead->SetPrevious(ptLink);

//  Assign the link to the head pointer
    m_ptHead = ptLink;

//  If the tail doesn't point to an entry, assign the tail with
//  the pointer of the new entry
    if(!m_ptTail)
        m_ptTail = ptLink;

//  Increment Count
    m_dwCount++;

}

/*|><|************************************************************************
*
* Method Name:  RemoveLink
*
*
* Inputs:       CTLink<TENTRY> *ptLink
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
TENTRY CTLinkedList<TENTRY>::RemoveLink(CTLink<TENTRY> *ptLink)
{
//  Declarations
    TENTRY tEntry;

//  Check if the Link passed is valid
    if(!ptLink)
        return(NULL);

//  Check if the entry is assigned to the head.  If so, adjust the head
    if(ptLink == m_ptHead)
        m_ptHead = ptLink->GetNext();

//  Check if the entry is assigned to the tail.  If so, adjust the tail
    if(ptLink == m_ptTail)
        m_ptTail = ptLink->GetPrevious();

//  Check if the entry is assigned to the iterator.  If so, adjust the iterator
    if(ptLink == m_ptIterator)
        m_ptIterator = ptLink->GetPrevious();

//  Decrement count
    m_dwCount--;

//  Get Entry
    tEntry = ptLink->GetEntry();

//  Delete Link
    delete ptLink;

    return(tEntry);

}

/*|><|************************************************************************
*
* Method Name:  ResetIterator
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      CTLink<TENTRY> *
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
CTLink<TENTRY> * CTLinkedList<TENTRY>::ResetIterator(void)
{

//  Reset Iterator
    m_ptIterator = m_ptTail;

//  Advance iterator in anticipation of the next call while
//  returning the link currently pointed to.
    return(AdvanceIterator());

}


/*|><|************************************************************************
*
* Method Name:  AdvanceIterator
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      CTLink<TENTRY> *
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
CTLink<TENTRY> * CTLinkedList<TENTRY>::AdvanceIterator(void)
{

//  Declarations
    CTLink<TENTRY> *ptListPtr = m_ptIterator;

//  Check whether link is still valid.  If so, assign the
//  adjacent link to be the new position of the iterator
    if(ptListPtr)
       m_ptIterator = ptListPtr->GetPrevious();

    return ptListPtr;

}




#endif
