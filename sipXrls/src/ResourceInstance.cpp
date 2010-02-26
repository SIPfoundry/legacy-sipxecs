//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#include <string.h>

// APPLICATION INCLUDES

#include "ResourceListServer.h"
#include "ResourceInstance.h"
#include <os/OsSysLog.h>
#include <utl/XmlContent.h>
#include <utl/UtlHashMapIterator.h>
#include <xmlparser/tinystr.h>
#include <xmlparser/TiXmlUtlStringWriter.h>
#include <xmlparser/ExtractContent.h>

// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS

// The string that appears in the Call-Id of all OPTIONS requests generated
// by NAT Keepalive, and used to detect if a phone is invalidly reporting
// such an OPTIONS request as a dialog.
#define NAT_KEEPALIVE_SIGNATURE "-reniatniamtan"
// Define this symbol to enable detection of invalidly reported OPTIONS.
#define NAT_KEEPALIVE_DETECT

// STATIC VARIABLE INITIALIZATIONS
const UtlContainableType ResourceInstance::TYPE = "ResourceInstance";


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
ResourceInstance::ResourceInstance(SubscriptionSet* parent,
                                   const char* instanceName,
                                   const char* subscriptionState) :
   mSubscriptionSet(parent),
   mInstanceName(instanceName),
   mSubscriptionState(subscriptionState),
   mContentPresent(FALSE)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceInstance:: mInstanceName = '%s', mSubscriptionSet = '%s', subscriptionState = '%s'",
                 mInstanceName.data(), mSubscriptionSet->getUri()->data(),
                 subscriptionState);

   // Add this ResourceInstance to mNotifyMap.
   getResourceListSet()->addNotifyMapping(mInstanceName, this);

   // Publish the state with the new instance.
   getResourceCached()->setToBePublished();
}

// Destructor
ResourceInstance::~ResourceInstance()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceInstance::~ mInstanceName = '%s'",
                 mInstanceName.data());

   // Delete this ResourceInstance from mNotifyMap.
   getResourceListSet()->deleteNotifyMapping(&mInstanceName);

   // Terminate the subscription for this resource instance.
   UtlBoolean ret;
   ret = getResourceListServer()->getSubscribeClient().
      endSubscriptionGroup(mInstanceName.data());
   OsSysLog::add(FAC_RLS,
                 ret ? PRI_DEBUG : PRI_WARNING,
                 "ResourceInstance::~ endSubscriptionGroup %s mInstanceName = '%s'",
                 ret ? "succeeded" : "failed",
                 mInstanceName.data());

   // Delete the XML in mXmlDialogs.
   destroyXmlDialogs();

   mContentPresent = FALSE;

   // This destructor does not mark any resource lists for publication,
   // as the termination of this instance has been published previously,
   // and there is no need to publish this instance's vanishing quickly.
}

/* ============================ MANIPULATORS ============================== */

// Return TRUE if the subscription state is "terminated".
UtlBoolean ResourceInstance::isSubscriptionStateTerminated()
{
   // The subscription is terminated if the subscription state string
   // starts with "terminated";
   UtlBoolean ret = strncmp(mSubscriptionState.data(),
                            "terminated", sizeof ("terminated") - 1) == 0;
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceInstance::isSubscriptionStateTerminated mInstanceName = '%s', mSubscriptionState = '%s', ret = %d",
                 mInstanceName.data(), mSubscriptionState.data(), ret);
   return ret;
}

