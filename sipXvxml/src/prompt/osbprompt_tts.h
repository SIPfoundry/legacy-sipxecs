//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

#ifndef _OSBpromptTTS_h_
#define _OSBpromptTTS_h_

// SYSTEM INCLUDES
#include <string>

// APPLICATION INCLUDES
#include "VXIvalue.h"
#include "os/OsDefs.h"
#include "os/OsStatus.h"
#include "net/Url.h"


// DEFINES
// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
// STRUCTS
// TYPEDEFS
typedef std::basic_string<VXIchar> vxistring;

// FORWARD DECLARATIONS

//:This is the class that provides utility functions for the version check
// application.

class OSBpromptTTS
{
/* //////////////////////////// PUBLIC //////////////////////////////////// */
  public:

/* ============================ CREATORS ================================== */

   OSBpromptTTS(const VXIchar* text, const VXIchar* baseUrl, const VXIchar* glbBaseUrl);
   //:Default constructor
   // text = XML containing text string to be rendered into audio
   // baseUrl = URL of document from which text came
   // glbBaseUrl = URL of directory of standard prompt files

   virtual
      ~OSBpromptTTS();
   //:Destructor

/* ============================ MANIPULATORS ============================== */


/* ============================ ACCESSORS ================================= */

   int getPromptUrls(Url** prompts);
   //: Returns the prompt urls constructed from the text
   //!param: (out) urls - prompt urls.
   //!retcode: number of urls.

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */
  protected:

/* //////////////////////////// PRIVATE /////////////////////////////////// */
  private:

   int                               mCount;
   // The URL of the directory of prompts to be used to render the text into audio.
   UtlString          mBaseUrl;
   UtlString          mText;
   UtlString          mFormat;
   UtlString          mLocale;
   Url               *mPrompts;
   UtlBoolean         mDayFirst;

   OSBpromptTTS(const OSBpromptTTS& rOSBpromptTTS);
   //:Copy constructor
   OSBpromptTTS& operator=(const OSBpromptTTS& rhs);
   //:Assignment operator

   void parse(const VXIchar* text);
   void parseBaseUrl(const VXIchar* url, const VXIchar* glbBaseUrl);

   int getDigitUrls(Url** prompts);
   int getOrdinalUrls(Url** prompts);
   void getOrdinalUrlHelper(UtlString& text, UtlString& url);

   int getNumberUrls(Url** prompts);

   int getPhoneUrls(Url** prompts);

   int getDateMDYUrls(Url** prompts);
   int getDateMDUrls(Url** prompts);
   int getDateDMYUrls(Url** prompts);
   int getDateDMUrls(Url** prompts);
   int getDateMYUrls(Url** prompts);
   int getDateYMUrls(Url** prompts);
   int getDateDUrls(Url** prompts);
   int getDateMUrls(Url** prompts);
   int getDateYUrls(Url** prompts);
   int getDayOfTheWeekUrls(Url** prompts);

   /* Turn time values into a string of prompts. */
   /* mText = "hh:mm:ss" */
   int getTimeHMSUrls(Url** prompts);
   /* mText = "hh:mm" */
   int getTimeHMUrls(Url** prompts);
   /* mText = "hh" */
   int getTimeHUrls(Url** prompts);
   int getDurationHMSUrls(Url** prompts);
   int getDurationHMUrls(Url** prompts);
   int getDurationMSUrls(Url** prompts);
   int getDurationHUrls(Url** prompts);
   int getDurationMUrls(Url** prompts);
   int getDurationSUrls(Url** prompts);

   int getAcronymUrls(Url** prompts);

   int getDateFrom(UtlString& mon, UtlString& day, UtlString& year, Url** prompts);
   /* Turn a time value into a string of prompts.
    * h, m, and s are the hours, minutes, and seconds components of the time.
    * Each is either absent (a null string), or two digits.
    * h must be present, m must be present if s is present.
    */
   int getTimeFrom(UtlString& h,
                   UtlString& m,
                   UtlString& s,
                   Url** prompts);
   int getDurationFrom(UtlString& h,
                       UtlString& m,
                       UtlString& s,
                       Url** prompts);



#ifdef TEST
   static bool sIsTested;
   //:Set to true after the tests for this class have been executed once

   void test();
   //:Verify assertions for this class

   // Test helper functions
   void testCreators();
   void testManipulators();
   void testAccessors();
   void testInquiry();

#endif //TEST
};

/* ============================ INLINE METHODS ============================ */

#endif  // _OSBpromptTTS_h_
