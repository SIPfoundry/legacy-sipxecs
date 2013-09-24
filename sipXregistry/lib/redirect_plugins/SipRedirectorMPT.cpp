//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include <utl/UtlRegex.h>
#include "os/OsDateTime.h"
#include "os/OsSysLog.h"
#include "sipdb/SIPDBManager.h"
#include "sipdb/PermissionDB.h"
#include "sipdb/ResultSet.h"
#include "SipRedirectorMPT.h"
#include "utl/UtlHashMapIterator.h"
#include "utl/XmlContent.h"

// DEFINES
// File containing the MPT mappings (in config dir).
#define MPT_FILE_NAME "MPT.xml"

// How big the forms on the Web page can get.
#define FORM_SIZE 10240

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS

// Static pointer to the unique SipRedirectorMPT object.
SipRedirectorMPT* MPTredirector;

// Static factory function.
extern "C" RedirectPlugin* getRedirectPlugin(const UtlString& instanceName)
{
   return new SipRedirectorMPT(instanceName);
}

// Constructor
SipRedirectorMPT::SipRedirectorMPT(const UtlString& instanceName) :
   RedirectPlugin(instanceName),
   mMapLock(OsBSem::Q_FIFO, OsBSem::FULL),
   writerTask(this)
{
   mLogName.append("[");
   mLogName.append(instanceName);
   mLogName.append("] SipRedirectorMPT");
}

// Destructor
SipRedirectorMPT::~SipRedirectorMPT()
{
}

// Read config information.
void SipRedirectorRegDB::readConfig(OsConfigDb& configDb)
{
   configDb.get("MAPPING_FILE", mMappingFileName);
}

// Initialize
OsStatus
SipRedirectorMPT::initialize(OsConfigDb& configDb,
                             int redirectorNo,
                             const UtlString& localDomainHost)
{
   mDomainName = localDomainHost;
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::SipRedirectorMPT domainName = '%s'", mLogName.data(),
                 mDomainName.data());

   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::SipRedirectorMPT Loading mappings from '%s'", mLogName.data(),
                 mMappingFileName.data());
   loadMappings(&mMappingFileName, &mMapUserToContacts, &mMapContactsToUser);

   // Set up the static pointer to the unique instance.
   MPTredirector = this;

   // Set up the HTTP server on socket 65008.
   mpSocket = new OsServerSocket(50, 65008);
   mpServer = new HttpServer(socket);
   mpServer->addRequestProcessor("/map.html", &displayForm);
   mpServer->start();

   // Start the writer task.
   writerTask.start();

   return OS_SUCCESS;
}

// Finalize
void
SipRedirectorMPT::finalize()
{
}

SipRedirector::LookUpStatus
SipRedirectorMPT::lookUp(
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
                 "userId = '%s'", mLogName.data(),
                 userId.data());

   // Look up the user ID in the map.
   mMapLock.acquire();
   UtlContainable* v = mMapUserToContacts.findValue(&userId);
   mMapLock.release();
   // If found, add the contacts.
   if (v)
   {
      // Extract all the contacts out of the contact string.
      const char* s;
      const char* s1;
      for (s = (dynamic_cast<UtlString*> (v))->data();
           *s != '\0';
           s = s1+1)
      {
         // Find the ending newline, if any.
         // (Beware that Tiny XML trims trailing newlines off text contents!)
         s1 = strchr(s, '\n');
         if (!s1)
         {
            s1 = s + strlen(s);
         }
         // Ignore it if it is null
         if (s1-s != 0)
         {
            // Construct a UtlString of this contact.
            UtlString c(s, s1-s);
            // Construct a Url of this contact.
            Url url(c.data(), FALSE);
            // Add the contact.
            contactList.add( url, *this );
         }
      }
   }

   return SipRedirector::SUCCESS;
}

