//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _TLink_h
#define _TLink_h

#include "rtcp/RtcpConfig.h"

/*|><|************************************************************************
*
* Class Name:   CTLink
*
* Inheritance:
*
* Methods:
*
* Attributes:
*
* Description:  The CPTTLink is a base class which defines the set
*               of methods and data members required to store an entry
*               on a doubly linked list.
*
*
*
*
************************************************************************|<>|*/
template <class TENTRY>
class CTLink
{

  public:   // Public Member Functions

/*|><|************************************************************************
*
* Method Name:  CTLink - Constructor
*
*
* Inputs:       TENTRY tEntry
*
* Outputs:      None
*
* Returns:      None
*
* Description:  Creates a link to be placed on a doubly linked list and stores
*               the templated pointer as the payload.
*
* Usage Notes:  This constuctor shall set the next pointer of this
*               object to NULL.
*
*
************************************************************************|<>|*/
      CTLink(TENTRY tEntry);



/*|><|************************************************************************
*
* Method Name:  CTLink - Destructor
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      None
*
* Description:  The destructor shall handle the deallocation or resources that
*               shall occur prior to object termination.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      ~CTLink();



/*|><|************************************************************************
*
* Method Name:  SetNext
*
*
* Inputs:       CTLink<TENTRY> *ptLink
*
* Outputs:      None
*
* Returns:      None
*
* Description:  Set the pointer to the next link in the list to which this
*               entry is linked.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      void SetNext(CTLink<TENTRY>  *ptLink);


/*|><|************************************************************************
*
* Method Name:  SetPrevious
*
*
* Inputs:       CTLink<TENTRY> * ptLink
*
* Outputs:      None
*
* Returns:      None
*
* Description:  Set the pointer to the previous link in the list to which this
*               entry is linked.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      void SetPrevious(CTLink<TENTRY>  *ptLink);


/*|><|************************************************************************
*
* Method Name:  GetNext
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      CTLink<TENTRY> *
*
* Description:  Get the pointer to the next link in the list to which this
*               entry is linked.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      CTLink<TENTRY>  *GetNext(void);


/*|><|************************************************************************
*
* Method Name:  GetPrevious
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      CTLink<TENTRY> *
*
* Description:  Get the pointer to the previous link in the list to which this
*               entry is linked.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      CTLink<TENTRY>  *GetPrevious(void);


/*|><|************************************************************************
*
* Method Name:  GetEntry
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      TENTRY
*
* Description:  Get the entry associated with this particular link.
*
* Usage Notes:
*
*
************************************************************************|<>|*/
      TENTRY GetEntry(void);


  private:  // Private Member Functions

/*|><|************************************************************************
*
* Attribute Name:   m_tEntry
*
* Type:             TENTRY
*
* Description:      A reference to the entry associated with this link
*
************************************************************************|<>|*/
      TENTRY m_tEntry;

/*|><|************************************************************************
*
* Attribute Name:   m_ptNext
*
* Type:             CTLink<TENTRY> *
*
* Description:      A pointer to the next entry within the doubly linked list.
*
************************************************************************|<>|*/
      CTLink<TENTRY> *m_ptNext;


/*|><|************************************************************************
*
* Attribute Name:   m_ptPrevious
*
* Type:             CTLink<TENTRY> *
*
* Description:  A pointer to the previous entry within a doubly linked list.
*
************************************************************************|<>|*/
      CTLink<TENTRY> *m_ptPrevious;
};



/*|><|************************************************************************
*
* Method Name:  CTLink - Constructor
*
*
* Inputs:       TENTRY tEntry
*
* Outputs:      None
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
*
************************************************************************|<>|*/
template <class TENTRY>
CTLink<TENTRY>::CTLink(TENTRY tEntry)
     : m_ptNext(NULL), m_ptPrevious(NULL)

{
//  Allow the copy constructor to move the information
    m_tEntry = tEntry;

}



/*|><|************************************************************************
*
* Method Name:  CTLink - Destructor
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
*
************************************************************************|<>|*/
template <class TENTRY>
CTLink<TENTRY>::~CTLink()
{

//  Declarations
    CTLink<TENTRY> *ptNext, *ptPrevious;

//  Adjust the back pointer
    if((ptPrevious = GetPrevious()))
        ptPrevious->SetNext(GetNext());

//  Adjust the forward pointer
    if((ptNext = GetNext()))
        ptNext->SetPrevious(GetPrevious());


}

/*|><|************************************************************************
*
* Method Name:  GetEntry
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
* Caveats:
*
************************************************************************|<>|*/
template <class TENTRY>
inline TENTRY CTLink<TENTRY>::GetEntry(void)
{
          return m_tEntry;
}

/*|><|************************************************************************
*
* Method Name:  GetNext
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
inline CTLink<TENTRY> * CTLink<TENTRY>::GetNext(void)
{
        return m_ptNext;
}

/*|><|************************************************************************
*
* Method Name:  GetPrevious
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
inline CTLink<TENTRY> * CTLink<TENTRY>::GetPrevious(void)
{
          return m_ptPrevious;
}

/*|><|************************************************************************
*
* Method Name:  SetNext
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
inline void CTLink<TENTRY>::SetNext(CTLink<TENTRY> *  ptLink)
{
        m_ptNext = ptLink;
}

/*|><|************************************************************************
*
* Method Name:  SetPrevious
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
inline void CTLink<TENTRY>::SetPrevious(CTLink<TENTRY> *  ptLink)
{
         m_ptPrevious = ptLink;
}


#endif
