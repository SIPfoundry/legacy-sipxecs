//
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
#include <utl/UtlRegex.h>
#include "os/OsDefs.h"
#include "os/OsDateTime.h"
#include "os/OsSysLog.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/ResultSet.h"
#include "SipRedirectorGateway.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/XmlContent.h"
#include "net/HttpBody.h"
#include "net/MimeBodyPart.h"

// DEFINES
// How big the forms on the Web page can get.
#define FORM_SIZE 10240

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorGateway(instanceName);
}

// Constructor
SipRedirectorGateway::SipRedirectorGateway(const UtlString& instanceName) :
   RedirectPlugin(instanceName),
   mPrefix(""),
   mMapLock(OsBSem::Q_FIFO, OsBSem::FULL),
   mWriterTask(this)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorGateway");
}

// Destructor
SipRedirectorGateway::~SipRedirectorGateway()
{
}

// Read config information.
void SipRedirectorGateway::readConfig(OsConfigDb& configDb)
{
   UtlString string;
   char *endptr;

   mReturn = OS_SUCCESS;

   if (configDb.get("MAPPING_FILE", mMappingFileName) != OS_SUCCESS ||
       mMappingFileName.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "%s::readConfig "
                    "MAPPING_FILE parameter '%s' missing or empty",
                    mLogName.data(), mMappingFileName.data());
      mReturn = OS_FAILED;
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig "
                    "MAPPING_FILE is '%s'",
                    mLogName.data(), mMappingFileName.data());
   }

   if (configDb.get("PREFIX", mPrefix) != OS_SUCCESS ||
       mPrefix.isNull())
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig "
                    "dialing prefix is empty", mLogName.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig "
                    "dialing prefix is '%s'", mLogName.data(), mPrefix.data());
   }

   if (configDb.get("DIGITS", string) == OS_SUCCESS &&
       !string.isNull() &&
       (mDigits = strtol(string.data(), &endptr, 10),
        endptr - string.data() == string.length() &&
        mDigits >= 1 && mDigits <= 10))
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig "
                    "variable digit count is %d", mLogName.data(), mDigits);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "%s::readConfig "
                    "variable digit count is missing, empty, "
                    "or out of range (1 to 10)",
                    mLogName.data());
      mReturn = OS_FAILED;
   }

   if (configDb.get("PORT", string) == OS_SUCCESS &&
       !string.isNull() &&
       (mPort = strtol(string.data(), &endptr, 10),
        endptr - string.data() == string.length() &&
        mPort >= 1 && mPort <= 65535))
   {
      OsSysLog::add(FAC_SIP, PRI_INFO,
                    "%s::readConfig "
                    "listening port is %d", mLogName.data(), mPort);
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "%s::readConfig "
                    "listening port '%s' is missing, empty, "
                    "or out of range (1 to 65535)",
                    mLogName.data(), string.data());
      mReturn = OS_FAILED;
   }
}

// Initialize
OsStatus
SipRedirectorGateway::initialize(OsConfigDb& configDb,
                                 int redirectorNo,
                                 const UtlString& localDomainHost)
{
   mDomainName = localDomainHost;
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::SipRedirectorGateway domainName = '%s'",
                 mLogName.data(), mDomainName.data());

   if (mReturn == OS_SUCCESS)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::SipRedirectorGateway Loading mappings from '%s'",
                    mLogName.data(), mMappingFileName.data());
      loadMappings(&mMappingFileName, &mMapUserToContacts, &mMapContactsToUser);

      // Set up the HTTP server on socket mPort.
      mpSocket = new OsServerSocket(50, mPort);
      mpServer = new HttpServer(socket);
      mpServer->addRequestProcessor("/map.html", &displayForm);
      mpServer->start();

      // Start the writer task.
      mWriterTask.start();
   }

   return mReturn;
}

// Finalize
void
SipRedirectorGateway::finalize()
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s::finalize entered", mLogName.data());

   mWriterTask.requestShutdown();
   // Wait 1 second for writer to shut down.
   OsTask::delay(1000);
}

