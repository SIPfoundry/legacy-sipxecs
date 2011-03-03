//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES
#include "os/OsConfigDb.h"
#include "os/OsFS.h"
#include "os/OsReadLock.h"
#include "os/OsWriteLock.h"
#include "os/OsSysLog.h"
#include "sipXecsService/SipXecsService.h"

// APPLICATION INCLUDES
#include "alarm/Alarm.h"
#include "EmergencyNotify.h"

// DEFINES
// CONSTANTS

const char DEFAULT_EMERG_RULES_FILENAME[] = "authrules.xml";
const char EMERGRULES_FILENAME_CONFIG_PARAM[] = "EMERGRULES";

// TYPEDEFS
// FORWARD DECLARATIONS

/// Factory used by PluginHooks to dynamically link the plugin instance
extern "C" AuthPlugin* getAuthPlugin(const UtlString& pluginName)
{
   return new EmergencyNotify(pluginName);
}

/// constructor
EmergencyNotify::EmergencyNotify(const UtlString& pluginName ///< the name for this instance
                                   )
   : AuthPlugin(pluginName)
   , mRulesLock(OsRWMutex::Q_FIFO)
   , mpEmergencyRules(NULL)
{
   OsSysLog::add(FAC_SIP,PRI_INFO,"EmergencyNotify plugin instantiated '%s'",
                 mInstanceName.data());
};

/// Read (or re-read) the authorization rules.
void
EmergencyNotify::readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                                    * parameters for this instance of this plugin. */
                             )
{
   /*
    * @note
    * The parent service may call the readConfig method at any time to
    * indicate that the configuration may have changed.  The plugin
    * should reinitialize itself based on the configuration that exists when
    * this is called.  The fact that it is a subhash means that whatever prefix
    * is used to identify the plugin (see PluginHooks) has been removed (see the
    * examples in PluginHooks::readConfig).
    */
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "EmergencyNotify[%s]::readConfig",
                 mInstanceName.data()
                 );

   OsWriteLock writeLock(mRulesLock);

   if (mpEmergencyRules)
   {
      delete mpEmergencyRules;
   }

   mpEmergencyRules = new EmergencyRulesUrlMapping();

   UtlString fileName;
   if ( ! configDb.get(EMERGRULES_FILENAME_CONFIG_PARAM, fileName) )
   {
      OsPath defaultPath =
         SipXecsService::Path(SipXecsService::ConfigurationDirType, DEFAULT_EMERG_RULES_FILENAME);

      fileName = SipXecsService::Path(SipXecsService::ConfigurationDirType, DEFAULT_EMERG_RULES_FILENAME);

      OsSysLog::add(FAC_SIP, PRI_DEBUG, "EmergencyNotify[%s]::readConfig "
                    " no rules file configured; trying '%s'",
                    mInstanceName.data(), fileName.data()
                    );
   }

   if (OS_SUCCESS == mpEmergencyRules->loadMappings(fileName))
   {
      OsSysLog::add(FAC_SIP, PRI_INFO, "EmergencyNotify[%s]::readConfig "
                    " successfully loaded rules file '%s'.",
                    mInstanceName.data(), fileName.data()
                    );
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_ERR, "EmergencyNotify[%s]::readConfig "
                    " error loading rules file '%s'.",
                    mInstanceName.data(), fileName.data()
                    );
      delete mpEmergencyRules;
      mpEmergencyRules = NULL;
   }
}

AuthPlugin::AuthResult
EmergencyNotify::authorizeAndModify(const UtlString& id, /**< The authenticated identity of the
                                                          *   request originator, if any (the null
                                                          *   string if not).
                                                          *   This is in the form of a SIP uri
                                                          *   identity value as used in the
                                                          *   credentials database (user@domain)
                                                          *   without the scheme or any parameters.
                                                          */
                                    const Url&  requestUri, ///< parsed target Uri
                                    RouteState& routeState, ///< the state for this request.
                                    const UtlString& method,///< the request method
                                    AuthResult  priorResult,///< results from earlier plugins.
                                    SipMessage& request,    ///< see AuthPlugin wrt modifying
                                    bool bSpiralingRequest,
                                    UtlString&  reason      ///< rejection reason
                                    )
{
   OsReadLock readLock(mRulesLock);

   if (mpEmergencyRules)
   {
      UtlString nameStr;
      UtlString descriptionStr;

      if (mpEmergencyRules->getMatchedRule(requestUri, nameStr, descriptionStr))
      {
         UtlString fromField;
         UtlString fromLabel;
         UtlString contactField;
         request.getFromUri(&fromField);
         request.getFromLabel(&fromLabel);
         request.getContactUri(0, &contactField);
         UtlSList alarmParams;
         alarmParams.append(&nameStr);
         alarmParams.append(&descriptionStr);
         alarmParams.append(&fromLabel);
         alarmParams.append(&fromField);
         alarmParams.append(&contactField);

         OsSysLog::add(FAC_ALARM, PRI_EMERG,
               "Emergency dial rule '%s (%s)' was invoked by '%s<%s>' Contact: %s",
               nameStr.data(), descriptionStr.data(), fromLabel.data(), fromField.data(),
               contactField.data());
         Alarm::raiseAlarm("EMERG_NUMBER_DIALED", alarmParams);
      }
   }

   return CONTINUE;
}

// convenience function for tester
bool EmergencyNotify::getMatchedRule(const Url& requestUri,///< target to check
                               UtlString& rNameStr,        ///< name of the rule that matched
                               UtlString& rDescriptionStr  ///< description of the rule that matched
                               ) const
{
   bool rc=false;
   if ( mpEmergencyRules )
   {
      rc = mpEmergencyRules->getMatchedRule(requestUri, rNameStr, rDescriptionStr);
   }
   return rc;
}

/// destructor
EmergencyNotify::~EmergencyNotify()
{
}
