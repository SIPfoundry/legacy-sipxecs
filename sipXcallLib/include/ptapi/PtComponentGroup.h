//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PtComponentGroup_h_
#define _PtComponentGroup_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "tao/TaoClientTask.h"
#include "os/OsBSem.h"
#include "os/OsProtectEventMgr.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PtComponent;
class TaoReference;
class TaoObjectMap;

//:PtComponentGroup is a grouping of Component objects. Terminals may be
// composed of zero or more ComponentGroups. Applications query the
// PhoneTerminal interface for the available ComponentGroups. Then they query
// this interface for the components which make up this component group.
class PtComponentGroup
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum PtComponentGroupType
   {
      HEAD_SET = 1,
      HAND_SET,
      SPEAKER_PHONE,
      PHONE_SET,
      RINGER,
          EXTERNAL_SPEAKER,
          SOUND,        // for splash sound
      OTHER
   };

/* ============================ CREATORS ================================== */
   PtComponentGroup(int groupType, const UtlString& rDescription,
                       PtComponent* pComponents[], int nItems);
     //:Constructor

        PtComponentGroup();
     //:Default constructor (not implemented for this class)

   PtComponentGroup(const PtComponentGroup& rPtComponentGroup);
     //:Copy constructor (not implemented for this class)

   PtComponentGroup& operator=(const PtComponentGroup& rhs);
     //:Assignment operator (not implemented for this class)

   virtual
   ~PtComponentGroup();
     //:Destructor


/* ============================ MANIPULATORS ============================== */
        PtStatus setHandsetVolume(int level);

        PtStatus setSpeakerVolume(int level);

        PtStatus setExtSpeakerVolume(int level);

        PtStatus setRingerVolume(int level);


   UtlBoolean activate(void);
     //:Enables the audio apparatus associated with the component group.
     // Returns TRUE if successful, FALSE if unsuccessful

   UtlBoolean deactivate(void);
     //:Disables the audio apparatus associated with the component group.
     // Returns TRUE if successful, FALSE if unsuccessful

   void setTaoClient(TaoClientTask *pClient);

/* ============================ ACCESSORS ================================= */
   PtStatus getComponents(PtComponent* pComponents[], int size,
                          int& nItems);
     //:Returns pointers to the components in this group.
     // The caller provides an array that can hold up to <i>size</i>
     // PsTaoComponent pointers.  This method fills in the <i>pComponents</i>
     // array with up to <i>size</i> pointers.  The actual number of items
     // filled in is passed back via the <i>nItems</i> argument.
     // Returns OS_LIMIT_REACHED if there are more than <i>nItems</i>
     // components in the group.  Otherwise, returns OS_SUCCESS.

   PtStatus getDescription(char* pDescription, int maxLen);
     //:Returns a string describing the component group.

   int getType() { return mGroupType;} ;
     //:Returns the type of the component group, either HEAD_SET, HAND_SET,
     //:SPEAKER_PHONE, PHONE_SET or OTHER.

        PtStatus getHandsetVolume(int& level);

        PtStatus getSpeakerVolume(int& level);

        PtStatus getSpeakerNominalVolume(int& level);

        PtStatus getExtSpeakerVolume(int& level);

        PtStatus getExtSpeakerNominalVolume(int& level);

        PtStatus getRingerVolume(int& level);

/* ============================ INQUIRY =================================== */

   UtlBoolean isActivated(void);
     //:Determine whether the audio apparatus associated with the component
     //:group is enabled.
     // Returns TRUE if activated, FALSE if deactivated.

/* //////////////////////////// PROTECTED ///////////////////////////////// */
friend class PtTerminal;

protected:
        OsTime          mTimeOut;

        void initialize();

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
        OsProtectEventMgr *mpEventMgr;
   PtComponent** mpComponents;
   UtlString       mDescription;
   int             mGroupType;
   UtlBoolean       mIsActivated;
   int             mNumItems;
   TaoClientTask        *mpClient;

    static OsBSem           semInit ;
      //: Binary Semaphore used to guard initialiation and tear down
        static TaoReference             *mpTransactionCnt;
        static int                              mRef;

        int mComponentRef;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PtComponentGroup_h_
