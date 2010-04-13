//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "utl/UtlSListIterator.h"
#include <assert.h>


// APPLICATION INCLUDES
#include "xmlparser/tinyxml.h"
#include "statusserver/StatusPluginReference.h"
#include "statusserver/SubscribeServerPluginBase.h"
#include "statusserver/PluginXmlParser.h"

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STATIC VARIABLE INITIALIZATIONS

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
StatusPluginReference::StatusPluginReference(SubscribeServerPluginBase& plugin,
                                             UtlString& eventType,
                                             TiXmlNode& pluginXmlNode) :
    UtlString(eventType)
{
    mEventType = eventType;
    mpPlugin = &plugin;

    // get permission associated with this plugin, if any
    TiXmlNode* nextNode = NULL;
    while ((nextNode = pluginXmlNode.IterateChildren(nextNode)))
    {
        if( nextNode->Type() == TiXmlNode::ELEMENT )
        {
            UtlString tagValue = nextNode->Value();
            if(tagValue.compareTo( XML_TAG_PERMISSIONMATCH, UtlString::ignoreCase ) == 0 )
            {
                // practically there should always be only one permission match tag but there
                // could be more than one permission associated with a plugin
                TiXmlElement* permissionMatchElement  = nextNode->ToElement();
                UtlBoolean permNodePresent = false;
                //get the user text value from it
                for( TiXmlNode* permissionNode = permissionMatchElement->FirstChild( XML_TAG_PERMISSION );
                     permissionNode;
                     permissionNode = permissionNode->NextSibling( XML_TAG_PERMISSION ) )
                {
                    permNodePresent = true;
                    TiXmlElement* permissionElement = permissionNode->ToElement();

                    //get permission Name
                    TiXmlNode* permissionText = permissionElement->FirstChild();
                    if(permissionText)
                    {
                        mPermissions.insert( new UtlString (permissionText->Value()) );
                    }
                }
            }
        }
    }
}

// Copy constructor
StatusPluginReference::StatusPluginReference(const StatusPluginReference& rStatusPluginReference)
{
}

// Destructor
StatusPluginReference::~StatusPluginReference()
{
    mPermissions.destroyAll();
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
StatusPluginReference&
StatusPluginReference::operator=(const StatusPluginReference& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   return *this;
}

/* ============================ ACCESSORS ================================= */

bool StatusPluginReference::hasPermissions() const
{
   return mPermissions.entries() > 0;
}

UtlSListIterator* StatusPluginReference::permissionsIterator()
{
   UtlSListIterator* iterator = new UtlSListIterator(mPermissions);
   return iterator;
}

void StatusPluginReference::getEventType( UtlString& eventType ) const
{
    eventType = mEventType;
}

SubscribeServerPluginBase* StatusPluginReference::getPlugin() const
{
    return(mpPlugin);
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */
