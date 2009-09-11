//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include <assert.h>

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsWriteLock.h"
#include "os/OsSharedLibMgr.h"
#include "xmlparser/tinyxml.h"
#include "statusserver/PluginXmlParser.h"
#include "statusserver/Notifier.h"
#include "statusserver/StatusPluginReference.h"
#include "statusserver/SubscribeServerPluginBase.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS

    // MWI is build in so we do not load a library for now
    // Eventually this should look for a built in named
    // entry point
    SubscribeServerPluginBase*
    MwiPluginFactory(const TiXmlNode& pluginConfigDataNode,
                     Notifier* notifier );

// FUNCTION POINTERS
    SubscribeServerPluginBase* (*PluginFactoryProc)(TiXmlNode&, Notifier* notifier);

// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PluginXmlParser::PluginXmlParser():
    mNotifier(NULL),
    mListMutexR(OsRWMutex::Q_FIFO),
    mListMutexW(OsRWMutex::Q_FIFO)
{}

PluginXmlParser::~PluginXmlParser()
{
    if( mNotifier )
        mNotifier = NULL;
}

///////////////////////////PUBLIC///////////////////////////////////

OsStatus
PluginXmlParser::loadPlugins (
    const UtlString configFileName,
    Notifier* notifier )
{
    OsStatus currentStatus = OS_SUCCESS;
    int index = 0;

    mDoc = new TiXmlDocument( configFileName.data() );

    if( mDoc->LoadFile() )
    {
        OsSysLog::add(FAC_SIP, PRI_DEBUG, "PluginXmlParser::loadMappings "
            "- Loaded %s", configFileName.data() );
    }
    else
    {
        OsSysLog::add(FAC_SIP, PRI_ERR, "PluginXmlParser::loadMappings "
            "- Unable to Open XML file %s", configFileName.data() );

        return OS_NOT_FOUND;
    }

    // start loading plugins from the configuration file
    // Get the "subscribe-server-plugins" element.
    // It is a child of the document, and can be selected by name.
    TiXmlNode* mMainPluginsNode =
        mDoc->FirstChild( XML_TAG_SUBSCRIBE_SERVER_PLUGINS );

    if ( !mMainPluginsNode )
    {
        OsSysLog::add(FAC_SIP, PRI_ERR, "PluginXmlParser::loadMappings "
            "- No child Node for subscribe-server-plugins");

        return OS_FILE_READ_FAILED;
    }

    TiXmlElement* mMainPluginsElement = mMainPluginsNode->ToElement();

    // get <SUBSCRIBE-PLUGIN> Nodes
    TiXmlNode* pluginNode = NULL;
    UtlString eventType;

    while ((pluginNode = mMainPluginsElement->IterateChildren(pluginNode)))
    {
        eventType = "";

        // Skip over comments etc, only interested in ELEMENTS
        if( pluginNode->Type() == TiXmlNode::ELEMENT )
        {
            TiXmlElement* pluginElement = pluginNode->ToElement();
            UtlString value;
            value.append( pluginElement->Value() );
            if( value.compareTo(XML_TAG_SUBSCRIBE_PLUGINS) == 0 )
            {
                index ++;

                TiXmlNode* attibuteNode = NULL;
                SubscribeServerPluginBase* newPlugin = NULL;

                // get event type associated with this plug in
                // create a new plugin for this event type
                attibuteNode = pluginElement->FirstChild(XML_TAG_EVENT_TYPE);
                if(attibuteNode)
                {
                    TiXmlNode* eventTypeValue = attibuteNode->FirstChild();
                    if(eventTypeValue && eventTypeValue->Type() == TiXmlNode::TEXT)
                    {
                        TiXmlText* eventTypeText = eventTypeValue->ToText();
                        if (eventTypeText)
                        {
                            eventType = eventTypeText->Value();

                            // MWI is built in so we do not load a library for now.
                            // Eventually we should remove this if block so it looks
                            // for DLL and entry point
                            if(eventType.compareTo(SIP_EVENT_MESSAGE_SUMMARY,
                                UtlString::ignoreCase ) == 0)
                            {
                                // create new plugin with the specific attributes defined in the node
                                newPlugin = MwiPluginFactory( *pluginNode,
                                                             notifier );
                            }

                            // load DLL and find entry point defined in XML
			    else
                            {
			      // OPINION: make fail-fast; so we fail if one plugin fails
			      currentStatus = loadPlugin(*pluginElement, notifier, &newPlugin);
			      if (currentStatus != OS_SUCCESS)
				return OS_SUCCESS;//:TODO: ???
                            }
                        }
                    }
                }

                if( newPlugin)
                {
                    StatusPluginReference* pluginContainer =
                        new StatusPluginReference(*newPlugin,
                                                  eventType,
                                                  *pluginNode);

                    addPluginToList( pluginContainer );
                }
		else
                {
                    OsSysLog::add(FAC_SIP, PRI_ERR, "PluginXmlParser::loadMappings "
                        "- No Plugin for Node number %d event type: %s",
                        index, eventType.data());
                }
            }
        }
    }
    return currentStatus;
}