// Process a notify event callback.
void ResourceInstance::notifyEventCallback(const UtlString* dialogHandle,
                                           const UtlString* content)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceInstance::notifyEventCallback mInstanceName = '%s', content = '%s'",
                 mInstanceName.data(), content->data());
   // Set to true if we find publishable data.
   bool publish = false;

   // Set the subscription state to "active".
   mSubscriptionState = "active";

   // Save the content as text for the RFC 4662 resource list events.
   mContent.remove(0);
   mContent.append(*content);
   mContentPresent = TRUE;

   // Dissect the XML for each dialog event and store it in a map
   // so we can construct BroadWorks-style resource list events
   // (which have to have full state).

   // Initialize Tiny XML document object.
   TiXmlDocument xmlDialogEvent;
   TiXmlNode* dialog_info_node;
   if (
       // Load the XML into it.
       xmlDialogEvent.Parse(mContent.data()) &&
       // Find the top element, which should be a <dialog-info>.
       (dialog_info_node = xmlDialogEvent.FirstChild("dialog-info")) != NULL &&
       dialog_info_node->Type() == TiXmlNode::ELEMENT)
   {
      // Check the state attribute.
      const char* p = dialog_info_node->ToElement()->Attribute("state");
      if (p && strcmp(p, "full") == 0)
      {
         // If the state is "full", terminate all non-terminated dialogs.  (XECS-1668)
         terminateXmlDialogs();
         publish = true;
         OsSysLog::add(FAC_RLS, PRI_DEBUG,
                       "ResourceInstance::notifyEventCallback all non-terminated dialogs");
      }

      // Find all the <dialog> elements.
      for (TiXmlNode* dialog_node = 0;
           (dialog_node = dialog_info_node->IterateChildren("dialog",
                                                            dialog_node));
         )
      {
         if (dialog_node->Type() == TiXmlNode::ELEMENT)
         {
            TiXmlElement* dialog_element = dialog_node->ToElement();

            // Determine if the <dialog> is a bogus report of a NAT Keepalive
            // OPTIONS message, as reported by Polycom SPIP firmware 3.1.2.
            // (XTRN-425)  If so, ignore it.

#ifdef NAT_KEEPALIVE_DETECT
            const char* call_id_attr = dialog_element->Attribute("call-id");
            // Reject <dialog>s on the narrowest grounds, that is, only if the
            // call-id attribute is present and contains NAT_KEEPALIVE_SIGNATURE.
            const bool ok =
               !(call_id_attr &&
                 strstr(call_id_attr,
                        NAT_KEEPALIVE_SIGNATURE) != NULL);
#else
            const bool ok = true;
#endif
            if (ok)
            {
               // Now that we've got a <dialog> element, edit it to fit
               // into a consolidated event notice.
               publish = true;

               // Prepend the resource instance name to the 'id'
               // attribute, so it is unique within the <resource>.
               UtlString id(mInstanceName);
               // mInstanceName is guaranteed to not contain ';', because
               // it is a dialog handle that we generate by concatenating
               // the Call-Id and tags using ',' as a separator.  And ';'
               // may not appear in Call-Ids or tags.
               id.append(";");
               id.append(dialog_element->Attribute("id"));
               dialog_element->SetAttribute("id", id.data());

               // Prepare the display name, so we can insert it easily
               // when we generate consolidated events.
               // Find or add the <local> element.
               TiXmlNode* local = dialog_element->FirstChild("local");
               if (!local)
               {
                  local = dialog_element->LinkEndChild(new TiXmlElement("local"));
               }
               // Find or add the <local><identity> element.
               TiXmlNode* identity = local->FirstChild("identity");
               if (!identity)
               {
                  identity =
                     local->LinkEndChild(new TiXmlElement("identity"));
               }
               // Clear the display attribute.
               identity->ToElement()->SetAttribute("display", "");

               // Put the resource URI as the content of the
               // <local><identity> element.
               // First, remove all text children.
               TiXmlNode* child;
               for (TiXmlNode* prev_child = 0;
                    (child = identity->IterateChildren(prev_child));
                  )
               {
                  if (child->Type() == TiXmlNode::TEXT)
                  {
                     identity->RemoveChild(child);
                     // Leave prev_child unchanged.
                  }
                  else
                  {
                     prev_child = child;
                  }
               }
               // Insert a text child containing the URI.
               identity->LinkEndChild(new TiXmlText(getResourceCached()->
                                                    getUri()->data()));

               // Now that we have the XML all nice and pretty, store a copy of
               // it in mXmlDialogs.
               // Clone the XML and create a UtlVoidPtr to wrap it.
               TiXmlElement* alloc_xml = dialog_element->Clone()->ToElement();

               // Look for an earlier version of this dialog in the hash map.
               UtlVoidPtr* p = dynamic_cast <UtlVoidPtr*> (mXmlDialogs.findValue(&id));
               if (p)
               {
                  // Replace the old XML with new XML.
                  delete static_cast <TiXmlElement*> (p->getValue());
                  p->setValue(alloc_xml);
                  OsSysLog::add(FAC_RLS, PRI_DEBUG,
                                "ResourceInstance::notifyEventCallback replaced dialog with id '%s'",
                                id.data());
               }
               else
               {
                  // Check that we don't have too many dialogs.
                  if (mXmlDialogs.entries() <
                      getResourceListServer()->getMaxDialogsInResInst())
                  {
                     mXmlDialogs.insertKeyAndValue(new UtlString(id),
                                                   new UtlVoidPtr(alloc_xml));
                     OsSysLog::add(FAC_RLS, PRI_DEBUG,
                                   "ResourceInstance::notifyEventCallback added dialog with id '%s'",
                                   id.data());
                  }
                  else
                  {
                     // Free alloc_xml, because we aren't saving a pointer to it.
                     delete alloc_xml;
                     OsSysLog::add(FAC_RLS, PRI_ERR,
                                   "ResourceInstance::notifyEventCallback cannot add dialog with id '%s', already %zu in ResourceInstance '%s'",                                id.data(), mXmlDialogs.entries(),
                                   mInstanceName.data());
                  }
               }
            }
            else
            {
               // The <dialog> was rejected because it appears to report
               // a NAT Maintainer OPTIONS message.
               // We log this at DEBUG level because if these appear,
               // there is likely to be one every 20 seconds.
               OsSysLog::add(FAC_RLS, PRI_DEBUG,
                             "ResourceInstance::notifyEventCallback "
                             "ignored <dialog> reporting a NAT Keepalive message "
                             "in subscription dialog handle '%s' - "
                             "see XTRN-426",
                             mInstanceName.data());
            }
         }
      }
   }
   else
   {
      // Report error parsing XML.
      OsSysLog::add(FAC_RLS, PRI_ERR,
                    "ResourceInstance::notifyEventCallback "
                    "Dialog event from '%s' not parsable.",
                    getResourceCached()->getUri()->data());
      OsSysLog::add(FAC_RLS, PRI_INFO,
                    "ResourceInstance::notifyEventCallback "
                    "Dialog event content is '%s'",
                    content->data());
      // Throw away the content, since we cannot generate matching
      // consolidated content.
      mContentPresent = FALSE;
      mContent.remove(0);
      destroyXmlDialogs();
   }

   // Get the change published, if we found <dialog> that was not incorrect.
   if (publish)
   {
      getResourceCached()->setToBePublished(FALSE, getResourceCached()->getUri());
   }
}

