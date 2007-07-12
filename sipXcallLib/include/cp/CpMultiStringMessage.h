//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _CpMultiStringMessage_h_
#define _CpMultiStringMessage_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include <os/OsDefs.h>
#include <os/OsMsg.h>
#include <cp/CallManager.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class CpMultiStringMessage : public OsMsg
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

    CpMultiStringMessage(
       unsigned char messageSubtype = CallManager::CP_UNSPECIFIED,
       const char* str1 = NULL, const char* str2 = NULL,
       const char* str3 = NULL, const char* str4 = NULL,
       const char* str5 = NULL,
       int int1 = 0, int int2 = 0,
       int int3 = 0, int int4 = 0,
       int int5 = 0, int int6 = 0,
       int int7 = 0);
    //:Default constructor


   virtual
   ~CpMultiStringMessage();
     //:Destructor

   virtual OsMsg* createCopy(void) const;

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */
        void getString1Data(UtlString& str1) const;
        void getString2Data(UtlString& str2) const;
        void getString3Data(UtlString& str3) const;
        void getString4Data(UtlString& str4) const;
        void getString5Data(UtlString& str5) const;
    int getInt1Data() const;
        int getInt2Data() const;
        int getInt3Data() const;
    int getInt4Data() const;
    int getInt5Data() const;
    int getInt6Data() const;
    int getInt7Data() const;

    void toString(UtlString& dumpString, const char* terminator = "\n") const;


/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
    int mInt1;
    int mInt2;
    int mInt3;
    int mInt4;
    int mInt5;
    int mInt6;
    int mInt7;
        UtlString mString1Data;
        UtlString mString2Data;
        UtlString mString3Data;
        UtlString mString4Data;
        UtlString mString5Data;

   CpMultiStringMessage(const CpMultiStringMessage& rCpMultiStringMessage);
     //:disable Copy constructor

   CpMultiStringMessage& operator=(const CpMultiStringMessage& rhs);
     //:disable Assignment operator

};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMultiStringMessage_h_