OsStatus
PluginXmlParser::loadPlugin (
    TiXmlElement& pluginElement,
    Notifier* notifier,
	SubscribeServerPluginBase** plugin)
{
	OsStatus status = OS_SUCCESS;

	UtlString dllTagName = "load-library";
	TiXmlElement* dllElem = requireElement(pluginElement, dllTagName, &status);
	if (status != OS_SUCCESS)
		return status;
	TiXmlText* dllTxt = requireText(*dllElem, &status);
	if (status != OS_SUCCESS)
		return status;

	// Dynamically load a library
	//OsSharedLibMgr* mgr = OsSharedLibMgr::getOsSharedLibMgr();
	OsSharedLibMgrBase* mgr = OsSharedLibMgr::getOsSharedLibMgr();
	if (!mgr)
		return OS_FAILED;
	OsStatus dllStatus = mgr->loadSharedLib(dllTxt->Value());
	if (dllStatus != OS_SUCCESS)
		return dllStatus;


	// Find the entry point in the library for the factory
	UtlString entryTagName = "plugin-factory";
	TiXmlElement* entryElem = requireElement(pluginElement, entryTagName, &status);
	if (status != OS_SUCCESS)
		return status;
	TiXmlText* entryTxt = requireText(*entryElem, &status);
	if (status != OS_SUCCESS)
		return status;

	OsStatus entryStatus = mgr->getSharedLibSymbol(dllTxt->Value(), entryTxt->Value(),
		    (void*&)PluginFactoryProc);

	if (entryStatus != OS_SUCCESS)
		return entryStatus;

	// Call the factory to construct the plugin
	*plugin = PluginFactoryProc(pluginElement, notifier);
	if (*plugin == NULL)
	{
		OsSysLog::add(FAC_SIP, PRI_ERR, "PluginXmlParser::loadPlugin return null "
	        "SubscribeServerPluginBasemissing %s ", entryTxt->Value());
		return OS_FAILED;
	}


    return OS_SUCCESS;
}

TiXmlElement*
PluginXmlParser::requireElement (
    const TiXmlElement& parent,
	const UtlString& tagName,
	OsStatus* err)
{
	TiXmlNode* n = (TiXmlNode*)parent.FirstChild(tagName.data());
	if (!n)
	{
		OsSysLog::add(FAC_SIP, PRI_ERR, "PluginXmlParser::requiredNode missing %s ",
                        tagName.data());
		*err = OS_FAILED;
	}

	TiXmlElement* e = n->ToElement();
	if (!e)
	{
		OsSysLog::add(FAC_SIP, PRI_ERR, "PluginXmlParser::requiredNode is not an element %s ",
                        tagName.data());
		*err = OS_FAILED;
	}

	return e;
}

TiXmlText*
PluginXmlParser::requireText (
    const TiXmlElement& elem,
	OsStatus* err)
{
	TiXmlNode* tn = (TiXmlNode*)elem.FirstChild();
	if (tn != NULL && tn->Type() == TiXmlNode::TEXT)
	{
		TiXmlText* t = tn->ToText();
		if (t->Value() != NULL)
		{
			return t;
		}
	}

	OsSysLog::add(FAC_SIP, PRI_ERR, "PluginXmlParser::requiredText "
		 "missing element text body %s ", elem.Value());
	*err = OS_FAILED;

	return NULL;
}

int PluginXmlParser::getListSize()
{
    OsWriteLock writeLock(mListMutexW);

    return mPluginTable.entries();
}

StatusPluginReference*
PluginXmlParser::getPlugin( const UtlString& eventType )
{
    UtlString matchEventType(eventType);
    StatusPluginReference* pluginContainer;
    {
      OsWriteLock writeLock(mListMutexW);
      pluginContainer = (StatusPluginReference*) mPluginTable.find(&matchEventType);
    }

    if (!pluginContainer)
    {
        OsSysLog::add(FAC_SIP, PRI_WARNING, "PluginXmlParser::getPlugin "
            "eventType '%s' not found", eventType.data() );
    }

    return pluginContainer;
}
///////////////////////////PRIVATE///////////////////////////////////
void
PluginXmlParser::addPluginToList(StatusPluginReference* pluginContainer)
{
    OsWriteLock writeLock(mListMutexW);

    mPluginTable.insert(pluginContainer);
}