RedirectPlugin::LookUpStatus
SipRedirectorGateway::lookUp(
   const SipMessage& message,
   const UtlString& requestString,
   const Url& requestUri,
   const UtlString& method,
   ContactList& contactList,
   RequestSeqNo requestSeqNo,
   int redirectorNo,
   SipRedirectorPrivateStorage*& privateStorage,
   ErrorDescriptor& errorDescriptor)
{
   UtlString userId;
   requestUri.getUserId(userId);
   OsSysLog::add(FAC_SIP, PRI_DEBUG, "%s::lookUp "
                 "userId = '%s'",
                 mLogName.data(), userId.data());

   // Test for the presence of the prefix.
   const char* user = userId.data();
   int prefix_length = mPrefix.length();
   // Compare the prefix.
   if (strncmp(user, mPrefix.data(), prefix_length) == 0)
   {
      // Extract the full routing prefix.
      UtlString routing_prefix(userId.data(), prefix_length + mDigits);

      // Look up the routing prefix in the map.
      mMapLock.acquire();
      UtlContainable* value = mMapUserToContacts.findValue(&routing_prefix);
      if (value)
      {
         // Have to make a copy of the string, as the map entry might get
         // changed later.
         UtlString hostpart(*(dynamic_cast <UtlString*> (value)));

         mMapLock.release();

         if (!hostpart.isNull())
         {
            // Add the contact.
            UtlString s("sip:");
            s.append(hostpart);
            Url uri(s);
            // The remainder of userId becomes the userId to the gateway.
            uri.setUserId(&userId.data()[prefix_length + mDigits]);
            // Add the contact.
            contactList.add( uri, *this );
         }
      }
      else
      {
         mMapLock.release();
      }
   }

   return RedirectPlugin::SUCCESS;
}

const UtlString& SipRedirectorGateway::name( void ) const
{
   return mLogName;
}

void SipRedirectorGateway::loadMappings(UtlString* file_name,
                                    UtlHashMap* mapUserToContacts,
                                    UtlHashMap* mapContactsToUser)
{
    // Load the XML file.
    TiXmlDocument *mDoc = new TiXmlDocument(file_name->data());
    if (mDoc->LoadFile())
    {
       // Look at the top element, which should be "Gateway".
       TiXmlNode* GatewayNode = mDoc->FirstChild("Gateway");
       if (GatewayNode)
       {
          mMapLock.acquire();

          for (TiXmlNode* mapNode = NULL;
               (mapNode = GatewayNode->IterateChildren("map", mapNode));
             )
          {
             // Carefully get the <prefix> and <hostpart> text.
             TiXmlNode* c1 = mapNode->FirstChild("prefix");
             if (!c1)
             {
                OsSysLog::add(FAC_SIP, PRI_ERR,
                              "%s::loadMappings cannot find <prefix> child",
                              mLogName.data());
             }
             else
             {
                TiXmlNode* c2 = c1->FirstChild();
                if (!c2)
                {
                   OsSysLog::add(FAC_SIP, PRI_ERR,
                                 "%s::loadMappings cannot find text child of <prefix>",
                                 mLogName.data());
                }
                else
                {
                   const char* prefix = c2->Value();
                   if (prefix == NULL || *prefix == '\0')
                   {
                      OsSysLog::add(FAC_SIP, PRI_ERR,
                                    "%s::loadMappings text of <prefix> is null",
                                    mLogName.data());
                   }
                   else
                   {
                      TiXmlNode* c3 = mapNode->FirstChild("hostpart");
                      if (!c3)
                      {
                         OsSysLog::add(FAC_SIP, PRI_ERR,
                                       "%s::loadMappings cannot find <hostpart> child",
                                       mLogName.data());
                      }
                      else
                      {
                         TiXmlNode* c4 = c3->FirstChild();
                         if (!c4)
                         {
                            OsSysLog::add(FAC_SIP, PRI_ERR,
                                          "%s::loadMappings cannot find text child of <hostpart>",
                                          mLogName.data());
                         }
                         else
                         {
                            const char* hostpart = c4->Value();
                            if (hostpart == NULL || *hostpart == '\0')
                            {
                               OsSysLog::add(FAC_SIP, PRI_ERR,
                                             "%s::loadMappings text of <hostpart> is null",
                                             mLogName.data());
                            }
                            else
                            {
                               // Load the mapping into the maps.
                               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                             "%s::loadMappings added '%s' -> '%s'",
                                             mLogName.data(), prefix, hostpart);
                               UtlString* prefix_string = new UtlString(prefix);
                               UtlString* hostpart_string = new UtlString(hostpart);
                               mapUserToContacts->insertKeyAndValue(prefix_string,
                                                                    hostpart_string);
                               mapContactsToUser->insertKeyAndValue(hostpart_string,
                                                                    prefix_string);
                            }
                         }
                      }
                   }
                }
             }
          }

          mMapsModified = FALSE;

          mMapLock.release();

          OsSysLog::add(FAC_SIP, PRI_DEBUG,
                        "%s::loadMappings done loading mappings", mLogName.data());
       }
       else
       {
          OsSysLog::add(FAC_SIP, PRI_CRIT,
                        "%s::loadMappings unable to extract Gateway element",
                        mLogName.data());
       }

    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_CRIT,
                     "%s::loadMappings LoadFile() failed", mLogName.data());
    }
}

