//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceSubscriptionReceiver_h_
#define _ResourceSubscriptionReceiver_h_

// SYSTEM INCLUDES
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
// APPLICATION INCLUDES

#include <net/SipMessage.h>
#include <net/SipSubscribeClient.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


//! Abstract class for processing subscription state change callbacks within a ResourceListSet.
/** When a subscription state callback is received by a ResourceListSet,
 *  once the thread obtains the lock on the ResourceListSet, it looks
 *  up the dialog handle in mSubscriptionMap to find the
 *  ResourceSubscriptionReceiver responsible for the subscription, and
 *  calls its subscriptionEventCallBack method.
 */
class ResourceSubscriptionReceiver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

    class CallBack : boost::noncopyable
    {
    public:

      typedef boost::mutex mutex;
      typedef boost::lock_guard<mutex> mutex_lock;
      typedef boost::shared_ptr<CallBack> Ptr;

      void subscriptionEventCallback(
        const UtlString* earlyDialogHandle,
        const UtlString* dialogHandle,
        SipSubscribeClient::SubscriptionState newState,
        const UtlString* subscriptionState
      );

      void invalidateCallBack();
    protected:
      CallBack(ResourceSubscriptionReceiver* receiver);

    private:
      mutex _mutex;
      ResourceSubscriptionReceiver* _receiver;
      bool _isValid;
      friend class ResourceSubscriptionReceiver;
    };

   //! Construct an instance
   ResourceSubscriptionReceiver();

   //! Destructor
   virtual ~ResourceSubscriptionReceiver();

   /** Handle a subscription state callback for the subscription
    *  handled by this ResourceSubscriptionReceiver.
    *  Overridden by every subclass.
    */
   virtual void subscriptionEventCallback(
      const UtlString* earlyDialogHandle,
      const UtlString* dialogHandle,
      SipSubscribeClient::SubscriptionState newState,
      const UtlString* subscriptionState
      ) = 0;

   CallBack::Ptr getSafeCallBack() const;
/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

    CallBack::Ptr _safeCallBack;
/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Disabled copy constructor
   ResourceSubscriptionReceiver(const ResourceSubscriptionReceiver& rResourceSubscriptionReceiver);

   //! Disabled assignment operator
   ResourceSubscriptionReceiver& operator=(const ResourceSubscriptionReceiver& rhs);

};

//
// Inlines
//
inline ResourceSubscriptionReceiver::CallBack::Ptr ResourceSubscriptionReceiver::getSafeCallBack() const
{
  return _safeCallBack;
}

inline ResourceSubscriptionReceiver::CallBack::CallBack(ResourceSubscriptionReceiver* receiver) :
  _receiver(receiver),
  _isValid(true)
{
}

inline void ResourceSubscriptionReceiver::CallBack::subscriptionEventCallback(
  const UtlString* earlyDialogHandle,
  const UtlString* dialogHandle,
  SipSubscribeClient::SubscriptionState newState,
  const UtlString* subscriptionState
)
{
  mutex_lock lock(_mutex);
  if (_isValid && _receiver)
    _receiver->subscriptionEventCallback(earlyDialogHandle, dialogHandle, newState, subscriptionState);
}

inline void ResourceSubscriptionReceiver::CallBack::invalidateCallBack()
{
  mutex_lock lock(_mutex);
  _isValid = false;
  _receiver = 0;
}

#endif  // _ResourceSubscriptionReceiver_h_
