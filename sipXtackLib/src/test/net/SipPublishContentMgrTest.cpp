//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <os/OsDefs.h>
#include <net/SipPublishContentMgr.h>
#include <net/HttpBody.h>
#include <net/SipSubscribeServerEventHandler.h>
#include <net/SipMessage.h>
#include <net/SipDialogEvent.h>

#define TEST_RESOURCE_ID "sip:moh@example.com"

static void* mAppData;
static UtlString mResourceId;
static UtlString mEventTypeKey;
static UtlString mEventType;

void static contentChangeCallback(void* applicationData,
                                 const char* resourceId,
                                 const char* eventTypeKey,
                                 const char* eventType)
{
   mAppData = applicationData;
   mResourceId = resourceId;
   mEventTypeKey = eventTypeKey;
   mEventType = eventType;
}

/* The default content constructor used by the testDefaultConstructor test. */
class TestDefaultConstructorClass : public SipPublishContentMgrDefaultConstructor
{
public:

   /** Generate the content for a resource and event.
    */
   virtual void generateDefaultContent(SipPublishContentMgr* contentMgr,
                                       const char* resourceId,
                                       const char* eventTypeKey,
                                       const char* eventType);

   /// Make a copy of this object according to its real type.
   virtual SipPublishContentMgrDefaultConstructor* copy();

   // Service routine for UtlContainable.
   virtual const char* const getContainableType() const;

protected:
   static UtlContainableType TYPE;    /** < Class type used for runtime checking */
};

// Static identifier for the type.
const UtlContainableType TestDefaultConstructorClass::TYPE = "TestDefaultConstructorClass";

// Generate the default content for dialog status.
// It generates default content for resourceId's that start with '1'.
void TestDefaultConstructorClass::generateDefaultContent(SipPublishContentMgr* contentMgr,
                                                         const char* resourceId,
                                                         const char* eventTypeKey,
                                                         const char* eventType)
{
   if (resourceId[0] == '1')
   {
      // Construct the body, an empty notice for the user.
      UtlString content;
      char buffer[100];
      sprintf(buffer, "This is default content for the resource '%s'.",
              resourceId);
      content.append(buffer);

      // Build an HttpBody.
      HttpBody *body = new HttpBody(content, strlen(content), "text/plain");

      // Install it for the resource.
      contentMgr->publish(resourceId, eventTypeKey, eventType, 1, &body);
   }
}

// Make a copy of this object according to its real type.
SipPublishContentMgrDefaultConstructor* TestDefaultConstructorClass::copy()
{
   // We cheat on copying these objects by just duplicating the pointer to the
   // original.  This is OK, because we never keep the result of copy() around
   // longer than the life of the original.  It also makes it much easier to see
   // if getPublished is returning a "copy" of the right object, as we can just
   // check the pointer value.
   return this;
}

// Get the ContainableType for a UtlContainable derived class.
UtlContainableType TestDefaultConstructorClass::getContainableType() const
{
    return TestDefaultConstructorClass::TYPE;
}

/**
 * Unit test for SipPublishContentMgr
 */
class SipPublishContentMgrTest : public CppUnit::TestCase
{
   CPPUNIT_TEST_SUITE(SipPublishContentMgrTest);
   CPPUNIT_TEST(testDefaultPublishContent);
   CPPUNIT_TEST(testDefaultConstructor);
   CPPUNIT_TEST(testPublishContent);
   CPPUNIT_TEST(testGetContent);
   CPPUNIT_TEST(testContentChangeObserver);
   CPPUNIT_TEST(testGetContentAccept);
   CPPUNIT_TEST_SUITE_END();

public:

   void testDefaultPublishContent()
      {
         const char *content =
            "<?xml version=\"1.0\"?>\n"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\"0\" state=\"full\" entity=\"sip:moh@panther.example.com:5120\">\n"
            "<dialog id=\"1\" call-id=\"call-1116603513-270@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>sip:moh@panther.example.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>sip:4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"2\" call-id=\"call-1116603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>sip:moh@panther.example.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>sip:4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"3\" call-id=\"call-2226603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>sip:moh@panther.example.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>sip:4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "</dialog-info>\n"
            ;

         SipPublishContentMgr publisher;

         ssize_t bodyLength = strlen(content);
         HttpBody *body = new HttpBody(content, bodyLength, "text/xml");

         int numOldContents;
         HttpBody *oldContents[2];

         publisher.getPublished(NULL, "dialog", 1,
                                numOldContents, oldContents, NULL);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of contents should be zero",
                                      0, numOldContents);