void SipRedirectorGateway::writeMappings(UtlString* file_name,
                                     UtlHashMap* mapUserToContacts)
{
   UtlString temp_file_name;
   FILE* f;

   temp_file_name = *file_name;
   temp_file_name.append(".new");
   f = fopen(temp_file_name.data(), "w");
   if (f == NULL)
   {
      OsSysLog::add(FAC_SIP, PRI_CRIT,
                    "%s::writeMappings fopen('%s') failed, errno = %d",
                    mLogName.data(), temp_file_name.data(), errno);
   }
   else
   {
      mMapLock.acquire();

      fprintf(f, "<Gateway>\n");
      UtlHashMapIterator itor(*mapUserToContacts);
      UtlContainable* c;
      while ((c = itor()))
      {
         UtlString* k = dynamic_cast<UtlString*> (c);
         UtlString k_escaped;
         XmlEscape(k_escaped, *k);
         UtlString* v =
            dynamic_cast<UtlString*> (mapUserToContacts->findValue(c));
         UtlString v_escaped;
         XmlEscape(v_escaped, *v);
         fprintf(f, "  <map><prefix>%s</prefix><hostpart>%s</hostpart></map>\n",
                 k_escaped.data(), v_escaped.data());
      }
      fprintf(f, "</Gateway>\n");
      fclose(f);

      mMapsModified = FALSE;

      mMapLock.release();

      if (rename(temp_file_name.data(), file_name->data()) != 0)
      {
         OsSysLog::add(FAC_SIP, PRI_CRIT,
                       "%s::writeMappings rename('%s', '%s') failed, errno = %d",
                       mLogName.data(), temp_file_name.data(), file_name->data(), errno);
      }
   }
}

UtlString* SipRedirectorGateway::addMapping(const char* contacts,
                                        int length)
{
   // Make the contact UtlString.
   UtlString* contactString = new UtlString(contacts, length);
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::addMapping inserting '%s'",
                 mLogName.data(), contactString->data());
   // Get its hash.
   unsigned hash = contactString->hash();
   // Keys are 24 bits, and the starting key is the lower 24 bits of the hash.
   unsigned key = hash & 0xFFFFFF;
   // The increment is the upper 24 bits of the hash, forced to be odd so all
   // possible keys will be hit.
   unsigned increment = ((hash >> 8) & 0xFFFFFF) | 1;
   UtlString* userString;

   mMapLock.acquire();

   // First, check if it's already mapped.
   UtlContainable* v = mMapContactsToUser.findValue(contactString);
   if (v)
   {
      userString = dynamic_cast<UtlString*> (v);
   }
   else
   {
      // Find an unused key value.
      while (1)
      {
         char buffer[20];
         sprintf(buffer, "=Gateway=%03x-%03x", (hash >> 12) & 0xFFF, hash & 0xFFF);
         userString = new UtlString(buffer);
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "%s::addMapping trying '%s'",
                       mLogName.data(), buffer);
         if (mMapUserToContacts.findValue(userString) == NULL)
         {
            break;
         }
         delete userString;
         key += increment;
         key &= 0xFFFFFF;
      }
      // Insert the mapping.
      mMapUserToContacts.insertKeyAndValue(userString, contactString);
      mMapContactsToUser.insertKeyAndValue(contactString, userString);

      mMapsModified = TRUE;
   }

   mMapLock.release();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::addMapping using '%s'",
                 mLogName.data(), userString->data());

   return userString;
}

