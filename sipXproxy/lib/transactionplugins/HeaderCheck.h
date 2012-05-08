/**
 *
 *
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */

#ifndef HEADERCHECK_H
#define	HEADERCHECK_H




// SYSTEM INCLUDES
// APPLICATION INCLUDES
#include "TransactionPlugin.h"
#include "net/SipOutputProcessor.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class HeaderCheckTest;

extern "C" TransactionPlugin* getTransactionPlugin(const UtlString& name);

/**
 * The purpose of this transaction plugin is to remove the call destination information
 * from an INVITE request header and copy/append it to the Record-Route.
 *
 */
class HeaderCheck : public TransactionPlugin, SipOutputProcessor
{
  public:

   /// destructor
   virtual ~HeaderCheck();

   /// Called when SIP messages are about to be sent by proxy
   virtual void handleOutputMessage( SipMessage& message,
                                     const char* address,
                                     int port );



   /// Read (or re-read) the transactionorization rules.
   virtual void readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                                  * parameters for this instance of this plugin. */
                           );
   /**<
    * @note
    * The parent service may call the readConfig method at any time to
    * indicate that the configuration may have changed.  The plugin
    * should reinitialize itself based on the configuration that exists when
    * this is called.  The fact that it is a subhash means that whatever prefix
    * is used to identify the plugin (see PluginHooks) has been removed (see the
    * examples in PluginHooks::readConfig).
    */

   virtual void announceAssociatedSipRouter( SipRouter* sipRouter );

   static SipRouter* _sipRouter; ///< stores pointer to owning SipRouter.
   
  protected:
  private:
   friend class HeaderCheckTest;
   friend TransactionPlugin* getTransactionPlugin(const UtlString& name);

   /// Constructor - private so that only the factory can call it.
   HeaderCheck(const UtlString& instanceName ///< the configured name for this plugin instance
                    );

// @cond INCLUDENOCOPY

   /// There is no copy constructor.
   HeaderCheck(const HeaderCheck& nocopyconstructor);

   /// There is no assignment operator.
   HeaderCheck& operator=(const HeaderCheck& noassignmentoperator);
// @endcond INCLUDENOCOPY
};




#endif	/* HEADERCHECK_H */

