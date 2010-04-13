//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _SipPublishContentMgr_h_
#define _SipPublishContentMgr_h_

// SYSTEM INCLUDES

// APPLICATION INCLUDES

#include <os/OsDefs.h>
#include <os/OsMutex.h>
#include <utl/UtlDefs.h>
#include <utl/UtlHashMap.h>
#include <utl/UtlHashBag.h>
#include <utl/UtlContainableAtomic.h>

// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// FORWARD DECLARATIONS
class HttpBody;
class UtlString;
class SipPublishContentMgrDefaultConstructor;

// TYPEDEFS

/** Class for managing body content to be accepted via PUBLISH or provided in NOTIFY requests
 *
 *  This class is a database that is used to store and retrieve
 *  content (i.e. SIP Event state bodies).  This class does not touch
 *  or process SIP messages.  It is used by other classes and
 *  applications to store and retrieve content related to SIP
 *  SUBSCRIBE, NOTIFY and PUBLISH requests.  The usual usage is to
 *  have one instance that maintains state for an unlimited number of
 *  resources and event types.
 *
 *  The resourceId and eventTypeKey have no semantics.  Syntactically,
 *  they are restricted only by:  (1) resourceId, eventTypeKey, and
 *  eventType may not be the null string, and (2) eventTypeKey may not
 *  contain a byte with the value 1 (control-A), so that the
 *  concatenation of resourceId and eventTypeKey can be split
 *  unambiguously.
 *
 *  It is up to the application or event package to decide what the
 *  resourceId and eventTypeKey look like.  In addition, there is an
 *  eventType that may provide a coarser classification than
 *  eventTypeKey.  Callback functions are registered for eventTypes
 *  rather than eventTypeKeys, which makes it easier to extend the set
 *  of eventTypeKey dynamically.
 *
 *  A suggested convention for the resourceId
 *  is to use:  sip:<userId>@<hostname>[:port] as provided in the
 *  SUBSCRIBE or PUBLISH request URI.  It is suggested that host be
 *  the SIP domainname, not the specific IP address of the host
 *  running the process, and that any URI parameters be omitted.
 *
 *  It is also suggested the SIP event type token be used (without any
 *  event header parameters) as the eventTypeKey.  Only in special
 *  cases where the content varies based upon an event parameter,
 *  should the parameter(s) be include included in the eventTypeKey.
 *  Usually, eventType is the same as eventTypeKey, or is the
 *  SIP event type alone, if eventTypeKey contains parameters.
 *
 * \par Putting In Event Content
 *  Applications put Event content information for a specific resourceId
 *  and eventTypeKey into the SipPublishContentMgr via the publish method.
 *
 * \par Retrieving Event Content
 *  Applications retrieve published content type via the getContent
 *  method.
 *
 * \par Removing Event Content
 *  All event content information for a resource Id and event type key can
 *  can be removed via the unpublish method.
 *
 * \par Default Event Content
 *  It is possible to define default content for an eventTypeKey.
 *  This default content is provided by the getContent method if no
 *  content was provided for the specific resource Id.  Default content
 *  is set via the publishDefault method.
 *
 * \par Default Event Content Constructor
 *  It is possible to define a default content constructor for an
 *  eventTypeKey.
 *  A default content constructor can be provided for an eventTypeKey
 *  by calling another publishDefault method.
 *  When a SUBSCRIBE arrives for an event with no provided content,
 *  the constructor is called to provide content.  The constructor may
 *  choose to provide no content for the resource.
 *
 *  The default content for an eventTypeKey is not expected to change,
 *  and so changes to default content do not trigger callbacks.
 *
 *  When content is searched for, the order of searching is:
 *  - content 
 *    provided by publish()
 *  - default content
 *    provided by publishDefault(..., SipPublishContentMgrDefaultConstructor*)
 *  - default content
 *    provided by publishDefault(..., HttpBody*)
 *
 * \par Full vs. Partial Content
 *  Content is recorded for two flavors, "partial" and "full".
 *  For any one eventType, the application uses full content only, or
 *  it uses both full and partial content for all resources.
 *  If both flavors are used, both flavors for a given resourceId are
 *  expected to be updated together.  (Changes to full content should
 *  be published before changes to partial content to avoid a race
 *  condition.)
 *  The two flavors are stored and retrieved independently, as
 *  specified by parameters to the various methods.
 *
 * \par Content Change Callback
 *  setContentChangeObserver sets a callback function.  It will be
 *  called when the content for a resource changes.  Note that changes
 *  to default content do not trigger callbacks, as default content is
 *  not expected to change during operation.  If both full and partial
 *  content are used, full content updates should specify noNotify =
 *  true, and only partial content updates will cause callbacks.
 *  Note that only one callback can be registered per eventType.
 *
 * \par Content Types
 *  It is expected that the set of content types available for a given
 *  resourceId/eventTypeKey will not change over time, with the exception
 *  of when content is provided for a resourceId that previously had none,
 *  or when content is removed for a recourceId that previously had some.
 *  An implication of this is that if fixed default content is provided,
 *  all recourceId's will have content for that set of content types.
 *  
 */