const char* form =
"<http>\n"
"<head>\n"
"<title>Gateway Server - Configure a gateway prefix</title>\n"
"</head>\n"
"<body>\n"
"<h1>Gateway Server - Configure a gateway prefix</h1>\n"
"%s\n"
"<form action='/map.html' method=post enctype='multipart/form-data'>\n"
"Routing prefix: "
"<input name=prefix type=string size=4></input> "
"(format is '9ggx', where 'gg' is your group number and 'x' is 1 to 9)<br />\n"
"<input type=submit name=get value='Get gateway' /><br />\n"
"Gateway hostport: "
"<input name=destination type=string size=30></input> "
"<input type=submit name=set value='Set gateway' />\n"
"</form>\n"
"</body>\n"
"</html>\n";

void
SipRedirectorGateway::displayForm(const HttpRequestContext& requestContext,
                              const HttpMessage& request,
                              HttpMessage*& response)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::displayForm entered", mLogName.data());

   UtlString method;
   request.getRequestMethod(&method);

   if (method.compareTo("POST") == 0)
   {
      processForm(requestContext, request, response);
   }
   else
   {
      response = new HttpMessage();

      // Send 200 OK reply.
      response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                           HTTP_OK_CODE,
                                           HTTP_OK_TEXT);
      // Insert the HTML into the response.
      HttpBody* body = new HttpBody(form, -1, CONTENT_TYPE_TEXT_HTML);
      response->setBody(body);
   }
}

void
SipRedirectorGateway::processForm(const HttpRequestContext& requestContext,
                              const HttpMessage& request,
                              HttpMessage*& response)
{
   UtlString string_get("get");
   UtlString string_set("set");
   UtlString string_prefix("prefix");
   UtlString string_destination("destination");

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::processForm entered", mLogName.data());
   UtlString* user;

   // Process the request.

   // Get the body of the request.
   const HttpBody* request_body = request.getBody();

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::processForm A *** request body is '%s'",
                 mLogName.data(), request_body->getBytes());

   // Get the values from the form.
   if (request_body->isMultipart())
   {
      // Extract the values from the form data.
      UtlHashMap values;
      int c = request_body->getMultipartCount();
      for (int i = 0; i < c; i++)
      {
         UtlString* name = new UtlString;
         if (request_body->getMultipart(i)->getPartHeaderValue("name", *name))
         {
            const char* v;
            int l;
            request_body->getMultipartBytes(i, &v, &l);
            // Strip leading and trailing whitespace from values.
            UtlString* value = new UtlString(v, l);
            value->strip(UtlString::both);
            OsSysLog::add(FAC_SIP, PRI_CRIT,
                          "%s::processForm "
                          "form value '%s' = '%s'",
                          mLogName.data(), name->data(), value->data());
            // 'name' and 'value' are now owned by 'values'.
            values.insertKeyAndValue(name, value);
         }
         else
         {
            // 'name' is not owned by 'values' and we have to delete it.
            delete name;
         }
      }

      if (values.findValue(&string_get))
      {
         // This is a "get gateway" request.

         // Insert the HTML into the response.
         HttpBody* response_body = new HttpBody(form, -1, CONTENT_TYPE_TEXT_HTML);
         response->setBody(response_body);
      }
      else if (values.findValue(&string_set))
      {
         // This is a "set gateway" request.

         // Validate the routing prefix.
         UtlString* prefix =
            dynamic_cast <UtlString*> (values.findValue(&string_prefix));
         if (prefixIsValid(*prefix))
         {
            // Validate the destination.
            UtlString* destination =
               dynamic_cast <UtlString*>
                   (values.findValue(&string_destination));
            if (destination_is_valid(destination))
            {
               OsSysLog::add(FAC_SIP, PRI_CRIT,
                             "%s::processForm "
                             "add mapping '%s' -> '%s'",
                             mLogName.data(), prefix->data(), destination->data());

               mMapLock.acquire();
               // Insert the mapping.
               mMapUserToContacts.insertKeyAndValue(prefix, destination);
               mMapContactsToUser.insertKeyAndValue(destination, prefix);
               mMapLock.release();

               writeMappings();
            }
         }
         // Insert the HTML into the response.
         HttpBody* response_body = new HttpBody(form, -1, CONTENT_TYPE_TEXT_HTML);
         response->setBody(response_body);
      }
      else
      {
         // Incomprehensible request.

         // Insert the HTML into the response.
         HttpBody* response_body = new HttpBody(form, -1, CONTENT_TYPE_TEXT_HTML);
         response->setBody(response_body);
      }
   }
   else
   {
      // Incomprehensible request.

      // Insert the default HTML into the response.
      HttpBody* response_body = new HttpBody(form, -1, CONTENT_TYPE_TEXT_HTML);
      response->setBody(response_body);
   }

