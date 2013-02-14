//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _ResourceNotifyReceiver_h_
#define _ResourceNotifyReceiver_h_

// SYSTEM INCLUDES
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
// APPLICATION INCLUDES

#include <net/SipMessage.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS


//! Abstract class for processing NOTIFY callbacks within a ResourceListSet.
/** When a NOTIFY callback is received by a ResourceListSet, once the thread
 *  obtains the lock on the ResourceListSet, it looks up the dialog handle
 *  in mNotifyMap to find the ResourceNotifyReceiver responsible for the
 *  subscription, and calls its notifyEventCallback method.
 */
class ResourceNotifyReceiver
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:
    class CallBack : boost::noncopyable
    {
    public:

      typedef boost::mutex mutex;
      typedef boost::lock_guard<mutex> mutex_lock;
      typedef boost::shared_ptr<CallBack> Ptr;

      void notifyEventCallback(const UtlString* dialogHandle, const UtlString* content);

      void invalidateCallBack();

      ResourceNotifyReceiver* receiver() const;
    protected:
      CallBack(ResourceNotifyReceiver* receiver);

    private:
      mutex _mutex;
      ResourceNotifyReceiver* _receiver;
      bool _isValid;
      friend class ResourceNotifyReceiver;
    };

   //! Construct an instance
   ResourceNotifyReceiver();

   //! Destructor
   virtual ~ResourceNotifyReceiver();

   /** Handle a NOTIFY callback for the subscription handled by this
    *  ResourceNotifyReceiver.
    */
   virtual void notifyEventCallback(const UtlString* dialogHandle,
                                    const UtlString* content) = 0;

   CallBack::Ptr getSafeCallBack() const;


/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

    CallBack::Ptr _safeCallBack;

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   //! Disabled copy constructor
   ResourceNotifyReceiver(const ResourceNotifyReceiver& rResourceNotifyReceiver);

   //! Disabled assignment operator
   ResourceNotifyReceiver& operator=(const ResourceNotifyReceiver& rhs);
};


//
// Inlines
//
inline ResourceNotifyReceiver::CallBack::Ptr ResourceNotifyReceiver::getSafeCallBack() const
{
  return _safeCallBack;
}

inline ResourceNotifyReceiver::CallBack::CallBack(ResourceNotifyReceiver* receiver) :
  _receiver(receiver),
  _isValid(true)
{
}

inline void ResourceNotifyReceiver::CallBack::notifyEventCallback(
  const UtlString* dialogHandle, const UtlString* content
)
{
  mutex_lock lock(_mutex);
  if (_isValid && _receiver)
    _receiver->notifyEventCallback(dialogHandle, content);
}

inline void ResourceNotifyReceiver::CallBack::invalidateCallBack()
{
  mutex_lock lock(_mutex);
  _isValid = false;
  _receiver = 0;
}

inline ResourceNotifyReceiver* ResourceNotifyReceiver::CallBack::receiver() const
{
  return _receiver;
}

#endif  // _ResourceNotifyReceiver_h_
