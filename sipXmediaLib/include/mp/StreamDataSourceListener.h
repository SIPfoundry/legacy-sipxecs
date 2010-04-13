//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
////////////////////////////////////////////////////////////////////////
//////


#ifndef _StreamDataSourceListener_h_
#define _StreamDataSourceListener_h_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "mp/StreamDataSource.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//:Listener interface for a StreamDataSource
class StreamDataSourceListener
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   StreamDataSourceListener();
     //:Default constructor

   virtual
   ~StreamDataSourceListener();
     //:Destructor

/* ============================ MANIPULATORS ============================== */

   virtual void dataSourceUpdate(StreamDataSource* pDataSource,
                                 StreamDataSourceEvent event) = 0 ;
     //: Informs the listener when the data soruce has an event to publish.
     //! param pDataSource - Data source publishing the state change
     //! param event - The new data source event state

/* ============================ ACCESSORS ================================= */

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

   StreamDataSourceListener(const StreamDataSourceListener& rStreamDataSourceListener);
     //:Copy constructor

   StreamDataSourceListener& operator=(const StreamDataSourceListener& rhs);
     //:Assignment operator

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:
};

/* ============================ INLINE METHODS ============================ */

#endif  // _StreamDataSourceListener_h_
