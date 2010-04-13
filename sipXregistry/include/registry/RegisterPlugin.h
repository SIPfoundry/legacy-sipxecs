//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _REGISTERPLUGIN_H_
#define _REGISTERPLUGIN_H_

// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "utl/UtlString.h"
#include "utl/Plugin.h"

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsConfigDb;
class SipMessage;
class SipUserAgent;

/**
 * SIP Registrar Plugin Hook Action.
 *
 * A RegisterPlugin is an action invoked by the SipRegistrarServer whenever a
 *   successful REGISTER request has been processed (that is, after it has
 *   effected its change on the registry database).  The plugin may then take
 *   any action based on the fact that the registration has occured.
 *
 * This class is the abstract base from which all RegisterPlugins must inherit.
 *
 * To configure a RegisterPlugin into the SipRegistrarServer, the registrar-config
 * file should have a directive configuring the plugin library:
 * @code
 * SIP_REGISTRAR_HOOK_LIBRARY.[instance] : [path to libexampleregplugin.so]
 * @endcode
 * Where [instance] is replaced by a unique plugin name, and the value
 * points to the libary that provides the plugin code.
 *
 * In addition to the class derived from this base, a RegisterPlugin library must
 * provide a factory routine named getRegisterPlugin with extern "C" linkage so
 * that the OsSharedLib mechanism can look it up in the dynamically loaded library
 * (looking up C++ symbols is problematic because of name mangling).
 * The factory routine looks like:
 * @code
 * class ExampleRegisterPlugin : public RegisterPlugin
 * {
 *    virtual void takeAction( const SipMessage&   registerMessage ///< the successful registration
 *                            ,const unsigned int  registrationDuration ///< the actual allowed
 *                                                                      /// registration time (note
 *                                                                      /// that this may be < the
 *                                                                      /// requested time).
 *                            ,SipUserAgent*       sipUserAgent     ///< to be used if the plugin
 *                                                                  /// wants to send any SIP msg
 *                            );
 *
 *    friend RegisterPlugin* getRegisterPlugin(const UtlString& name);
 * }
 *
 * extern "C" RegisterPlugin* getRegisterPlugin(const UtlString& instance)
 * {
 *   return new ExampleRegisterPlugin(instance);
 * }
 * @endcode
 *
 * @see Plugin
 */
class RegisterPlugin : public Plugin
{
public:

    /// Take whatever action this register plugin exists to perform.
    virtual void takeAction( const SipMessage&   registerMessage  ///< the successful registration
                            ,const unsigned int  registrationDuration /**< the actual allowed
                                                                       * registration time (note
                                                                       * that this may be < the
                                                                       * requested time). */
                            ,SipUserAgent*       sipUserAgent     /**< to be used if the plugin
                                                                   *   wants to send any SIP msg */
                            ) = 0;
    /**<
     * A plugin may not actually affect the registration itself; it may only take
     * other actions (such as updating presence information) that are triggered
     * by the registration.
     *
     * Plugins are invoked in lexically sorted order based on the name used to identify
     * them in the registrar-config file, but plugin authors are strongly discouraged
     * from making any assumptions based on this ordering.
     */


    static const char* Prefix;  ///< the configuration file prefix = "SIP_REGISTRAR"
    static const char* Factory; ///< the factory routine name = "getRegisterPlugin"

  protected:

    /// Constructor is private so that it is only callable from the subclasses
    RegisterPlugin(const UtlString& instanceName) :
       Plugin(instanceName)
       {
       };

    virtual ~RegisterPlugin()
       {
       };

  private:

    /// There is no copy constructor.
    RegisterPlugin(const RegisterPlugin&);

    /// There is no assignment operator.
    RegisterPlugin& operator=(const RegisterPlugin&);

};

#endif // _REGISTERPLUGIN_H_
