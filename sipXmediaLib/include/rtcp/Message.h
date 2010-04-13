//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _Message_h
#define _Message_h

#include "rtcp/RtcpConfig.h"

#ifdef __pingtel_on_posix__
#include "os/OsMsg.h"
#endif

//  Defines
#define MAX_ARGUMENTS       5
#define UNASSIGNED          (unsigned short)-1



/*|><|************************************************************************
*
* Class Name:   CMessage
*
* Inheritance:
*
* Methods:
*
* Attributes:
*
* Description:
*
*
*
*
************************************************************************|<>|*/
class CMessage
#ifdef __pingtel_on_posix__
    : public OsMsg
#endif
{

  public:   // Public Member Functions



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
    CMessage(unsigned long dwMsgType=0, void *pvArgument1=(void *)0,
             void *pvArgument2=(void *)0, void *pvArgument3=(void *)0,
             void *pvArgument4=(void *)0, void *pvArgument5=(void *)0);

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
    ~CMessage();


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
    void AddContents(unsigned long dwMsgType, void *pvArgument1=(void *)0,
                     void *pvArgument2=(void *)0, void *pvArgument3=(void *)0,
                     void *pvArgument4=(void *)0, void *pvArgument5=(void *)0);

#ifdef __pingtel_on_posix__
    virtual OsMsg * createCopy(void) const;
#endif

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
    void GetContents(unsigned long *pdwMsgType, void **ppvArgument1,
                     void **ppvArgument2, void **ppvArgument3,
                     void **ppvArgument4, void **ppvArgument5);


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
    void RemoveContents(void);



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
    unsigned long GetMsgType(void);


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
    void *GetFirstArgument(void);


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
    void *GetSecondArgument(void);

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
    void *GetThirdArgument(void);

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
    void *GetFourthArgument(void);

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
    void *GetFifthArgument(void);


  private:  // Private Data Members

/*|><|************************************************************************
*
* Attribute Name:   m_dwMsgType
*
* Type:             unsigned long
*
* Description:      An identifier for the type of message.
*
************************************************************************|<>|*/
    unsigned long m_dwMsgType;


/*|><|************************************************************************
*
* Attribute Name:   m_pvArguments[MAX_ARGUMENTS]
*
* Type:             void *
*
* Description:      An array of pointers containing arguments associated with
*                   the message.  A maximum of 5 (MAX_ARGUMENTS) optional
*                   arguments may be associated with a message.
*
************************************************************************|<>|*/
    void *m_pvArgument[MAX_ARGUMENTS];

/*|><|************************************************************************
*
* Attribute Name:   m_wArgIndex
*
* Type:             unsigned short
*
* Description:      An internal index used to identify the next argument to be
*                   retrieved by a caller.
*
************************************************************************|<>|*/
    unsigned short m_wArgIndex;

};


#endif
