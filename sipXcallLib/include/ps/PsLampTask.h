//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsLampTask_h_
#define _PsLampTask_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsBSem.h"
#include "os/OsRWMutex.h"
#include "os/OsTask.h"
#include "ps/PsLampDev.h"
#include "ps/PsLampInfo.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Task responsible for managing the phone set lamps
class PsLampTask : public OsTask
{

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static PsLampTask* getLampTask(void);
     //:Return a pointer to the Lamp task, creating it if necessary

   virtual
   ~PsLampTask();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus init(const int maxLampIndex);
     //:Cause the Lamp task to (re)initialize itself
     // The task will allocate an array [0..maxLampIndex] of PsLampInfo
     // objects to hold lamp state.

   OsStatus setLampInfo(int index,
                        int lampId,
                        const char* lampName,
                        PsLampInfo::LampMode lampMode);
     //:Set the lamp information for the lamp designated by "index"
     // Returns OS_NOT_FOUND if the index is out of range.

   OsStatus setMode(int lampId, PsLampInfo::LampMode lampMode);
     //:Set the mode for the lamp indicated by lampId
     // Returns OS_NOT_FOUND if there is no lamp with that lampId.

   OsStatus setMode(const char* pLampName, PsLampInfo::LampMode lampMode);
     //:Set the mode for the lamp indicated by pLampName
     // Returns OS_NOT_FOUND if there is no lamp with that name.

/* ============================ ACCESSORS ================================= */

   const PsLampInfo& getLampInfo(const int index);
     //:Return the lamp information for the lamp designated by "index"

   int getMaxLampIndex(void) const;
     //:Returns the max index for the array of PsLampInfo objects

   OsStatus getMode(int lampId, PsLampInfo::LampMode& rMode);
     //:Get the current mode for the lamp designated by lampId
     // The mode is returned in the "rMode" variable.
     // Returns OS_NOT_FOUND if there is no lamp with that lampId.

   OsStatus getMode(const char* pLampName, PsLampInfo::LampMode& rMode);
     //:Get the current mode for the lamp designated by pLampName
     // The mode is returned in the "rMode" variable.
     // Returns OS_NOT_FOUND if there is no lamp with that name.

   OsStatus getName(int lampId, const char*& rpName);
     //:Returns the name for the lamp designated by lampId
     // The name is returned in the "rpName" variable.
     // Returns OS_NOT_FOUND if there is no lamp with that lampId.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   PsLampTask();
     //:Constructor (called only indirectly via getLampTask())
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

   int run(void* pArg);
     //:The body of the task.
     // Responsible for updating the lamps as needed.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   enum LampTaskConstants
   {
      TICK_PERIOD_MSECS = 50,
      FLASH_ON_TICKS    = 10,
      FLASH_OFF_TICKS   = 10,
      FLUTTER_ON_TICKS  = 1,
      FLUTTER_OFF_TICKS = 1,
      WINK_ON_TICKS     = 1,
      WINK_OFF_TICKS    = 19
   };

   void calculateLampModeAggregates(void);
     //:Calculate the lamp mode aggregates (the lamp IDs that are turned on
     //:for each mode)

   void doCleanup(void);
     //:Release dynamically allocated storage
     // A write lock should be acquired before calling this method.

   int            mMaxLampIdx;       // max lamp index
   int            mModeTickMultiple; // common multiple for all tick modes
   OsRWMutex      mMutex;            // mutex for synchonizing access to data
   int            mTickCnt;          // current lamp tick count
   PsLampInfo*    mpLampInfo;        // ptr to an array of PsLampInfo objects
   PsLampDev*     mpLampDev;         // ptr to lamp device

                                     // Logical OR of lamps in various modes
   unsigned long  mModeBrokenFlutterLamps;  // Broken flutter mode lamps
   unsigned long  mModeFlashLamps;          // Flash mode lamps
   unsigned long  mModeFlutterLamps;        // Flutter mode lamps
   unsigned long  mModeSteadyLamps;         // Steady mode lamps
   unsigned long  mModeWinkLamps;           // Wink mode lamps

   unsigned long  mOnLamps;          // Lamps that are turned on

   // Static data members used to enforce Singleton behavior
   static PsLampTask* spInstance;    // pointer to the single instance of
                                     //  the PsLampTask class
   static OsBSem      sLock;         // semaphore used to ensure that there
                                     //  is only one instance of this class

   PsLampTask(const PsLampTask& rPsLampTask);
     //:Copy constructor (not implemented for this task)

   PsLampTask& operator=(const PsLampTask& rhs);
     //:Assignment operator (not implemented for this class)

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsLampTask_h_
