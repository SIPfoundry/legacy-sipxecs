//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _MpCodecFactory_h_
#define _MpCodecFactory_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsStatus.h"
#include "os/OsBSem.h"
#include "net/SdpCodec.h"
#include "mp/MpEncoderBase.h"
#include "mp/MpDecoderBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Singleton class used to generate encoder and decoder objects of a
//:an indicated type.
class MpCodecFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static MpCodecFactory* getMpCodecFactory(void);
     //:Return a pointer to the MpCodecFactory singleton object, creating
     //:it if necessary

   virtual
   ~MpCodecFactory();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   OsStatus createDecoder(SdpCodec::SdpCodecTypes internalCodecId,
                          int payloadType,
                          MpDecoderBase*& rpDecoder);
     //:Returns a new instance of a decoder of the indicated type
     //!param: internalCodecId - (in) codec type identifier
     //!param: payloadType - (in) RTP payload type associated with this decoder
     //!param: rpDecoder - (out) Reference to a pointer to the new decoder object

   OsStatus createEncoder(SdpCodec::SdpCodecTypes internalCodecId,
                          int payloadType,
                          MpEncoderBase*& rpEncoder);
     //:Returns a new instance of an encoder of the indicated type
     //!param: internalCodecId - (in) codec type identifier
     //!param: payloadType - (in) RTP payload type associated with this encoder
     //!param: rpEncoder - (out) Reference to a pointer to the new encoder object

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   MpCodecFactory();
     //:Constructor (called only indirectly via getMpCodecFactory())
     // We identify this as a protected (rather than a private) method so
     // that gcc doesn't complain that the class only defines a private
     // constructor and has no friends.

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   // Static data members used to enforce Singleton behavior
   static MpCodecFactory* spInstance; // pointer to the single instance of
                                      //  the MpCodecFactory class
   static OsBSem          sLock;      // semaphore used to ensure that there
                                      //  is only one instance of this class

   MpCodecFactory(const MpCodecFactory& rMpCodecFactory);
     //:Copy constructor (not supported)

   MpCodecFactory& operator=(const MpCodecFactory& rhs);
     //:Assignment operator (not supported)
};

/* ============================ INLINE METHODS ============================ */

#endif  // _MpCodecFactory_h_
