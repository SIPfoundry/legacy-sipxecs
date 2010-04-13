//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

#ifndef _SdpCodecFactory_h_
#define _SdpCodecFactory_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES
#include "utl/UtlDList.h"

#include <os/OsBSem.h>
#include <os/OsRWMutex.h>

#include <net/SdpCodec.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//: Factory and container for all supported codec types

// Class detailed description which may extend to multiple lines
class SdpCodecFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   static SdpCodecFactory* getSdpCodecFactory();
   //: Get singleton instance
   // Note: This class can be used in non-singleton mode
   // by avoiding the use of this one method.

   SdpCodecFactory(int numCodecs = 0,
                   SdpCodec* codecArray[] = NULL);
     //:Default constructor

   SdpCodecFactory(const SdpCodecFactory& rSdpCodecFactory);
     //:Copy constructor

   SdpCodecFactory& operator=(const SdpCodecFactory& rhs);
     //:Assignment operator

   virtual
   ~SdpCodecFactory();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   void addCodec(SdpCodec& newCodec);
   //: Add a new codec type to the list of known codecs

   void addCodecs(int numCodecs, SdpCodec* newCodecs[]);
   //: Add copies of the array of codecs

   void bindPayloadTypes();
   // Assign any unset payload type ids

   void copyPayloadType(SdpCodec& codec);
   //: If there is a matching codec in this factory, set its payload type to that of the given codec

   void copyPayloadTypes(int numCodecs, SdpCodec* codecArray[]);
   //: For all matching codecs, copy the payload type from the codecArray to the matching codec in this factory

   void clearCodecs(void);
   //: Discard all codecs

   int buildSdpCodecFactory(UtlString &codecList);
   //: Function just called other buildSdpCodecFactory. Here for compatibility

   int buildSdpCodecFactory(int codecCount, SdpCodec::SdpCodecTypes codecTypes[]);
   //: Add the default set of codecs specified in list; returns 0 if OK.

   void setCodecCPULimit(int iLimit);
     //:Limits the advertised codec by CPU limit level.
     //!param (in) iLimit - The limit level for codecs.  A value of
     //       SDP_CODEC_CPU_LOW indicates only low cpu intensity codecs and
     //       a value of SDP_CODEC_CPU_HIGH indicates either low or high
     //       cpu intensity.


/* ============================ ACCESSORS ================================= */

   const SdpCodec* getCodec(SdpCodec::SdpCodecTypes internalCodecId);
   //: Get a codec given an internal codec id

   const SdpCodec* getCodecByType(int payloadTypeId);
   //: Get a codec given the payload type id

   const SdpCodec* getCodec(const char* mimeType,
                            const char* mimeSubType);
   //: Get a codec given the mime type and subtype

   int getCodecCount();
   //: Get the number of codecs

   void getCodecs(int& numCodecs,
                  SdpCodec**& codecArray);

   void getCodecs(int& numCodecs,
                  SdpCodec**& codecArray,
                  const char* mimeType);

   void getCodecs(int& numCodecs,
                  SdpCodec**& codecArray,
                  const char* mimeType,
                  const char* subMimeType);
   //: Returns a copy of all the codecs

   void toString(UtlString& serializedFactory);
   //: String representation of factory and codecs

   static SdpCodec::SdpCodecTypes getCodecType(const char* pCodecName);
   //: Converts the readable text codec name into an enum defined in Sdpcodec.h

   int getCodecCPULimit();
     //:Gets the codec CPU limit level

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
   void addCodecNoLock(SdpCodec& newCodec);
   //: Add a new codec type to the list of known codecs

   UtlDList mCodecs;
   OsRWMutex mReadWriteMutex;
   int mCodecCPULimit ;

   // Note: the follwing are only needed for the
   // singleton instance method getSdpCodecFactory.
   // This class can be used in non-singleton mode
   // by avoiding the use of this one method.
   static SdpCodecFactory* spInstance;
   static OsBSem       sLock;       // semaphore used to ensure that there
                                    //  is only one instance of this class

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SdpCodecFactory_h_
