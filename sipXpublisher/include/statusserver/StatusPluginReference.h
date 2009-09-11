//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _StatusPluginReference_h_
#define _StatusPluginReference_h_

// SYSTEM INCLUDES
//#include <...>

// APPLICATION INCLUDES

#include <utl/UtlString.h>
#include <utl/UtlSList.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class UtlSListIterator;
class SubscribeServerPluginBase;
class TiXmlNode;

//:Class short description which may consist of multiple lines (note the ':')
// Class detailed description which may extend to multiple lines
class StatusPluginReference : public UtlString
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

/* ============================ CREATORS ================================== */

   StatusPluginReference(SubscribeServerPluginBase& plugin,
                         UtlString& eventType,
                         TiXmlNode& pluginNode);
     //:Default constructor

   virtual
   ~StatusPluginReference();
     //:Destructor

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

   bool hasPermissions() const;

   UtlSListIterator* permissionsIterator();

   void getEventType( UtlString& eventType ) const;

   SubscribeServerPluginBase* getPlugin() const;

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    StatusPluginReference& operator=(const StatusPluginReference& rhs);
    //:Assignment operator disabled

    StatusPluginReference(const StatusPluginReference& rStatusPluginReference);
    //:Copy constructor disabled

    UtlString mEventType;
    SubscribeServerPluginBase* mpPlugin;
    UtlSList mPermissions;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _StatusPluginReference_h_