// Process a termination of the subscription of this ResourceInstance.
void ResourceInstance::terminateContent(const char* subscriptionState)
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceInstance::terminateContent mInstanceName = '%s', subscriptionState = '%s'",
                 mInstanceName.data(), subscriptionState);

   // Remove the content.
   mContent.remove(0);
   mContentPresent = FALSE;
   destroyXmlDialogs();

   // Update the subscription state.
   mSubscriptionState = subscriptionState;
   // Tell the ResourceCached to generate and publish new content.
   // The resource will be deleted the next time
   // ResourceCache::purgeTerminated() is called.
   getResourceCached()->setToBePublished();
}

// Remove any dialogs in the terminated state.
// This operates only on the parsed dialogs in mXmlDialogs, used to generate
// the consolidated state.  The RFC 4662 events are not affected, but
// presumably the resource is not allowing terminated dialogs to accumulate
// in the events it sends.
void ResourceInstance::purgeTerminatedDialogs()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceInstance::purgeTerminatedDialogs mInstanceName = '%s'",
                 mInstanceName.data());

   // Iterate through all the <dialog> elements.
   UtlHashMapIterator itor(mXmlDialogs);
   UtlContainable* id;
   while ((id = itor()))
   {
      // Get the <state> element content.
      UtlVoidPtr* p = dynamic_cast <UtlVoidPtr*> (itor.value());
      TiXmlElement* dialog_element =
         static_cast <TiXmlElement*> (p->getValue());
      TiXmlNode* state_node = dialog_element->FirstChild("state");
      UtlString state;
      textContentShallow(state, state_node);

      if (state.compareTo("terminated") == 0)
      {
         // This dialog was terminated.  Remove it.
         delete dialog_element;
         mXmlDialogs.destroy(id);
      }
   }
   // Note that we do not have to publish this change, as the deletion
   // of these dialogs does not have to be sent to the subscribers
   // quickly.
}

