//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _CpMediaInterfaceFactory_h_
#define _CpMediaInterfaceFactory_h_

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
class CpMediaInterfaceFactoryImpl ;
class CpMediaInterface ;
class SdpCodec ;

/**
 * The CpMediaInterfaceFactory is responsible for create media interfaces.
 * Developers must install factory implementation by invoking the
 * setFactoryImplementation(...) method.
 *
 * The destructor of this factory will automatically destroy the
 * CpMediaIntefaceFactoryInterface.
 */
class CpMediaInterfaceFactory
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   /**
    * Default constructor
    */
   CpMediaInterfaceFactory();


   /**
    * Destructor
    */
   virtual ~CpMediaInterfaceFactory();

/* ============================ MANIPULATORS ============================== */

    /**
     * Set the factory implementation for creating media processing
     * connections. If an existing factory was set, it will be deleted and the
     * new factory will be used for future calls to createMediaInterface.
     *
     * The factory will automatically be deleted when the destructor of
     * CpMediaInterfaceFactory is invoked.
     */
    void setFactoryImplementation(CpMediaInterfaceFactoryImpl* pFactoryInterface) ;


    /**
     * Create a media interface using the designated
     * CpMediaIntefaceFactoryInterface.
     */
    CpMediaInterface* createMediaInterface(const char* publicAddress,
                                           const char* localAddress,
                                           int numCodecs,
                                           SdpCodec* sdpCodecArray[],
                                           const char* locale,
                                           int expeditedIpTos,
                                           const char* szStunServer,
                                           int stunOptions,
                                           int iStunKeepAlivePeriodSecs) ;


/* ============================ ACCESSORS ================================= */

    CpMediaInterfaceFactoryImpl* getFactoryImplementation() ;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:
    CpMediaInterfaceFactoryImpl* mpFactoryImpl ;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   /**
    * Disabled equals operator
    */
   CpMediaInterfaceFactory& operator=(const CpMediaInterfaceFactory& rhs);

   /**
    * Disabled copy constructor
    */
   CpMediaInterfaceFactory(const CpMediaInterfaceFactory& rCpMediaInterfaceFactory);
};

/* ============================ INLINE METHODS ============================ */

#endif  // _CpMediaInterfaceFactory_h_
