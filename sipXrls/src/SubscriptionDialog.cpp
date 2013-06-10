
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


#include "SubscriptionDialog.h"


namespace resip {


SubscriptionDialog::SubscriptionDialog(HandleManager& ham)  :
    AppDialog(ham)
{
}


SubscriptionDialogSet::SubscriptionDialogSet(DialogUsageManager& dum) :
  AppDialogSet(dum),
   _dum(dum),
  _dialogCount(0)
{
}

AppDialog* SubscriptionDialogSet::createAppDialog(const SipMessage& msg)
{
  _dialogCount++;
  if (_dialogCount > 1)
  {
    //
    // we got a fork
    //
  }
  return new SubscriptionDialog(_dum);
}

SubscribeAppDialogSetFactory::SubscribeAppDialogSetFactory()
{
}

AppDialogSet* SubscribeAppDialogSetFactory::createAppDialogSet(DialogUsageManager& dum, const SipMessage&)
{
  return new SubscriptionDialogSet(dum);
}

SubscriptionDUM::SubscriptionDUM(SipStack& stack, bool createDefaultFeatures) :
  DialogUsageManager(stack, createDefaultFeatures)
{
}


} // resip





