//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceNotifyReceiver_h_
#define _ResourceNotifyReceiver_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES

#include <net/SipMessage.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


//! Abstract class for processing NOTIFY callbacks within a ResourceListSet.
/** When a NOTIFY callback is received by a ResourceListSet, once the thread
 *  obtains the lock on the ResourceListSet, it looks up the dialog handle
 *  in mNotifyMap to find the ResourceNotifyReceiver responsible for the
 *  subscription, and calls its notifyEventCallback method.
 */
class ResourceNotifyReceiver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

   //! Construct an instance
   ResourceNotifyReceiver();

   //! Destructor
   virtual ~ResourceNotifyReceiver();

   /** Handle a NOTIFY callback for the subscription handled by this
    *  ResourceNotifyReceiver.
    */
   virtual void notifyEventCallback(const UtlString* dialogHandle,
                                    const SipMessage* msg) = 0;

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Disabled copy constructor
   ResourceNotifyReceiver(const ResourceNotifyReceiver& rResourceNotifyReceiver);

   //! Disabled assignment operator
   ResourceNotifyReceiver& operator=(const ResourceNotifyReceiver& rhs);
};

#endif  // _ResourceNotifyReceiver_h_