         publisher.publishDefault("dialog", "dialog", 1, &body);

         publisher.getPublished(NULL, "dialog", 1,
                                numOldContents, oldContents, NULL);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of contents are not the same",
                                      1, numOldContents);
         UtlString returned_contents;
         ssize_t returned_length;
         oldContents[0]->getBytes(&returned_contents, &returned_length);
         CPPUNIT_ASSERT_MESSAGE("bad body",
                                strcmp(returned_contents.data(),
                                       content) == 0);

         publisher.unpublishDefault("dialog", "dialog");
      }

   void testDefaultConstructor()
      {
         // Exercise the default content constructor logic.

         // The constructor will provide default content for every resource
         // whose name starts with "1".

         // The test event type.
         const char *event_type = "testdefaultconstructor";

         SipPublishContentMgr publisher;

         // Verify that the default content constructor is null.

         int numOldContents;
         HttpBody *oldContents[2];
         SipPublishContentMgrDefaultConstructor *constructor;

         publisher.getPublished(NULL, event_type, 1,
                                numOldContents, oldContents, &constructor);

         CPPUNIT_ASSERT_MESSAGE("getPublished should return 0 old contents",
                                numOldContents == 0);
         CPPUNIT_ASSERT_MESSAGE("getPublished should return NULL",
                                constructor == NULL);

         // Register the default content constructor.

         TestDefaultConstructorClass* p = new TestDefaultConstructorClass;
         publisher.publishDefault(event_type, event_type, p);

         // See if getPublished can retrieve it.

         publisher.getPublished(NULL, event_type, 1,
                                numOldContents, oldContents, &constructor);

         CPPUNIT_ASSERT_MESSAGE("getPublished should return 0 old contents",
                                numOldContents == 0);
         CPPUNIT_ASSERT_MESSAGE("constructor not returned by getPublished",
                                constructor == p);

         // Ensure that it can't be retrieved for other event types.

         publisher.getPublished(NULL, "dialog", 1,
                                numOldContents, oldContents, &constructor);

         CPPUNIT_ASSERT_MESSAGE("getPublished should return 0 old contents",
                                numOldContents == 0);
         CPPUNIT_ASSERT_MESSAGE("getPublished should return NULL for 'dialog'",
                                constructor == NULL);

         // Provide a string for default content also.

         const char *default_content = "This is the default string.";
         {
            ssize_t bodyLength = strlen(default_content);
            HttpBody *body = new HttpBody(default_content, bodyLength,
                                          "text/plain");
            publisher.publishDefault(event_type, event_type, 1, &body);
         }

         // See if getPublished returns the string.

         publisher.getPublished(NULL, event_type, 1,
                                numOldContents, oldContents, &constructor);

         CPPUNIT_ASSERT_MESSAGE("getPublished should return 1 old contents",
                                numOldContents == 1);
         CPPUNIT_ASSERT_MESSAGE("getPublished should return default string",
                                strcmp(oldContents[0]->getBytes(), default_content) == 0);

         // Make sure getPublished does not return the string for other events.

         publisher.getPublished(NULL, "dialog", 1,
                                numOldContents, oldContents, &constructor);

         CPPUNIT_ASSERT_MESSAGE("getPublished should return 0 old contents",
                                numOldContents == 0);

         // Check to see if default content is produced for the right
         // resources.

         // Resource:
         //    Name    Specific content?    Default from constr.? Default str.?
         //    0       N                    N                     Y
         //    1a      N                    Y                     Y
         //    1b      Y                    Y                     Y
         //    2       Y                    N                     Y

         const char *content_1b = "This is content for 1b.";
         {
            ssize_t bodyLength = strlen(content_1b);
            HttpBody *body = new HttpBody(content_1b, bodyLength,
                                          "text/plain");
            publisher.publish("1b", event_type, event_type, 1, &body);
         }
         const char *content_2 = "This is content for 2.";
         {
            ssize_t bodyLength = strlen(content_2);
            HttpBody *body = new HttpBody(content_2, bodyLength, "text/plain");
            publisher.publish("2", event_type, event_type, 1, &body);
         }

         HttpBody *b;
         UtlBoolean d;
         const char *s;
         ssize_t l;

         publisher.getContent("0", event_type, event_type, "text/plain", b, d);
         CPPUNIT_ASSERT_MESSAGE("Content for 0 should be default",
                                d);
         b->getBytes(&s, &l);
         CPPUNIT_ASSERT_MESSAGE("Content for 0 is incorrect",
                                strcmp(s, default_content) == 0);

         publisher.getContent("1a", event_type, event_type, "text/plain", b, d);
         CPPUNIT_ASSERT_MESSAGE("Content for 1a should be default",
                                d);
         b->getBytes(&s, &l);
         CPPUNIT_ASSERT_MESSAGE("Content for 1a is incorrect",
                                strcmp(s, "This is default content for the resource '1a'.") == 0);

         publisher.getContent("1b", event_type, event_type, "text/plain", b, d);
         CPPUNIT_ASSERT_MESSAGE("Content for 1b should not be default",
                                !d);
         b->getBytes(&s, &l);
         CPPUNIT_ASSERT_MESSAGE("Content for 1b is incorrect",
                                strcmp(s, content_1b) == 0);

         publisher.getContent("2", event_type, event_type, "text/plain", b, d);
         CPPUNIT_ASSERT_MESSAGE("Content for 2 should not be default",
                                !d);
         b->getBytes(&s, &l);
         CPPUNIT_ASSERT_MESSAGE("Content for 2 is incorrect",
                                strcmp(s, content_2) == 0);

         // Remove the default content constructor.

         publisher.unpublishDefault(event_type, event_type);

         // See if getPublished now returns NULL.

         publisher.getPublished(NULL, event_type, 1,
                                numOldContents, oldContents, &constructor);

         CPPUNIT_ASSERT_MESSAGE("getPublished should return 0 old contents",
                                numOldContents == 0);
         CPPUNIT_ASSERT_MESSAGE("getPublished should return NULL for the default constructor",
                                constructor == NULL);
      }

   void testPublishContent()
      {
         const char *content =
            "<?xml version=\"1.0\"?>\n"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\"0\" state=\"full\" entity=\"sip:moh@panther.example.com:5120\">\n"
            "<dialog id=\"1\" call-id=\"call-1116603513-270@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>sip:moh@panther.example.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>sip:4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"2\" call-id=\"call-1116603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>sip:moh@panther.example.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>sip:4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"3\" call-id=\"call-2226603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>sip:moh@panther.example.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>sip:4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "</dialog-info>\n"
            ;

         SipPublishContentMgr publisher;

         ssize_t bodyLength = strlen(content);
         HttpBody *body = new HttpBody(content, bodyLength, "text/xml");

         int numOldContents;
         HttpBody *oldContents[2];

         publisher.getPublished(TEST_RESOURCE_ID, "dialog",
                                1, numOldContents, oldContents, NULL);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of contents should be zero",
                                      0, numOldContents);

         publisher.publish(TEST_RESOURCE_ID, "dialog", "dialog", 1, &body);

         SipSubscribeServerEventHandler eventHandler;
         SipMessage notifyRequest;
         CPPUNIT_ASSERT(eventHandler.getNotifyContent(TEST_RESOURCE_ID,
                                                      "dialog",
                                                      "dialog",
                                                      publisher,
                                                      "text/xml",
                                                      notifyRequest,
                                                      TRUE));
         const char* notifyBodyBytes = NULL;
         ssize_t notifyBodySize = 0;
         const HttpBody* notifyBody = notifyRequest.getBody();
         notifyBody->getBytes(&notifyBodyBytes, &notifyBodySize);
         CPPUNIT_ASSERT(notifyBodyBytes);
         CPPUNIT_ASSERT(strcmp(content, notifyBodyBytes) == 0);

         publisher.getPublished(TEST_RESOURCE_ID, "dialog",
                                1, numOldContents, oldContents, NULL);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of contents are not the same",
                                      1, numOldContents);
         UtlString returned_contents;
         ssize_t returned_length;
         oldContents[0]->getBytes(&returned_contents, &returned_length);
         CPPUNIT_ASSERT_MESSAGE("bad body",
                                strcmp(returned_contents.data(),
                                       content) == 0);

         publisher.unpublish(TEST_RESOURCE_ID, "dialog", "dialog");
      }

   void testGetContent()
      {
         const char *content =
            "<?xml version=\"1.0\"?>\n"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\"0\" state=\"full\" entity=\"sip:moh@panther.example.com:5120\">\n"
            "<dialog id=\"1\" call-id=\"call-1116603513-270@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>sip:moh@panther.example.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>sip:4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"2\" call-id=\"call-1116603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>sip:moh@panther.example.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>sip:4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "<dialog id=\"3\" call-id=\"call-2226603513-890@10.1.1.153\" local-tag=\"264460498\" remote-tag=\"1c10982\" direction=\"recipient\">\n"
            "<state>confirmed</state>\n"
            "<duration>0</duration>\n"
            "<local>\n"
            "<identity>sip:moh@panther.example.com:5120</identity>\n"
            "<target uri=\"sip:moh@10.1.1.26:5120\"/>\n"
            "</local>\n"
            "<remote>\n"
            "<identity>sip:4444@10.1.1.153</identity>\n"
            "</remote>\n"
            "</dialog>\n"
            "</dialog-info>\n"
            ;

         SipPublishContentMgr publisher;

         ssize_t bodyLength = strlen(content);
         HttpBody *body = new HttpBody(content, bodyLength,
                                       DIALOG_EVENT_CONTENT_TYPE);

         int numOldContents;
         HttpBody *oldContents[2];

         publisher.publish(TEST_RESOURCE_ID, "dialog", "dialog", 1, &body);

         UtlBoolean foundContent;
         UtlBoolean isDefaultContent;

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             // The content type is hard-coded
                                             // here to check that the #define
                                             // DIALOG_EVENT_CONTENT_TYPE,
                                             // which is used everywhere in
                                             // sipX, is right.
                                             "application/dialog-info+xml",
                                             oldContents[0], isDefaultContent);

         CPPUNIT_ASSERT(FALSE==isDefaultContent);

         CPPUNIT_ASSERT(TRUE==foundContent);

         ssize_t length;
         const char* contentBody = NULL;

         oldContents[0]->getBytes(&contentBody, &length);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength, length);

         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content, contentBody);

         foundContent = publisher.getContent("something-else@example.com", "dialog", "dialog",
                                             "application/dialog-info+xml",
                                             oldContents[0], isDefaultContent);

         CPPUNIT_ASSERT(FALSE==foundContent);

         publisher.getPublished(TEST_RESOURCE_ID, "dialog", 1,
                                numOldContents, oldContents, NULL);

         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of contents are not the same",
                                      1, numOldContents);

         publisher.unpublish(TEST_RESOURCE_ID, "dialog", "dialog");

         UtlString returned_contents;
         ssize_t returned_length;
         oldContents[0]->getBytes(&returned_contents, &returned_length);
         CPPUNIT_ASSERT_MESSAGE("bad body",
                                strcmp(returned_contents.data(),
                                       content) == 0);
      }

   void testContentChangeObserver()
      {
         const char *content =
            "<?xml version=\"1.0\"?>\n"
            "<dialog-info xmlns=\"urn:ietf:params:xml:ns:dialog-info\" version=\"0\" state=\"full\" entity=\"sip:moh@panther.example.com:5120\">\n"
            "</dialog-info>\n"
            ;
         const char *resourceId = TEST_RESOURCE_ID;
         const char *eventType = "dialog";
         const void* appData = "testContentChangeObserver";

         SipPublishContentMgr publisher;

         publisher.setContentChangeObserver(eventType, (void *)appData, contentChangeCallback);

         ssize_t bodyLength = strlen(content);
         HttpBody *body = new HttpBody(content, bodyLength,
                                       DIALOG_EVENT_CONTENT_TYPE);

         publisher.publish(resourceId, eventType, eventType, 1, &body);

         CPPUNIT_ASSERT_MESSAGE("bad app data pointer", appData == mAppData);
         ASSERT_STR_EQUAL_MESSAGE("incorrect resource Id", resourceId, mResourceId.data());
         ASSERT_STR_EQUAL_MESSAGE("incorrect event type key", eventType, mEventTypeKey.data());
         ASSERT_STR_EQUAL_MESSAGE("incorrect event type", eventType, mEventType.data());

         void* myAppData = NULL;

         SipPublishContentMgr::SipPublisherContentChangeCallback myCallbackFunc;

         publisher.removeContentChangeObserver(eventType, ((void*&)myAppData), myCallbackFunc);

         CPPUNIT_ASSERT_MESSAGE("bad app data pointer", appData == myAppData);

         CPPUNIT_ASSERT_MESSAGE("bad callback founction pointer", contentChangeCallback == myCallbackFunc);
      }

   void testGetContentAccept()
      {
         SipPublishContentMgr publisher;

         HttpBody *(bodies[2]);

         const char *content_text_plain = "text/plain content";
         ssize_t bodyLength_text_plain = strlen(content_text_plain);
         bodies[0] = new HttpBody(content_text_plain, bodyLength_text_plain,
                                  "text/plain");

         const char *content_text_xml = "text/xml content";
         ssize_t bodyLength_text_xml = strlen(content_text_xml);
         bodies[1] = new HttpBody(content_text_xml, bodyLength_text_xml,
                                  "text/xml;charset=X");

         publisher.publish(TEST_RESOURCE_ID, "dialog", "dialog", 2, bodies);

         HttpBody *content;
         UtlBoolean foundContent;
         UtlBoolean isDefaultContent;
         ssize_t length;
         const char* contentBody;

         // Search with no Accept.

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             // Null pointer.
                                             NULL,
                                             content, isDefaultContent);
         CPPUNIT_ASSERT(FALSE==isDefaultContent);
         CPPUNIT_ASSERT(TRUE==foundContent);

         content->getBytes(&contentBody, &length);
         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content_text_plain, contentBody);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength_text_plain, length);

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             // Null string.
                                             "",
                                             content, isDefaultContent);
         CPPUNIT_ASSERT(FALSE==isDefaultContent);
         CPPUNIT_ASSERT(TRUE==foundContent);

         content->getBytes(&contentBody, &length);
         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content_text_plain, contentBody);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength_text_plain, length);

         // Search with "Accept: text/plain".

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             "text/plain",
                                             content, isDefaultContent);
         CPPUNIT_ASSERT(FALSE==isDefaultContent);
         CPPUNIT_ASSERT(TRUE==foundContent);

         content->getBytes(&contentBody, &length);
         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content_text_plain, contentBody);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength_text_plain, length);

         // Search with "Accept: text/xml".

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             "text/xml",
                                             content, isDefaultContent);
         CPPUNIT_ASSERT(FALSE==isDefaultContent);
         CPPUNIT_ASSERT(TRUE==foundContent);

         content->getBytes(&contentBody, &length);
         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content_text_xml, contentBody);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength_text_xml, length);

         // Search with "Accept: text/plain,text/xml".

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             "text/plain,text/xml",
                                             content, isDefaultContent);
         CPPUNIT_ASSERT(FALSE==isDefaultContent);
         CPPUNIT_ASSERT(TRUE==foundContent);

         content->getBytes(&contentBody, &length);
         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content_text_plain, contentBody);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength_text_plain, length);

         // Search with "Accept: text/xml,text/plain".

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             "text/xml,text/plain",
                                             content, isDefaultContent);
         CPPUNIT_ASSERT(FALSE==isDefaultContent);
         CPPUNIT_ASSERT(TRUE==foundContent);

         content->getBytes(&contentBody, &length);
         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content_text_plain, contentBody);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength_text_plain, length);

         // Search with "Accept: text/nonexistent,text/plain".

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             "text/nonexistent,text/plain",
                                             content, isDefaultContent);
         CPPUNIT_ASSERT(FALSE==isDefaultContent);
         CPPUNIT_ASSERT(TRUE==foundContent);

         content->getBytes(&contentBody, &length);
         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content_text_plain, contentBody);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength_text_plain, length);


         // Search with "Accept: text/plain,text/nonexistent".

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             "text/plain",
                                             content, isDefaultContent);
         CPPUNIT_ASSERT(FALSE==isDefaultContent);
         CPPUNIT_ASSERT(TRUE==foundContent);

         content->getBytes(&contentBody, &length);
         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content_text_plain, contentBody);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength_text_plain, length);

         // Search with "Accept: text/nonexistent,text/xml".

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             "text/nonexistent,text/xml",
                                             content, isDefaultContent);
         CPPUNIT_ASSERT(FALSE==isDefaultContent);
         CPPUNIT_ASSERT(TRUE==foundContent);

         content->getBytes(&contentBody, &length);
         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content_text_xml, contentBody);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength_text_xml, length);

         // Search with "Accept: text/xml,text/nonexistent".

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             "text/xml",
                                             content, isDefaultContent);
         CPPUNIT_ASSERT(FALSE==isDefaultContent);
         CPPUNIT_ASSERT(TRUE==foundContent);

         content->getBytes(&contentBody, &length);
         ASSERT_STR_EQUAL_MESSAGE("incorrect body value", content_text_xml, contentBody);
         CPPUNIT_ASSERT_EQUAL_MESSAGE("number of bytes are not the same",
                                      bodyLength_text_xml, length);

         // Search with "Accept: text/nonexistent".

         foundContent = publisher.getContent(TEST_RESOURCE_ID, "dialog", "dialog",
                                             "text/nonexistent",
                                             content, isDefaultContent);
         CPPUNIT_ASSERT(FALSE==isDefaultContent);
         CPPUNIT_ASSERT(FALSE==foundContent);
      }
};

CPPUNIT_TEST_SUITE_REGISTRATION(SipPublishContentMgrTest);
