//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef PLUGINXMLPARSER_H
#define PLUGINXMLPARSER_H

// SYSTEM INCLUDES



// APPLICATION INCLUDES
#include "xmlparser/tinyxml.h"
#include "os/OsRWMutex.h"
#include "statusserver/SubscribeServerPluginBase.h"
#include "utl/UtlHashBag.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class TiXmlDocument;
class Notifier;
class StatusPluginReference;

class PluginXmlParser
{
public:
    PluginXmlParser();

    virtual ~PluginXmlParser();

    OsStatus loadPlugins( const UtlString configFileName, Notifier* notifier );

    int getListSize();

    StatusPluginReference* getPlugin( const UtlString& eventType );

private:
	UtlHashBag mPluginTable;
    Notifier* mNotifier;

    TiXmlDocument* mDoc;
	OsRWMutex mListMutexR;
	OsRWMutex mListMutexW;

    void addPluginToList(StatusPluginReference* plugin);

    OsStatus loadPlugin( TiXmlElement& pluginElement,
		                 Notifier* notifier,
						 SubscribeServerPluginBase** plugin );

    TiXmlElement* requireElement( const TiXmlElement& parent,
	                              const UtlString& tagName,
	                              OsStatus* err );

	TiXmlText* requireText( const TiXmlElement& elem, OsStatus* err );


};

#endif // PLUGINXMLPARSER_H
