//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// Includes
#include "rtcp/Message.h"
#ifdef INCLUDE_RTCP /* [ */


/*|><|************************************************************************
*
* Method Name:  CMessage - Constructor
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
CMessage::CMessage(unsigned long dwMsgType, void *pvArgument1,
                   void *pvArgument2, void *pvArgument3,
                   void *pvArgument4, void *pvArgument5)
         :
#ifdef __pingtel_on_posix__
         OsMsg('R', 'C'), /* Arbitrary values: R for RTCP and C for CMessage */
#endif
         m_dwMsgType(UNASSIGNED),   // Default Initializers
         m_wArgIndex(0)
{

//  Store the contents of the message
    AddContents(dwMsgType, pvArgument1, pvArgument2, pvArgument3,
                  pvArgument4, pvArgument5);

}



/*|><|************************************************************************
*
* Method Name:  CMessage - Destructor
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
CMessage::~CMessage()
{

}


/*|><|************************************************************************
*
* Method Name:  AddContents
*
*
* Inputs:       unsigned long dwMsgType,
*               void *pvArgument1,
*               void *pvArgument2,
*               void *pvArgument3,
*               void *pvArgument4,
*               void *pvArgument5
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
void CMessage::AddContents(unsigned long dwMsgType, void *pvArgument1,
                           void *pvArgument2, void *pvArgument3,
                           void *pvArgument4, void *pvArgument5)
{


//  Store message Type
    m_dwMsgType = dwMsgType;

//  Store any accompanying arguments in array
    m_pvArgument[0] = pvArgument1;
    m_pvArgument[1] = pvArgument2;
    m_pvArgument[2] = pvArgument3;
    m_pvArgument[3] = pvArgument4;
    m_pvArgument[4] = pvArgument5;

}

#ifdef __pingtel_on_posix__ /* [ */
OsMsg * CMessage::createCopy(void) const
{
    CMessage * newmsg = new CMessage(*this);
    if(newmsg)
        newmsg->AddContents(m_dwMsgType, m_pvArgument[0], m_pvArgument[1],
                        m_pvArgument[2], m_pvArgument[3], m_pvArgument[4]);
    return (OsMsg *)newmsg;
}
#endif /* __pingtel_on_posix__ ] */

/*|><|************************************************************************
*
* Method Name:  GetContents
*
*
* Inputs:       None
*
* Outputs:      unsigned long *pdwMsgType,
*               void **ppvArgument1,
*               void **ppvArgument2,
*               void **ppvArgument3,
*               void **ppvArgument4,
*               void **ppvArgument5
*
* Returns:      None
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void CMessage::GetContents(unsigned long *pdwMsgType, void **ppvArgument1,
                           void **ppvArgument2, void **ppvArgument3,
                           void **ppvArgument4, void **ppvArgument5)
{


//  Return Message Type
    *pdwMsgType = m_dwMsgType;

//  Return any accompanying arguments
     *ppvArgument1 = m_pvArgument[0];
     *ppvArgument2 = m_pvArgument[1];
     *ppvArgument3 = m_pvArgument[2];
     *ppvArgument4 = m_pvArgument[3];
     *ppvArgument5 = m_pvArgument[4];

}
/*|><|************************************************************************
*
* Method Name:  RemoveContents
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
void CMessage::RemoveContents(void)
{

//  Clear Message Type and Arguments
    m_dwMsgType = UNASSIGNED;
    m_pvArgument[0] = (void *)0;
    m_pvArgument[1] = (void *)0;
    m_pvArgument[2] = (void *)0;
    m_pvArgument[3] = (void *)0;
    m_pvArgument[4] = (void *)0;

//  Reset Argument Index
    m_wArgIndex = 0;

}


/*|><|************************************************************************
*
* Method Name:  GetMsgType
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
unsigned long CMessage::GetMsgType(void)
{

    return(m_dwMsgType);
}

/*|><|************************************************************************
*
* Method Name:  GetFirstArgument
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      void *
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void * CMessage::GetFirstArgument(void)
{


//  Return corresponding argument
    return(m_pvArgument[0]);

}

/*|><|************************************************************************
*
* Method Name:  GetSecondArgument
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      void *
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void * CMessage::GetSecondArgument(void)
{


//  Return corresponding argument
    return(m_pvArgument[1]);

}

/*|><|************************************************************************
*
* Method Name:  GetThirdArgument
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      void *
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void * CMessage::GetThirdArgument(void)
{


//  Return corresponding argument
    return(m_pvArgument[2]);

}

/*|><|************************************************************************
*
* Method Name:  GetFourthArgument
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      void *
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void * CMessage::GetFourthArgument(void)
{


//  Return corresponding argument
    return(m_pvArgument[3]);

}

/*|><|************************************************************************
*
* Method Name:  GetFifthArgument
*
*
* Inputs:       None
*
* Outputs:      None
*
* Returns:      void *
*
* Logic Notes:
*
* Caveats:
*
************************************************************************|<>|*/
void * CMessage::GetFifthArgument(void)
{


//  Return corresponding argument
    return(m_pvArgument[4]);

}
#endif /* INCLUDE_RTCP ] */