void SipRedirectorMPT::loadMappings(UtlString* file_name,
                                    UtlHashMap* mapUserToContacts,
                                    UtlHashMap* mapContactsToUser)
{
    // Load the XML file.
    TiXmlDocument *mDoc = new TiXmlDocument(file_name->data());
    if (mDoc->LoadFile())
    {
       // Look at the top element, which should be "MPT".
       TiXmlNode* MPTNode = mDoc->FirstChild("MPT");
       if (MPTNode)
       {
          mMapLock.acquire();

          for (TiXmlNode* mapNode = NULL;
               (mapNode = MPTNode->IterateChildren("map", mapNode));
             )
          {
             // Carefully get the <user> and <content> text.
             TiXmlNode* c1 = mapNode->FirstChild("user");
             if (!c1)
             {
                OsSysLog::add(FAC_SIP, PRI_ERR,
                              "%s::loadMappings cannot find <user> child", mLogName.data());
             }
             else
             {
                TiXmlNode* c2 = c1->FirstChild();
                if (!c2)
                {
                   OsSysLog::add(FAC_SIP, PRI_ERR,
                                 "%s::loadMappings cannot find text child of <user>", mLogName.data());
                }
                else
                {
                   const char* user = c2->Value();
                   if (user == NULL || *user == '\0')
                   {
                      OsSysLog::add(FAC_SIP, PRI_ERR,
                                    "%s::loadMappings text of <user> is null", mLogName.data());
                   }
                   else
                   {
                      TiXmlNode* c3 = mapNode->FirstChild("contacts");
                      if (!c3)
                      {
                         OsSysLog::add(FAC_SIP, PRI_ERR,
                                       "%s::loadMappings cannot find <contacts> child", mLogName.data());
                      }
                      else
                      {
                         TiXmlNode* c4 = c3->FirstChild();
                         if (!c4)
                         {
                            OsSysLog::add(FAC_SIP, PRI_ERR,
                                          "%s::loadMappings cannot find text child of <contacts>", mLogName.data());
                         }
                         else
                         {
                            const char* contact = c4->Value();
                            if (contact == NULL || *contact == '\0')
                            {
                               OsSysLog::add(FAC_SIP, PRI_ERR,
                                             "%s::loadMappings text of <contacts> is null", mLogName.data());
                            }
                            else
                            {
                               // Load the mapping into the maps.
                               OsSysLog::add(FAC_SIP, PRI_DEBUG,
                                             "%s::loadMappings added '%s' -> '%s'", mLogName.data(),
                                             user, contact);
                               UtlString* user_string = new UtlString(user);
                               UtlString* contact_string = new UtlString(contact);
                               mapUserToContacts->insertKeyAndValue(user_string,
                                                                    contact_string);
                               mapContactsToUser->insertKeyAndValue(contact_string,
                                                                    user_string);
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
                        "%s::loadMappings unable to extract MPT element", mLogName.data());
       }

    }
    else
    {
       OsSysLog::add(FAC_SIP, PRI_CRIT,
                     "%s::loadMappings LoadFile() failed", mLogName.data());
    }
}

void SipRedirectorMPT::writeMappings(UtlString* file_name,
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
                    "%s::writeMappings fopen('%s') failed, errno = %d '%s'",
                    mLogName.data(), temp_file_name.data(),
                    errno, strerror(errno));
   }
   else
   {
      mMapLock.acquire();

      fprintf(f, "<MPT>\n");
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
         fprintf(f, "  <map><user>%s</user><contacts>%s</contacts></map>\n",
                 k_escaped.data(), v_escaped.data());
      }
      fprintf(f, "</MPT>\n");
      fclose(f);

      mMapsModified = FALSE;

      mMapLock.release();

      if (rename(temp_file_name.data(), file_name->data()) != 0)
      {
         OsSysLog::add(FAC_SIP, PRI_CRIT,
                       "%s::writeMappings rename('%s', '%s') failed, errno = %d '%s'",
                       mLogName.data(), temp_file_name.data(), file_name->data(),
                       errno, strerror(errno));
      }
   }
}

UtlString* SipRedirectorMPT::addMapping(const char* contacts,
                                        int length)
{
   // Make the contact UtlString.
   UtlString* contactString = new UtlString(contacts, length);
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::addMapping inserting '%s'", mLogName.data(),
                 contactString->data());
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
         sprintf(buffer, "=MPT=%03x-%03x", (hash >> 12) & 0xFFF, hash & 0xFFF);
         userString = new UtlString(buffer);
         OsSysLog::add(FAC_SIP, PRI_DEBUG,
                       "%s::addMapping trying '%s'", mLogName.data(),
                       buffer);
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
                 "%s::addMapping using '%s'", mLogName.data(),
                 userString->data());

   return userString;
}

const char* form =
"<http>\n"
"<head>\n"
"<title>MPT Server - Create a redirection</title>\n"
"</head>\n"
"<body>\n"
"<h1>MPT Server - Create a redirection</h1>\n"
"%s\n"
"<form action='/map.html' method=post enctype='multipart/form-data'>\n"
"<textarea name=t cols=60 rows=10>%s</textarea><br />\n"
"<input type=submit value='Create redirection' />\n"
"</form>\n"
"<br />\n"
"<br />\n"
"<i>For assistance, contact <a href=\"mailto:dworley@pingtel.com\">Dale Worley at Pingtel</a>.<i>\n"
"</body>\n"
"</html>\n";

void
SipRedirectorMPT::displayForm(const HttpRequestContext& requestContext,
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
      // Construct the HTML.
      char buffer[FORM_SIZE];
      sprintf(buffer, form, "", "Enter SIP URIs separated by newlines.");
      // Insert the HTML into the response.
      HttpBody* body = new HttpBody(buffer, -1, CONTENT_TYPE_TEXT_HTML);
      response->setBody(body);
   }
}