#if 0

#if 0
   // This is quite a chore, because getMultipartBytes gets the entire
   // multipart section, including the trailing delimiter, rather than just
   // the body, which is what we need.
   const char* value;
   int length;
   request_body->getMultipartBytes(0, &value, &length);
#if 0
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::processForm A *** seeing '%.*s'", mLogName.data(), length, value);
#endif
   // Advance 'value' over the first \r\n\r\n, which ends the headers.
   const char* s = strstr(value, "\r\n\r\n");
   if (s)
   {
      s += 4;                   // Allow for length of \r\n\r\n.
      length -= s - value;
      value = s;
   }
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::processForm B *** seeing '%.*s'", mLogName.data(), length, value);
#if 0
   // Search backward for the last \r, excepting the one in the second-to-last
   // position, which marks the end of the contents.
   if (length >= 3)
   {
      for (s = value + length - 3;
           !(s == value || *s == '\r');
           s--)
      {
         /* empty */
      }
      length = s - value;
   }
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::processForm seeing '%.*s'", mLogName.data(), length, value);
#endif

   // Add the mappings.
   const char* error_msg;
   int error_location;
   UtlBoolean success =
      addMappings(value, length, user, error_msg, error_location);

   // Construct the response.

   response = new HttpMessage();

   // Send 200 OK reply.
   response->setResponseFirstHeaderLine(HTTP_PROTOCOL_VERSION,
                                        HTTP_OK_CODE,
                                        HTTP_OK_TEXT);
   // Construct the HTML.
   char buffer1[100];
#if 0
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::processForm *** domain '%s'",
                 mLogName.data(), Gatewayredirector->mDomainName.data());
#endif
   if (success)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::processForm success user '%s'",
                    mLogName.data(), user->data());
      sprintf(buffer1, "<code>sip:<font size=\"+1\">%s</font>@%s:65070</code> redirects to:<br />",
              user->data(), Gatewayredirector->mDomainName.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::processForm failure error_msg '%s', error_location %d",
                    mLogName.data(), error_msg, error_location);
      strcpy(buffer1, "<i>Error:</i>");
   }
   // Transcribe the input value into buffer2.
   char buffer2[FORM_SIZE];
   char* p;
   int i;
   if (success)
   {
      // An impossible location.
      error_location = -1;
   }
   for (p = buffer2, i = 0;
        ;
        i++)
   {
      // If this is the error location, insert the error message.
      if (i == error_location)
      {
         *p++ = '!';
         *p++ = '-';
         *p++ = '-';
         strcpy(p, error_msg);
         p += strlen(error_msg);
         *p++ = '-';
         *p++ = '-';
         *p++ = '!';
      }
      // Test for ending the loop after testing to insert the error message,
      // because the error message may be after the last character.
      if (i >= length)
      {
         break;
      }
      switch (value[i])
      {
      case '<':
         *p++ = '&';
         *p++ = 'l';
         *p++ = 't';
         *p++ = ';';
         break;
      case '>':
         *p++ = '&';
         *p++ = 'g';
         *p++ = 't';
         *p++ = ';';
         break;
      case '&':
         *p++ = '&';
         *p++ = 'a';
         *p++ = 'm';
         *p++ = 'p';
         *p++ = ';';
         break;
      default:
         *p++ = value[i];
         break;
      }
   }
   *p++ = '\0';
