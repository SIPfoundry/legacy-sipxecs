//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////
#ifndef _SHAREDSECRET_H_
#define _SHAREDSECRET_H_

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlString.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS
class OsConfigDb;

/// Reads a shared secret value from the domain configuration.
/**
 * To use the shared secret from the configuration:
 * @code
 *
 * // read the domain configuration
 * OsConfigDb domainConfiguration;
 * domainConfiguration.loadFromFile(SipXecsService::domainConfigPath());
 *
 * // get the shared secret for generating signatures
 * SharedSecret signingSecret(domainConfiguration);
 *
 * // generate a hash that signs the (existing) value
 * NetMd5Codec signature;
 *
 * signature.hash(signingSecret);
 * signature.hash(value);
 *
 * UtlString signedValue(value);
 * signature.appendHashValue(signedValue);
 *
 * @endcode
 *
 * @see SipXecsService::domainConfigPath
 * @see OsConfigDb::loadFromFile
 * @see NetMd5Codec
 */
class SharedSecret : public UtlString
{
  public:

   /// constructor
   SharedSecret(OsConfigDb& domainConfigDb);
   /**< @note This constructor asserts if there is not a valid secret in the configuration file */

   /// destructor
   virtual ~SharedSecret();

  private:

   // @cond INCLUDENOCOPY
   /// There is no copy constructor.
   SharedSecret(const SharedSecret& nocopyconstructor);

   /// There is no assignment operator.
   SharedSecret& operator=(const SharedSecret& noassignmentoperator);
   // @endcond
};

#endif // _SHAREDSECRET_H_