class SipPublishContentMgr
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
public:

    /** Callback used to notify interested applications when content has changed
     *  Well-behaved applications that register and implement this function
     *  should not block.  They should quickly return as failure to do so
     *  may hinder timely processing and system performance.
     *
     *  /param applicationData - provided with the callback function pointer
     *  when it was registered.
     *  /param resourceId, eventTypeKey, eventType - identify the resourceId
     *  whose content has changed.
     *  /param reason - If content is no longer available for this
     *  resourceId/eventTypeKey, the reason argument of the unpublish
     *  call is passed as the reason parameter.  If content is
     *  available (because the content change was caused by a publish
     *  method, or unpublish "revealed" default content, reason is
     *  NULL.  If reason is not NULL, the previous content has not yet
     *  been removed, and is still accessible via getContent.
     */
    typedef void (*SipPublisherContentChangeCallback) (void* applicationData,
                                                       const char* resourceId,
                                                       const char* eventTypeKey,
                                                       const char* eventType,
                                                       const char* reason);

/* ============================ CREATORS ================================== */

    /// Default publish container constructor
    SipPublishContentMgr();

    /// Destructor
    virtual
    ~SipPublishContentMgr();


/* ============================ MANIPULATORS ============================== */

    /** Provide the default content for the given event type key
     *
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent order for all uses of eventTypeKey in this class.
     *         There is no semantics enforced.  This is an opaque string used
     *         as part of the key.
     *  \param eventType - SIP event type token
     *  \param numContentTypes - the number of bodies (each having a unique
     *         content type) provided in the eventContent array.  Multiple
     *         content types are published if the server wants to deliver
     *         different content types based upon the SUBSCRIBE Accepts
     *         header content types listed.  Must be 1 or greater.
     *  \param eventContent - the SIP Event state content which was provided
     *         via a PUBLISH or requested via a SUBSCRIBE to be
     *         delivered via a NOTIFY.  If multiple bodies are
     *         provided and the content types of the bodies match more
     *         than one of the MIME types provided in the SUBSCRIBE
     *         Accepts header, the order of the bodies in the
     *         eventContent array indicates preference.  The bodies
     *         are NOT copied, but their memory becomes owned by the
     *         SipPublishContentMgr object and will be deleted by it
     *         when they are no longer needed.
     *  \param fullState - if TRUE, this content is the full state for
     *         the event.  If FALSE, it is "partial state", the incremental
     *         changes since the previously published content.
     *         In most uses, there is no value in providing default
     *         content for "partial" state.
     */
    virtual void publishDefault(const char* eventTypeKey,
                                const char* eventType,
                                int numContentTypes,
                                HttpBody* eventContent[],
                                UtlBoolean fullState = TRUE);

    /** Add a default content constructor function.
     *
     *  \param *defaultConstructor becomes owned by the SipPublishContentMgr,
     *         which will delete it when it is no longer needed.
     */
    virtual void publishDefault(const char* eventTypeKey,
                                const char* eventType,
                                SipPublishContentMgrDefaultConstructor*
                                defaultConstructor,
                                UtlBoolean fullState = TRUE);

    /** Remove the default content and default content constructor for
     *  eventTypeKey.
     */
    virtual void unpublishDefault(const char* eventTypeKey,
                                  const char* eventType,
                                  UtlBoolean fullState = TRUE);

    /** Provide the given content for the resource and event type key
     *  An application provides content (i.e. SIP event state bodies)
     *  through this interface for the given resourceId and eventTypeKey.
     *  The resourceId and eventTypeKey together compose a unique key which
     *  identifies the provided content.  The significance of the
     *  resourceId and eventTypeKey is determined by the user code.
     *  In typical usage, the resourceId is the request URI of the
     *  PUBLISH request (to be sent) or the SUBSCRIBE request (to be
     *  responded to), and the eventTypeKey is the SIP Event header
     *  field.
     *  Given the resourceId and eventTypeKey, each particular content
     *  is identified based on the content type of the content (which each
     *  element of eventContent carries within itself) and the full/partial
     *  status (which is given by fullState).
     *  A call to publish() replaces all content types for the given
     *  value of fullState.
     *  \param resourceId - a unique id for the resource, typically the
     *         identity or AOR for the event type content.  There is no
     *         semantics enforced.  This is an opaque string used as part
     *         of the key.
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent order for all uses of eventTypeKey in this class.
     *         There is no semantics enforced.  This is an opaque string used
     *         as part of the key.
     *  \param eventType - SIP event type token
     *  \param numContentTypes - the number of bodies (each having a unique
     *         content type) provided in the eventContent array.  Multiple
     *         content types are published if the server wants to deliver
     *         different content types based upon the SUBSCRIBE Accepts
     *         header content types listed.  Must be 1 or greater.
     *  \param eventContent - the SIP Event state content which is provided
     *         via a PUBLISH or requested via a SUBSCRIBE to be delivered
     *         via a NOTIFY.  If multiple bodies are provided and the content
     *         types match more than one of the MIME types provided in the
     *         SUBSCRIBE Accepts header, the order of the bodies in the
     *         eventContent array indicates a preference.
     *         The bodies are NOT copied, but their memory becomes
     *         owned by the SipPublishContentMgr object and will be
     *         deleted by it when they are no longer needed.
     *  \param fullState - if TRUE, this content is the full state for
     *         the event.  If FALSE, it is "partial state", the incremental
     *         changes since the previously published content.
     *         Note that if partial state is searched for using ::getContent,
     *         then it must be supplied, as ::getContent will not retrieve
     *         full-state content in lieu of missing partial-state content.
     *         Note that full-state changes should be published before
     *         partial-state changes, as otherwise a new subscription between the
     *         two ::publish() calls will see only the previous state.
     *  \param noNotify - if TRUE, do not generate any NOTIFYs for this content
     *         change.  This should only be used in generateDefaultContent
     *         methods, or when publishing full content and partial content
     *         will be published immediately (which will trigger callbacks).
     */
    virtual void publish(const char* resourceId,
                         const char* eventTypeKey,
                         const char* eventType,
                         int numContentTypes,
                         HttpBody* eventContent[],
                         UtlBoolean fullState = TRUE,
                         UtlBoolean noNotify = FALSE);

    /** Remove the content for the given resourceId and eventTypeKey
     *  The content bodies are deleted.
     *  Both the 'partial' and 'full' contents are deleted.
     *  This deletion may cause the resourceId/eventTypeKey to have no
     *  content, or it may reveal default content.
     *  If a default content constructor is provided, it will be
     *  called to see if it generates content.
     *  If after removing content, no (full) content is available for this
     *  resourceId/eventtypeKey, the callback will be called with
     *  the *former* content being visible, and the reason value will
     *  be passed to it.
     *  If default content is available, the callback will be called
     *  with the new content, and the callback will receive a NULL
     *  reason value, as when publish() is called.
     *  The reason value is currently only used by SipSubscribeServer.
     *  See for SipSubscribeServe.h for reason values used to
     *  correctly handle the termination of subscriptions in various
     *  circumstances.)
     *  \param resourceId - a unique id for the resource, typically the
     *         identity or AOR for the event type content.  There is no
     *         semantics enforced.  This is an opaque string used as part
     *         of the key.
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent order for all uses of eventTypeKey in this class.
     *         There is no semantics enforced.  This is an opaque string used
     *         as part of the key.
     *  \param eventType - SIP event type token
     *  \param reason - a (const char*) value that is passed opaquely to
     *         the callback function if there is no remaining content
     *         for this resourceId/eventTypeKey.
     */
    virtual void unpublish(const char* resourceId,
                           const char* eventTypeKey,
                           const char* eventType,
                           const char* reason);

    /** Fetch the published content for a given resourceId/eventTypeKey.
     *  The content body pointers point to copies of the stored
     *  bodies, and the caller is responsible for deleting them.
     *  \param resourceId - a unique id for the resource, or NULL
     *         to retrieve the default content for the eventTypeKey.
     *  \param eventTypeKey - the unique id for the event type.
     *  \param fullState - TRUE for full state content, FALSE for
     *         partial state content
     *  \param numContentTypes - upon return, numContentType is set
     *         to the number of bodies in the eventContent array.
     *         May be 0 if there are no bodies, in which case eventContent
     *         must still be freed.
     *  \param eventContent - will be assigned a pointer to an array of
     *         HttpBody*'s that are the content.  Must be freed using
     *         delete[].
     *  \param pDefaultConstructor - if not NULL and resourceId is NULL,
     *         *pDefaultConstructor is set to point to a copy of the
     *         defaultConstructor for eventTypeKey (if one is set), or
     *         NULL.
     */
    virtual void getPublished(const char* resourceId,
                              const char* eventTypeKey,
                              UtlBoolean fullState,
                              int& numContentTypes,
                              HttpBody**& eventContent,
                              SipPublishContentMgrDefaultConstructor**
                              defaultConstructor);

    /** Get the content for the given resourceId, eventTypeKey and
     *  list of acceptable MIME types.
     *  Retrieves the content type identified by the resourceID and eventTypeKey.
     *  The given contentTypes indicates what content types are acceptable (i.e.
     *  the MIME types from the SUBSCRIBE Accept header).
     *  Returns true if acceptable content was obtained.
     *  \param resourceId - a unique id for the resource, typically the
     *         identity or AOR for the event type content.  There is no
     *         semantics enforced.  This is an opaque string used as part
     *         of the key.
     *  \param eventTypeKey - a unique id for the event type, typically the
     *         SIP Event type token.  Usually this does not contain any of
     *         the SIP Event header parameters.  However it may contain
     *         event header parameters if the parameter identifies different
     *         content.  If event parameters are included, they must be in
     *         a consistent order for all uses of eventTypeKey in this class.
     *         There is no semantics enforced.  This is an opaque string used
     *         as part of the key.
     *  \param fullState - if TRUE, search for full-state content.
     *         If FALSE, search for partial-state content.
     *         Note that full-state content will not be returned if partial-state
     *         content is requested but none is found.
     *  \param acceptHeaderValue - the MIME types allowed to be returned in
     *         the content argument.  The first match is the one returned.
     *         This string has the same syntax/format as the SIP
     *         Accept header value.  Note that if this string is empty,
     *         (corresponding to an Accept header with a null value)
     *         no MIME types are acceptable.  If this string has the special
     *         value acceptAllTypes (corresponding to no Accept header), the
     *         first (sequentially) content body will be selected.
     *         Wildcards in the MIME types are not processed, and
     *         parameters on the content body types are deleted before
     *         comparison.  (This is a major violation of the
     *         standards, but does not cause problems in practice.)
     *  \param content - the content body if a match was found, otherwise NULL.
     *         The content body is a copy that must be freed.
     *  \param isDefaultContent - if there was no content specific to the resourceId
     *         and default content was provided for the given eventTypeKey,
     *         then isDefaultContent is set to TRUE and 'content' contains
     *         values from the default content for eventTypeKey.
     *  \param availableMediaTypes - if no acceptable content was found,
     *         the content types of the content for the resourceId/eventTypeKey
     *         (separated by commas) are written into *avaiableMediaTypes,
     *         if it is not NULL.
     *         Thus, if the method returns false and *availableMediaTypes is
     *         the null string, there is no content available.
     */
    static const UtlString acceptAllTypes;
    virtual UtlBoolean getContent(const char* resourceId,
                                  const char* eventTypeKey,
                                  const char* eventType,
                                  UtlBoolean fullState,
                                  const UtlString& acceptHeaderValue,
                                  HttpBody*& content,
                                  UtlBoolean& isDefaultContent,
                                  UtlString* availableMediaTypes);

    /** Set the callback which gets invoked whenever the content changes
     *  Currently only one observer is allowed per eventTypeKey. Subsequent
     *  calls to setContentChangeObserver will replace it.
     *  NULL can be used to remove the current observer.
     *  Note: The callback is not invoked when the default content changes.
     *  \param eventTypeKey - SIP event type key
     *  \param applicationData - application specific data that is to be
     *         passed back to the application in the callback function.
     */
    virtual void setContentChangeObserver(const char* eventType,
                                                  SipPublisherContentChangeCallback callbackFunction,
                                                  void* applicationData);

    /** Obtain the current observer and its applicatonData for the
     *  given eventTypeKey.
     */
    virtual void getContentChangeObserver(const char* eventType,
                                          SipPublisherContentChangeCallback& callbackFunction,
                                          void*& applicationData);