// Add to the HttpBody the current state of the resource.
void ResourceInstance::generateBody(UtlString& rlmi,
                                    HttpBody& body,
                                    UtlBoolean consolidated,
                                    const UtlString& displayName) const
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceInstance::generateBody mInstanceName = '%s', consolidated = %d, displayName = '%s', mContentPresent = %d",
                 mInstanceName.data(), consolidated, displayName.data(),
                 mContentPresent);

   if (consolidated)
   {
      if (mContentPresent)
      {
         // If this is a consolidated dialog event list, edit the
         // stored XML into the right form and append each dialog
         // to the resource list event notice.

         TiXmlUtlStringWriter writer(&rlmi);

         // Iterate through all the <dialog> elements.
         UtlHashMapIterator itor(mXmlDialogs);
         UtlContainable* id;
         while ((id = itor()))
         {
            UtlVoidPtr* p = dynamic_cast <UtlVoidPtr*> (itor.value());
            TiXmlElement* dialog_element =
               static_cast <TiXmlElement*> (p->getValue());

            // Now that we've got a <dialog> element, edit it to fit
            // into a consolidated event notice.

            // Get the display name right.
            // Find the <local> element, which we know exists due to
            // earlier processing.
            TiXmlNode* local = dialog_element->FirstChild("local");
            // Find the <local><identity> element, which we know
            // exists due to earlier processing.
            TiXmlNode* identity = local->FirstChild("identity");
            // Update the display attribute, as that is what will show
            // on the phone.
            identity->ToElement()->
               SetAttribute("display", displayName);

            // Un-parse the dialog into the string for storage.
            writer << *dialog_element;
            writer << "\r\n";
         }
      }
   }
   else
   {
      // Generate the XML for the instance.
      rlmi += "    <instance id=\"";
      XmlEscape(rlmi, mInstanceName);
      rlmi += "\" state=\"";
      // Subscription states don't require escaping.
      rlmi += mSubscriptionState;
      rlmi += "\"";

      // Generate the body part for the instance, if necessary.
      if (mContentPresent)
      {
         UtlString contentBodyPartCid;
         // Use the count of parts in 'body' to generate a unique identifier for
         // each part.
         contentBodyPartCid.appendNumber(body.getMultipartCount());
         contentBodyPartCid += "@";
         contentBodyPartCid += getResourceListServer()->getDomainName();

         rlmi += " cid=\"";
         rlmi += contentBodyPartCid;
         rlmi += "\"";

         // Now add the <...> and use it in the header.
         contentBodyPartCid.prepend("<");
         contentBodyPartCid.append(">");

         HttpBody content_body(mContent.data(),
                               mContent.length(),
                               getResourceListServer()->getContentType());
         UtlDList content_body_parameters;
         content_body_parameters.append(
            new NameValuePair(HTTP_CONTENT_ID_FIELD,
                              contentBodyPartCid));
         body.appendBodyPart(content_body, content_body_parameters);
         content_body_parameters.destroyAll();
      }

      rlmi += "/>\r\n";
   }
}