void
SipRedirectorMPT::processForm(const HttpRequestContext& requestContext,
                              const HttpMessage& request,
                              HttpMessage*& response)
{
   OsSysLog::add(FAC_SIP, PRI_DEBUG,
                 "%s::processForm entered", mLogName.data());
   UtlString* user;

   // Process the request.

   // Get the body of the request.
   const HttpBody* request_body = request.getBody();

   // Get the value from the form.
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
      MPTredirector->addMappings(value, length, user, error_msg,
                                 error_location);

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
                 "%s::processForm *** domain '%s'", mLogName.data(),
                 MPTredirector->mDomainName.data());
#endif
   if (success)
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::processForm success user '%s'", mLogName.data(),
                    user->data());
      sprintf(buffer1, "<code>sip:<font size=\"+1\">%s</font>@%s:65070</code> redirects to:<br />",
              user->data(), MPTredirector->mDomainName.data());
   }
   else
   {
      OsSysLog::add(FAC_SIP, PRI_DEBUG,
                    "%s::processForm failure error_msg '%s', error_location %d", mLogName.data(),
                    error_msg, error_location);
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
   char buffer[FORM_SIZE];
   sprintf(buffer, form, buffer1, buffer2);
   // Insert the HTML into the response.
   HttpBody* response_body = new HttpBody(buffer, -1, CONTENT_TYPE_TEXT_HTML);
   response->setBody(response_body);
}

// Construct mappings from an input string.
UtlBoolean
SipRedirectorMPT::addMappings(const char* value,
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
         UtlString* user = MPTredirector->addMapping(open+1, p - (open+1));
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
   user = MPTredirector->addMapping(buffer, p - buffer);
   return TRUE;
}

// Constructor for the writer task.
MPTWriterTask::MPTWriterTask(void* pArg) :
   OsTask("MPT-Writer", pArg, OsTask::DEF_PRIO, OsTask::DEF_OPTIONS,
          OsTask::DEF_STACKSIZE)
{
}

// Running code of the writer task.
int MPTWriterTask::run(void* arg)
{
   // Get the pointer to the redirector.
   SipRedirectorMPT* redirector = (SipRedirectorMPT*) (arg);

   // Loop forever.
   while (1)
   {
      // Wait 5 seconds between iterations.
      delay(5000);

      // Check whether the maps need to be written out.
      redirector->mMapLock.acquire();
      UtlBoolean must_write = redirector->mMapsModified;
      redirector->mMapLock.release();

      if (must_write)
      {
         // writeMappings seizes the lock itself.
         redirector->writeMappings(&redirector->configFileName,
                                   &redirector->mMapUserToContacts);
      }
   }

   return 0;
}

const UtlString& SipRedirectorMPT::name( void ) const
{
   return mLogName;
}