#endif

#endif

}

// Construct mappings from an input string.
UtlBoolean
SipRedirectorGateway::addMappings(const char* value,
                                  int length,
                                  UtlString*& user,
                                  const char*& error_msg,
                                  int& location)
{
   // Process the input string one character at a time.
   // Buffer into which to edit the input string.
   char buffer[FORM_SIZE];
   // Pointer for filling the edit buffer.
   char* p = buffer;
   char c;
   int count = length;
   while (count-- > 0)
   {
      c = *value++;
      switch (c)
      {
      case '\n':
         // Trim trailing whitespace on the line.
         while (p > buffer && p[-1] != '\n' && isspace(p[-1]))
         {
            p--;
         }
         // Fall through to insert if not at the beginning of a line.
      case ' ':
      case '\t':
         // If at the beginning of a line, ignore it.
         if (p > buffer && p[-1] != '\n' && p[-1] != '{')
         {
            *p++ = c;
         }
         break;
      case '}':
      {
         // Process component redirection.
         // Trim trailing whitespace.
         while (p > buffer && isspace(p[-1]))
         {
            p--;
         }
         // Find the matching '{'.
         *p = '\0';             // End scope of strrchr.
         char* open = strrchr(buffer, '{');
         if (open == NULL)
         {
            error_msg = "Unmatched '}'";
            location = length - count;
            return FALSE;
         }
         if (open+1 == p)
         {
            error_msg = "No contacts given";
            location = length - count;
            return FALSE;
         }
         // Insert the addresses into the map and get the assigned user name.
         UtlString* user = addMapping(open+1, p - (open+1));
         // Truncate off the sub-redirection.
         p = open;
         // Append the resulting user name.
         strcpy(p, user->data());
         p += strlen(user->data());
      }
         break;
      case '{':
         // '{' is copied into the buffer like other characters.
      default:
         // Ordinary characters are just copied.
         *p++ = c;
         break;
      }
   }
   // Trim trailing whitespace.
   while (p > buffer && isspace(p[-1]))
   {
      p--;
   }
   // Check that there are no unclosed '{'.
   *p = '\0';                   // To limit strchr's search.
   if (strchr(buffer, '{') != NULL)
   {
      error_msg = "Unmatched '{'";
      // Report at end of string because we have no better choice.
      location = length;
      return FALSE;
   }
   // Check that the contacts are not empty.
   if (p == buffer)
   {
      error_msg = "No contacts given";
      location = 0;
      return FALSE;
   }
   // Insert the addresses into the map and return the assigned user name.
   user = addMapping(buffer, p - buffer);
   return TRUE;
}

// Constructor for the writer task.
GatewayWriterTask::GatewayWriterTask(void* pArg) :
   OsTask("GatewayWriter", pArg, OsTask::DEF_PRIO,
          OsTask::DEF_OPTIONS, OsTask::DEF_STACKSIZE)
{
}

// Running code of the writer task.
int GatewayWriterTask::run(void* arg)
{
   // Get the pointer to the redirector.
   mpRedirector = (SipRedirectorGateway*) (arg);

   // Loop forever.
   while (!isShuttingDown())
   {
      // Wait 1 second between iterations.
      delay(1000);

      // Check whether the maps need to be written out.
      mpRedirector->mMapLock.acquire();
      UtlBoolean must_write = mpRedirector->mMapsModified;
      mpRedirector->mMapLock.release();

      if (must_write)
      {
         // writeMappings seizes the lock itself.
         mpRedirector->writeMappings(&mpRedirector->mMappingFileName,
                                     &mpRedirector->mMapUserToContacts);
      }
   }
   return 0;
}

UtlBoolean SipRedirectorGateway::prefixIsValid(UtlString& p)
{
   int len, digits;
   char last;

   return
      (len = mPrefix.length(),
       // Initial 'len' chars must be the prefix.
       strncmp(p.data(), mPrefix.data(), len) == 0 &&
       (digits = strspn(p.data() + len, "0123456789"),
        // Remaining chars must be digits.
        p.length() == len + digits &&
        (last = p.length(len - 1),
         // Last digit must be 1-9.
         last >= 1)));
}