/* ============================ ACCESSORS ================================= */

/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ INQUIRY =================================== */

// Dump the object's internal state.
void ResourceInstance::dumpState()
{
   // indented 12

   OsSysLog::add(FAC_RLS, PRI_INFO,
                 "\t            ResourceInstance %p mInstanceName = '%s', "
                 "mSubscriptionState = '%s', mContentPresent = %d, mContent = '%s'",
                 this, mInstanceName.data(), mSubscriptionState.data(),
                 mContentPresent,
                 mContentPresent ? mContent.data() : "[invalid]");

   UtlHashMapIterator itor(mXmlDialogs);
   UtlString* dialog_id;
   while ((dialog_id = dynamic_cast <UtlString*> (itor())))
   {
      UtlVoidPtr* p = dynamic_cast <UtlVoidPtr*> (itor.value());
      TiXmlElement* dialog_element =
         static_cast <TiXmlElement*> (p->getValue());
      UtlString s;
      TiXmlUtlStringWriter writer(&s);
      writer <<*dialog_element;

      OsSysLog::add(FAC_RLS, PRI_INFO,
                    "\t              mXmlDialogs{'%s'} = '%s'",
                    dialog_id->data(), s.data());
   }
}

/**
 * Get the ContainableType for a UtlContainable-derived class.
 */
UtlContainableType ResourceInstance::getContainableType() const
{
   return ResourceInstance::TYPE;
}

/* //////////////////////////// PRIVATE /////////////////////////////////// */

/* ============================ FUNCTIONS ================================= */

// Destroy the contents of mXmlDialogs.
void ResourceInstance::destroyXmlDialogs()
{
   // First, follow through the UtlVoidPtr's to get pointers to the
   // XML trees and delete them.
   UtlHashMapIterator itor(mXmlDialogs);
   UtlContainable* id;
   while ((id = itor()))
   {
      UtlVoidPtr* p = dynamic_cast <UtlVoidPtr*> (itor.value());
      delete static_cast <TiXmlElement*> (p->getValue());
   }
   // Now clear the hash map and its keys and the UtlVoidPtr's themselves.
   mXmlDialogs.destroyAll();
}

//! Terminate the non-Terminated contents of mXmlDialogs.
void ResourceInstance::terminateXmlDialogs()
{
   OsSysLog::add(FAC_RLS, PRI_DEBUG,
                 "ResourceInstance::terminateXmlDialogs mInstanceName = '%s'",
                 mInstanceName.data());

   // Iterate through the contents.
   UtlHashMapIterator itor(mXmlDialogs);
   UtlContainable* id;
   while ((id = itor()))
   {
      // The XML document for a single dialog.
      UtlVoidPtr* value = dynamic_cast <UtlVoidPtr*> (itor.value());
      TiXmlElement* dialog_element = static_cast <TiXmlElement*> (value->getValue());
      if (NULL != dialog_element)
      {
         // Get the "state" XML node.
         TiXmlNode* state = dialog_element->FirstChild("state");
         // Destroy the dialog only if the state is not "terminated".
         UtlString stateText;
         textContentShallow(stateText, state); // textContentShallow allows state == NULL.

         // Check if the state is not terminated.
         if (0 != stateText.compareTo("terminated"))
         {
            TiXmlElement newstate = "state";
            TiXmlText newtext = "terminated";

            // Replace the old state element with a new state element indicating terminated.
            // The calling routine will then normally overwrite this with the actual (full) state.
            // Note that tinyxml will delete the old state element for us.
            newstate.InsertEndChild(newtext);
            dialog_element->ReplaceChild(state, newstate);
         }
      }
   }
}
