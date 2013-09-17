/*
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

#ifndef CUSTOMEXCEPTIONHANDLERS_H_
#define CUSTOMEXCEPTIONHANDLERS_H_

#include <os/OsExceptionHandler.h>

// custom behavior was to exit, custom one is to abort
inline void customMongoSocketExceptionHandling(std::exception& e)
{
  catch_global_print(static_cast<mongo::DBException&>(e).toString().c_str());
  std::abort();
}

//custom mongo connect exception handling : log & abort
inline void customMongoConnectExceptionHandling(std::exception& e)
{
  catch_global_print(static_cast<mongo::DBException&>(e).toString().c_str());
  std::abort();
}

#endif /* CUSTOMEXCEPTIONHANDLERS_H_ */
