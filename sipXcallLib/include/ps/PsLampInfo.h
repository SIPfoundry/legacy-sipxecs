//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _PsLampInfo_h_
#define _PsLampInfo_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class PsLampTask;

//:Phone set button information
class PsLampInfo
{

friend class PsLampTask;

/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

   enum LampMode
   {
      OFF            = 0x00,
      STEADY         = 0x01,
      FLASH          = 0x02,     // slow on and off
      FLUTTER        = 0x04,     // fast on and off
      BROKEN_FLUTTER = 0x08,     // superposition of flash and flutter
      WINK           = 0x10,
   };

/* ============================ CREATORS ================================== */

   PsLampInfo(int         lampId=0,
              const char* pName="",
              LampMode    mode=OFF);
     //:Constructor
     // Default values are provided for all of the arguments so that it is
     // possible to allocate an array of PsLampInfo objects.

   PsLampInfo(const PsLampInfo& rPsLampInfo);
     //:Copy constructor

   virtual
   ~PsLampInfo();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   PsLampInfo& operator=(const PsLampInfo& rhs);
     //:Assignment operator

/* ============================ ACCESSORS ================================= */

   int getId(void) const;
     //:Returns the lamp ID

   const char* getName(void) const;
     //:Returns the lamp name

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

   int      mLampId;     // lamp identifier
   LampMode mLampMode;   // current lamp mode
   char*    mpLampName;  // name for this lamp

   // Access to the mode information for lamp objects needs to be
   // synchronized.  The following methods: setInfo(), setMode() and getMode()
   // are intended to be called indirectly via methods associated with the
   // PsLampTask (which ensures that access to the data is synchronized
   // appropriately.

   void setInfo(int lampId, LampMode mode, char* pLampName);
     //:Set all of the properties for the PsLampInfo object

   void setMode(LampMode mode);
     //:Set the lamp mode

   PsLampInfo::LampMode getMode(void) const;
     //:Returns the lamp mode

};

/* ============================ INLINE METHODS ============================ */

#endif  // _PsLampInfo_h_
