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

#ifndef TRANSACTIONPLUGIN_H
#define	TRANSACTIONPLUGIN_H

#include "utl/UtlString.h"
#include "utl/Plugin.h"
#include "net/SipMessage.h"
#include "SipRouter.h"

class TransactionPlugin : public Plugin
{
  public:

   static const char* Prefix;  ///< the configuration file prefix = "SIPX_PROXY"
   static const char* Factory; ///< the factory routine name = "getTransactionPlugin"

   /// destructor
   virtual ~TransactionPlugin() {};
      
  

   /// Read (or re-read) whatever configuration the plugin requires.
   virtual void readConfig( OsConfigDb& configDb /**< a subhash of the individual configuration
                                                  * parameters for this instance of this plugin. */
                           ) = 0;
   
   /// Used to announce the SIP Router instance that is logically associated with this Auth Plugin.
   /// Plugins that need to interact with their associated SIP Router can override this method
   /// and save the passed pointer for later use. 
   virtual void announceAssociatedSipRouter( SipRouter* sipRouter ){};

   /**<
    * @note
    * The parent service may call the readConfig method at any time to
    * indicate that the configuration may have changed.  The plugin
    * should reinitialize itself based on the configuration that exists when
    * this is called.  The fact that it is a subhash means that whatever prefix
    * is used to identify the plugin (see PluginHooks) has been removed (see the
    * examples in PluginHooks::readConfig).
    */
  
  protected:

   /// constructor
   TransactionPlugin(const UtlString& instanceName ///< the configured name for this plugin instance
              ) :
      Plugin(instanceName)
   {
   };
    

  private:
// @cond INCLUDENOCOPY
   
      /// There is no copy constructor.
      TransactionPlugin(const TransactionPlugin& nocopyconstructor);

      /// There is no assignment operator.
      TransactionPlugin& operator=(const TransactionPlugin& noassignmentoperator);
// @endcond INCLUDENOCOPY

};



#endif	/* TRANSACTIONPLUGIN_H */

