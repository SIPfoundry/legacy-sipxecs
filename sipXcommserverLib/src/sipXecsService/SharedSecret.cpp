//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "sipXecsService/SipXecsService.h"
#include "sipXecsService/SharedSecret.h"

#include "os/OsConfigDb.h"
#include "os/OsSysLog.h"

#include "net/NetBase64Codec.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
SharedSecret::SharedSecret(OsConfigDb& domainConfigDb)
{
   UtlString base64secret;

   if (OS_SUCCESS == domainConfigDb.get(SipXecsService::DomainDbKey::SHARED_SECRET, base64secret))
   {
      if (   !NetBase64Codec::decode(base64secret, *this)
          || isNull()
          )
      {
         OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                       "SharedSecret::_ invalid value '%s' for '%s' found in '%s'; aborting",
                       base64secret.data(),
                       SipXecsService::DomainDbKey::SHARED_SECRET,
                       domainConfigDb.getIdentityLabel());

         // We assume that if the component wants a signing secret for some security-critical
         // purpose.  Rather than continue without this security-critical data, stop.
         assert(false);
      }
      else
      {
         OsSysLog::add(FAC_KERNEL, PRI_DEBUG,
                       "SharedSecret::_ loaded from '%s' length %zu",
                       domainConfigDb.getIdentityLabel(), length());
      }
   }
   else
   {
      OsSysLog::add(FAC_KERNEL, PRI_CRIT,
                    "SharedSecret::_ no value for '%s' found in '%s'; using fixed value",
                    SipXecsService::DomainDbKey::SHARED_SECRET, domainConfigDb.getIdentityLabel());

      // We assume that if the component wants a signing secret for some security-critical
      // purpose.  Rather than continue without this security-critical data, stop.
      // @TODO assert(false);
   }
};





/// destructor
SharedSecret::~SharedSecret()
{
};