/* ============================ ACCESSORS ================================= */

    /// Get debugging information.
    void getStats(int& numDefaultContent,
                  int& numDefaultConstructor,
                  int& numResourceSpecificContent,
                  int& numCallbacksRegistered);

/* ============================ INQUIRY =================================== */


   //! Dump the object's internal state.
   void dumpState();
   //! Service function for dumping state.
   void dumpStateBag(UtlHashBag& bag, const char* name);

/* //////////////////////////// PROTECTED ///////////////////////////////// */
protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
private:

    /// Copy constructor NOT ALLOWED
    SipPublishContentMgr(const SipPublishContentMgr& rSipPublishContentMgr);

    /// Assignment operator NOT ALLOWED
    SipPublishContentMgr& operator=(const SipPublishContentMgr& rhs);

    /// lock for single thread use
    void lock();

    /// unlock for use
    void unlock();

    /// The lock itself
    OsMutex mPublishMgrMutex;

    // The following three hash-bags contain PublishContentContainer's
    // which index as strings:
    // Index as strings "resourceId\001eventTypeKey".
    UtlHashBag mContentEntries;
    // Index as strings "resourceId\001eventTypeKey".
    UtlHashBag mPartialContentEntries;
    // Index as strings "\001eventTypeKey".
    UtlHashBag mDefaultContentEntries;
    // Index as strings "\001eventTypeKey".
    UtlHashBag mDefaultPartialContentEntries;

    // Keys are string "eventType", values are
    // SipPublishContentMgrDefaultConstructor's.
    UtlHashMap mDefaultContentConstructors;
    UtlHashMap mDefaultPartialContentConstructors;

    // Members are PublishCallbackContainer's, which index as strings
    // "eventType".
    UtlHashBag mEventContentCallbacks;
};

