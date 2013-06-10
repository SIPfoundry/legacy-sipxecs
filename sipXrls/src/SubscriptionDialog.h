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


#ifndef OSS_SUBSCRIPTIONDIALOG_H_INCLUDED
#define	OSS_SUBSCRIPTIONDIALOG_H_INCLUDED


#include <string>
#include <resip/dum/AppDialog.hxx>
#include <resip/dum/AppDialogSetFactory.hxx>
#include <resip/dum/AppDialogSet.hxx>
#include <resip/dum/DialogUsageManager.hxx>


namespace resip {


class  SubscriptionDialog : public AppDialog
{
public:
  SubscriptionDialog(HandleManager& ham);
};

class SubscriptionDialogSet : public AppDialogSet
{
public:
  SubscriptionDialogSet(DialogUsageManager& dum);
  AppDialog* createAppDialog(const SipMessage& msg);
protected:
  DialogUsageManager& _dum;
  int _dialogCount;
};

class SubscribeAppDialogSetFactory : public AppDialogSetFactory
{
public:
  SubscribeAppDialogSetFactory();
  AppDialogSet* createAppDialogSet(DialogUsageManager& dum, const SipMessage&);
};

class SubscriptionDUM : public DialogUsageManager
{
public:

  SubscriptionDUM(SipStack& stack, bool createDefaultFeatures=false);
};



} // resip



#endif	/// SUBSCRIPTIONDIALOG_H_INCLUDED