/**
 * Helper class for SipPublishContentMgr.
 *
 * Each instance is a device for producing default content for a
 * resource/event-type when generateDefaultContent is set but there is no
 * content for the resource/event-type.
 *
 * SipPublicContentMgrDefaultConstructor is pure virtual.  Instances
 * can only be created of subclasses that provide a generateDefaultContent()
 * method.
 */
class SipPublishContentMgrDefaultConstructor : public UtlContainableAtomic
{
  public:

   /** Generate the content for a resource and event.
    *  Called when getContent is called for a resourceId/eventTypeKey
    *  that has no published content.  generateDefaultContent may set
    *  content for that combination, or it can do nothing, which
    *  forces getContent to use the default content (if any) for that
    *  eventTypeKey.  If generateDefaultContent calls
    *  contentMgr->publish(), it must provide noNotify = TRUE, because
    *  the caller will call the callback (if needed).
    */
   void virtual generateDefaultContent(SipPublishContentMgr* contentMgr,
                                       const char* resourceId,
                                       const char* eventTypeKey,
                                       const char* eventType) = 0;

   /// Make a copy of this object according to its real type.
   virtual SipPublishContentMgrDefaultConstructor* copy() = 0;

};

/* ============================ INLINE METHODS ============================ */

#endif  // _SipPublishContentMgr_h_
