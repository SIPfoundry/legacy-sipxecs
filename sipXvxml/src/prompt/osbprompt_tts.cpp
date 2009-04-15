//
//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

#ifdef TEST
#include <assert.h>
#include "utl/UtlMemCheck.h"
#endif //TEST
#include <ctype.h>
#include <stdio.h>
#include <time.h>

#define RT_FEEDBACK_ON_PHONE

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "os/OsFileSystem.h"
#include "osbprompt_tts.h"
#include "xmlparser/tinyxml.h"
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
#define PROMPT_DIR "prompts/"

const char *MONTH[12] = {"january.wav", "february.wav", "march.wav", "april.wav",
                         "may.wav", "june.wav", "july.wav", "august.wav",
                         "september.wav", "october.wav", "november.wav", "december.wav" };

const char *TIME[9] = { "o_one.wav", "o_two.wav", "o_three.wav", "o_four.wav",
                        "o_five.wav", "o_six.wav", "o_seven.wav", "o_eight.wav", "o_nine.wav" };


const char *DAY[7] = {  "sunday.wav", "monday.wav", "tuesday.wav", "wednesday.wav",
                        "thursday.wav", "friday.wav", "saturday.wav" };

// Note: for now we only handle the number below 999 trillion
const char *THOUSAND[4] = { "thousand", "million", "billion", "trillion" };

// STATIC VARIABLE INITIALIZATIONS


/* //////////////////////////// PUBLIC //////////////////////////////////// */

/* ============================ CREATORS ================================== */

// Constructor
// text = XML containing text string to be rendered into audio
// baseUrl = URL of document from which text came
// glbBaseUrl = URL of directory of standard prompt files
OSBpromptTTS::OSBpromptTTS(const VXIchar* text, const VXIchar* baseUrl, const VXIchar* glbBaseUrl)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::OSBpromptTTS: text = '%ls', baseUrl = '%ls', glbBaseUrl = '%ls'",
                 text, baseUrl, glbBaseUrl);
#ifdef TEST
   if (!sIsTested)
   {
      sIsTested = true;
      test();
   }
#endif //TEST

   mPrompts = 0;
   mDayFirst = FALSE;
   // Extract the text to be rendered from the XML.
   parse(text);
   // Set mBaseUrl, the directory of prompts to be used.
   parseBaseUrl(baseUrl, glbBaseUrl);
   if ((mFormat.index("date:dm") != UtlString::UTLSTRING_NOT_FOUND) ||
       (mFormat.index("date:md") != UtlString::UTLSTRING_NOT_FOUND))
   {
      // Determine the date format to be used for this language. By default, the
      // MDY order is used - if the file DATE_DMY exists in the directory with
      // voice prompts, the DMY order is used.
      static char urlPrefix[] = "file://";
      if (mBaseUrl.index(urlPrefix) == 0)
      {
         UtlString baseDirectory = mBaseUrl;
         baseDirectory.remove(0, sizeof(urlPrefix) - 1);
         OsFile dmyFile(baseDirectory + "DATE_DMY");
         mDayFirst = dmyFile.exists();
      }
   }
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::OSBpromptTTS: mBaseUrl = '%s'",
                 mBaseUrl.data());
}

// Copy constructor
OSBpromptTTS::OSBpromptTTS(const OSBpromptTTS& rOSBpromptTTS)
{
   mCount = rOSBpromptTTS.mCount;
   mBaseUrl = rOSBpromptTTS.mBaseUrl;
   mText = rOSBpromptTTS.mText;
   mFormat = rOSBpromptTTS.mFormat;
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::OSBpromptTTS(copy): mCount = %d, mBaseUrl = '%s', mText = '%s', mFormat = '%s'",
                 mCount, mBaseUrl.data(), mText.data(), mFormat.data());
}

// Destructor
OSBpromptTTS::~OSBpromptTTS()
{
   if (mPrompts)
   {
      delete[] mPrompts;
      mPrompts = 0;
   }
}

/* ============================ MANIPULATORS ============================== */

// Assignment operator
OSBpromptTTS&
OSBpromptTTS::operator=(const OSBpromptTTS& rhs)
{
   if (this == &rhs)            // handle the assignment to self case
      return *this;

   mCount = rhs.mCount;
   mBaseUrl = rhs.mBaseUrl;
   mText = rhs.mText;
   mFormat = rhs.mFormat;
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::OSBpromptTTS(assignment): mCount = %d, mBaseUrl = '%s', mText = '%s', mFormat = '%s'",
                 mCount, mBaseUrl.data(), mText.data(), mFormat.data());

   return *this;
}


/* ============================ ACCESSORS ================================= */
int OSBpromptTTS::getPromptUrls(Url** prompts)
{
   int count = 0;

   if (mFormat.compareTo("number:digits", UtlString::ignoreCase) == 0)
   {
      count = getDigitUrls(prompts);
   }
   else if (mFormat.compareTo("number:ordinal", UtlString::ignoreCase) == 0)
   {
      count = getOrdinalUrls(prompts);
   }
   else if (mFormat.compareTo("number", UtlString::ignoreCase) == 0)
   {
      count = getNumberUrls(prompts);
   }
   else if (mFormat.compareTo("digits", UtlString::ignoreCase) == 0)
   {
      count = getDigitUrls(prompts);
   }
   else if (mFormat.compareTo("date:mdy", UtlString::ignoreCase) == 0)
   {
      count = getDateMDYUrls(prompts);
   }
   else if (mFormat.compareTo("date:md", UtlString::ignoreCase) == 0)
   {
      count = getDateMDUrls(prompts);
   }
   else if (mFormat.compareTo("date:dmy", UtlString::ignoreCase) == 0)
   {
      count = getDateDMYUrls(prompts);
   }
   else if (mFormat.compareTo("date:dm", UtlString::ignoreCase) == 0)
   {
      count = getDateDMUrls(prompts);
   }
   else if (mFormat.compareTo("date:my", UtlString::ignoreCase) == 0)
   {
      count = getDateMYUrls(prompts);
   }
   else if (mFormat.compareTo("date:ym", UtlString::ignoreCase) == 0)
   {
      count = getDateYMUrls(prompts);
   }
   else if (mFormat.compareTo("date:d", UtlString::ignoreCase) == 0)
   {
      count = getDateDUrls(prompts);
   }
   else if (mFormat.compareTo("date:w", UtlString::ignoreCase) == 0)
   {
      count = getDayOfTheWeekUrls(prompts);
   }
   else if (mFormat.compareTo("date:m", UtlString::ignoreCase) == 0)
   {
      count = getDateMUrls(prompts);
   }
   else if (mFormat.compareTo("date:y", UtlString::ignoreCase) == 0)
   {
      count = getDateYUrls(prompts);
   }
   else if (mFormat.compareTo("date", UtlString::ignoreCase) == 0)
   {
      count = getDateMDYUrls(prompts);
   }
   else if (mFormat.compareTo("time:hms", UtlString::ignoreCase) == 0)
   {
      count = getTimeHMSUrls(prompts);
   }
   else if (mFormat.compareTo("time:hm", UtlString::ignoreCase) == 0)
   {
      count = getTimeHMUrls(prompts);
   }
   else if (mFormat.compareTo("time:h", UtlString::ignoreCase) == 0)
   {
      count = getTimeHUrls(prompts);
   }
   else if (mFormat.compareTo("time", UtlString::ignoreCase) == 0)
   {
      count = getTimeHMSUrls(prompts);
   }
   else if (mFormat.compareTo("duration:hms", UtlString::ignoreCase) == 0)
   {
      count = getDurationHMSUrls(prompts);
   }
   else if (mFormat.compareTo("duration:hm", UtlString::ignoreCase) == 0)
   {
      count = getDurationHMUrls(prompts);
   }
   else if (mFormat.compareTo("duration:ms", UtlString::ignoreCase) == 0)
   {
      count = getDurationMSUrls(prompts);
   }
   else if (mFormat.compareTo("duration:h", UtlString::ignoreCase) == 0)
   {
      count = getDurationHUrls(prompts);
   }
   else if (mFormat.compareTo("duration:m", UtlString::ignoreCase) == 0)
   {
      count = getDurationMUrls(prompts);
   }
   else if (mFormat.compareTo("duration:s", UtlString::ignoreCase) == 0)
   {
      count = getDurationSUrls(prompts);
   }
   else if (mFormat.compareTo("duration", UtlString::ignoreCase) == 0)
   {
      count = getDurationHMSUrls(prompts);
   }
   else if (mFormat.compareTo("telephone", UtlString::ignoreCase) == 0)
   {
      count = getPhoneUrls(prompts);
   }
   else if (mFormat.compareTo("acronym", UtlString::ignoreCase) == 0)
   {
      count = getAcronymUrls(prompts);
   }
   else
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_ERR,
                    "OSBpromptTTS::getPromptUrls: Unknown format:  mFormat = '%s'",
                    mFormat.data());
   }

   return count;
}

/* ============================ INQUIRY =================================== */

/* //////////////////////////// PROTECTED ///////////////////////////////// */

/* //////////////////////////// PRIVATE /////////////////////////////////// */
void OSBpromptTTS::parse(const VXIchar* txt)
{
   TiXmlDocument doc( "speak.xml" );
   vxistring text = txt;
   int len = text.length();
   char* str = new char[len + 1];

   for (int i = 0; i < len; i++)
      str[i] = text[i];
   str[len] = 0;

   doc.Parse( str );

   TiXmlElement* ucElement = 0;

   // It is a child of the document, and can be selected by name.
   TiXmlNode* node = doc.FirstChild( "speak" );
   if (node)
   {
      ucElement = node->ToElement();
      mLocale = ucElement->Attribute("xml:lang");
      TiXmlNode* subnode = ucElement->FirstChild( "say-as" );
      if (subnode)
      {
         ucElement = subnode->ToElement();
         TiXmlNode* txtnode = ucElement->FirstChild();
         if (txtnode && txtnode->Type() == TiXmlNode::TEXT)
         {
            TiXmlText* text = txtnode->ToText();
            if (text)
            {
               mText =  UtlString(text->Value());
               mText = mText.strip(UtlString::both);
               mFormat = ucElement->Attribute("type");
            }
         }
      }
   }

   if (str)
   {
      delete[] str;
      str = NULL;
   }

}


// Set up mBaseUrl.
// baseUrl = URL of document from which text came
// glbBaseUrl = URL of directory of standard prompt files
void OSBpromptTTS::parseBaseUrl(const VXIchar* url, const VXIchar* glbBaseUrl)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::parseBaseUrl: mBaseUrl(before) = '%s', url = '%ls', glbBaseUrl = '%ls'",
                 mBaseUrl.data(), url, glbBaseUrl);

   // If glbBaseUrl is provided, convert it into 1-byte characters and
   // then a UtlString to use as mBaseUrl.

   if (glbBaseUrl)
      url = glbBaseUrl;  // always use this if specified in mediaserver-config

   size_t len;
   if (url)
   {
      len = wcslen(url);
      char* tmp = new char[len+1];
      for (unsigned int i = 0; i < len; i++)
         tmp[i] = url[i];
      tmp[len] = 0;
      mBaseUrl = UtlString(tmp);
      delete[] tmp; 
      tmp = NULL;
   }

   // If glbBaseUrl is not provided, and mBaseUrl is a full URL, reduce
   // it to its directory and append PROMPT_DIR.
   if ( !glbBaseUrl &&
        ( (mBaseUrl.index("http:") != UTL_NOT_FOUND) ||
          (mBaseUrl.index("https:") != UTL_NOT_FOUND) ) )
   {
      if ((len = mBaseUrl.last('/')) != (size_t)UTL_NOT_FOUND)
      {
         mBaseUrl = mBaseUrl(0, (len + 1));
         // :TODO: I suspect this line should be below the current 'if'.
         mBaseUrl.append(PROMPT_DIR);
      }
   }
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::parseBaseUrl: mBaseUrl(after) = '%s'",
                 mBaseUrl.data());
}

int OSBpromptTTS::getDigitUrls(Url** prompts)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getDigitUrls: mText = '%s', mBaseUrl = '%s'",
                 mText.data(), mBaseUrl.data());
   const char* text = mText.data();
   mCount = mText.length();
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getDigitUrls: mText = '%s'", mText.data());

   if (mCount <= 0) return 0;

   UtlString urlstr;
   mPrompts = new Url[mCount];
   for (int i = 0; i < mCount; i++)
   {
      urlstr = mBaseUrl + UtlString(&text[i], 1) + ".wav";
      mPrompts[i] = Url(urlstr);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getDigitUrls: mPrompts[%d] = '%s'",
                    i, mPrompts[i].toString().data());
   }

   *prompts = mPrompts;

   return mCount;
}

int OSBpromptTTS::getNumberUrls(Url** prompts)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getNumberUrls: mText = '%s', mBaseUrl='%s'",
                 mText.data(), mBaseUrl.data());

   int i;
   int length = mText.length() ;
   if (length <= 0)
   {
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_WARNING,
                    "OSBpromptTTS::getNumberUrls: empty input text. mText = '%s'",
                    mText.data());
      return 0;
   }

   UtlString* subTexts = new UtlString[length];

   // For French language
   if (mLocale.compareTo("fr-FR", UtlString::matchCase) == 0)
   {
   }
   else // US English is always as default
   {
      // Parse the input text into a sequence of sub units
      // i.e. xxx,xxx,xxx,xxx,xxx
      int num = atoi(mText.data());
      
      UtlString base_number[10];
      int beginIndex;
      int baseLength;
      int totalLength = length;
      int totalBases = (length - 1) / 3 + 1;
      for (i = totalBases; i > 0; i--)
      {
         if (i == 1)
         {
            beginIndex = 0;
            baseLength = totalLength - 3 * (totalBases - 1);
         }
         else
         {
            beginIndex = totalLength - 3 * (totalBases - i + 1);
            baseLength = 3;
         }

         base_number[i-1] = mText(beginIndex, baseLength);
      }

      // Construct the proper text for each sub unit
      mCount = 0;
      UtlString tmpNumber;
      for (i = 0; i < totalBases; i++)
      {
         num = atoi(base_number[i].data());
         if (num != 0)
         {
            totalLength = base_number[i].length();
            if (totalLength == 3)
            {
               subTexts[mCount] = base_number[i](0, 1);
               if (subTexts[mCount].compareTo("0") != 0)
               {
                  subTexts[mCount] += "00";
                  mCount++;
               }

               tmpNumber = base_number[i](1, 2);
               base_number[i] = tmpNumber;
            }

            if (base_number[i].compareTo("00") != 0)
            {
               subTexts[mCount] = base_number[i];
               mCount++;
            }

            if (i != totalBases - 1)
            {
               subTexts[mCount] = THOUSAND[totalBases-2-i];
               mCount++;
            }
         }
      }
   }

   // Construct a list of prompt URLs
   mPrompts = new Url[mCount];
   for (int i = 0; i < mCount; i++)
   {
      mPrompts[i] = Url(mBaseUrl + subTexts[i] + ".wav");
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getDigitUrls: mPrompts[%d] = '%s'",
                    i, mPrompts[i].toString().data());
   }

   *prompts = mPrompts;
   
   if (subTexts)
   {
      delete [] subTexts;
   }

   return mCount;
}

int OSBpromptTTS::getOrdinalUrls(Url** prompts)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getOrdinalUrls: mText = '%s', mBaseUrl='%s'",
                 mText.data(), mBaseUrl.data());
   mCount = 1;
   // UNUSED VARIABLE const char* text = mText.data();
   mPrompts = new Url[mCount];

   if (!mText.isNull())
   {
      UtlString url;
      getOrdinalUrlHelper( mText, url );
      mPrompts[0] = Url( url );
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getOrdinalUrls: mText = '%s', mPrompts[0] = '%s'",
                    mText.data(), mPrompts[0].toString().data());
   }

   *prompts = mPrompts;

   return mCount;
}

void OSBpromptTTS::getOrdinalUrlHelper(UtlString& text, UtlString& url)
{
   // find the length of the number to be played.
   int length = text.length();

   // Stores the last digit of the number.
   // This will determine the suffix of the ordinal -
   // (1)st, (2)nd, (3)rd or (4-9)th.
   UtlString lastDigit;
   lastDigit.append(text, length - 1, 1);

   // Get the URL
   if ((lastDigit == "1") && (text.compareTo("11") != 0))
   {
      url = mBaseUrl + text + UtlString("st.wav");
   }
   else if ((lastDigit == "2") && (text.compareTo("12") != 0))
   {
      url = mBaseUrl + text + UtlString("nd.wav");
   }
   else if ((lastDigit == "3") && (text.compareTo("13") != 0))
   {
      url = mBaseUrl + text + UtlString("rd.wav");
   }
   else
   {
      url = mBaseUrl + text + UtlString("th.wav");
   }

   return;
}

// Example: 781-938-5306
int OSBpromptTTS::getPhoneUrls(Url** prompts)
{
   const char* text = mText.data();
   mCount = mText.length();

   if (mCount <= 0) return 0;

   UtlString urlstr;
   int j = 0, last = 0;
   mPrompts = new Url[mCount];
   for (int i = 0; i < mCount; i++)
   {
      if (text[i] <= '9' && text[i] >= '0')
      {
         urlstr = mBaseUrl + UtlString(&text[i], 1) + UtlString(".wav");
         mPrompts[j++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getPhoneUrls: mPrompts[%d] = '%s'",
                       j - 1, mPrompts[j - 1].toString().data());
         last = i;
      }
   }

   urlstr = mBaseUrl + UtlString(&text[last], 1) + UtlString(".wav");
   mPrompts[j - 1] = Url(urlstr);
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getPhoneUrls: mPrompts[%d] = '%s'",
                 j - 1, mPrompts[j - 1].toString().data());
   *prompts = mPrompts;

   return mCount;
}

// Example: 10/21/2002
int OSBpromptTTS::getDateMDYUrls(Url** prompts)
{
   // UNUSED VARIABLE const char* text = mText.data();
   UtlString mon = "", day = "", year = "";
   ssize_t i, j;
   j = mText.index("/");
   if (j != UTL_NOT_FOUND)
   {
      mon = mText(0, j);
   }
   i = mText.index("/", (j + 1));
   if (i != UTL_NOT_FOUND)
   {
      day = mText((j + 1), (i - j - 1));
      year = mText((i + 1), (mText.length() - i - 1));
   }

   return getDateFrom(mon, day, year, prompts);
}

int OSBpromptTTS::getDateMDUrls(Url** prompts)
{
   // UNUSED VARIABLE const char* text = mText.data();
   UtlString mon = "", day = "", year = "";
   ssize_t j;
   j = mText.index("/");
   if (j != UTL_NOT_FOUND)
   {
      mon = mText(0, j);
      day = mText((j + 1), (mText.length() - j - 1));
   }

   return getDateFrom(mon, day, year, prompts);
}

int OSBpromptTTS::getDateDMYUrls(Url** prompts)
{
   // UNUSED VARIABLE const char* text = mText.data();
   UtlString mon = "", day = "", year = "";
   ssize_t i, j;
   j = mText.index("/");
   if (j != UTL_NOT_FOUND)
   {
      day = mText(0, j);
   }
   i = mText.index("/", (j + 1));
   if (i != UTL_NOT_FOUND)
   {
      mon = mText((j + 1), (i - j - 1));
      year = mText((i + 1), (mText.length() - i - 1));
   }

   return getDateFrom(mon, day, year, prompts);
}

int OSBpromptTTS::getDateDMUrls(Url** prompts)
{
   // UNUSED VARIABLE const char* text = mText.data();
   UtlString mon = "", day = "", year = "";
   ssize_t j;
   j = mText.index("/");
   if (j != UTL_NOT_FOUND)
   {
      day = mText(0, j);
      mon = mText((j + 1), (mText.length() - j - 1));
   }

   return getDateFrom(mon, day, year, prompts);
}

int OSBpromptTTS::getDateMYUrls(Url** prompts)
{
   // UNUSED VARIABLE const char* text = mText.data();
   UtlString mon = "", day = "", year = "";
   ssize_t j;
   j = mText.index("/");
   if (j != UTL_NOT_FOUND)
   {
      mon = mText(0, j);
      year = mText((j + 1), (mText.length() - j - 1));
   }

   return getDateFrom(mon, day, year, prompts);
}

int OSBpromptTTS::getDateYMUrls(Url** prompts)
{
   // UNUSED VARIABLE const char* text = mText.data();
   UtlString mon = "", day = "", year = "";
   ssize_t j;
   j = mText.index("/");
   if (j != UTL_NOT_FOUND)
   {
      year = mText(0, j);
      mon = mText((j + 1), (mText.length() - j - 1));
   }

   return getDateFrom(mon, day, year, prompts);
}

int OSBpromptTTS::getDateDUrls(Url** prompts)
{
   UtlString mon = "", year = "";
   return getDateFrom(mon, mText, year, prompts);
}

int OSBpromptTTS::getDayOfTheWeekUrls(Url** prompts)
{
   mCount = 1;
   mPrompts = new Url[1];
   if( mText.index("sun", 0, UtlString::ignoreCase) != UTL_NOT_FOUND )
   {
      UtlString urlstr = mBaseUrl + UtlString(DAY[0]);
      mPrompts[0] = Url( urlstr );
   }
   else if( mText.index("mon", 0, UtlString::ignoreCase) != UTL_NOT_FOUND )
   {
      UtlString urlstr = mBaseUrl + UtlString(DAY[1]);
      mPrompts[0] = Url( urlstr );
   }
   else if( mText.index("tue", 0, UtlString::ignoreCase) != UTL_NOT_FOUND )
   {
      UtlString urlstr = mBaseUrl + UtlString(DAY[2]);
      mPrompts[0] = Url( urlstr );
   }
   else if( mText.index("wed", 0, UtlString::ignoreCase) != UTL_NOT_FOUND )
   {
      UtlString urlstr = mBaseUrl + UtlString(DAY[3]);
      mPrompts[0] = Url( urlstr );
   }
   else if( mText.index("thu", 0, UtlString::ignoreCase) != UTL_NOT_FOUND )
   {
      UtlString urlstr = mBaseUrl + UtlString(DAY[4]);
      mPrompts[0] = Url( urlstr );
   }
   else if( mText.index("fri", 0, UtlString::ignoreCase) != UTL_NOT_FOUND )
   {
      UtlString urlstr = mBaseUrl + UtlString(DAY[5]);
      mPrompts[0] = Url( urlstr );
   }
   else if( mText.index("sat", 0, UtlString::ignoreCase) != UTL_NOT_FOUND )
   {
      UtlString urlstr = mBaseUrl + UtlString(DAY[6]);
      mPrompts[0] = Url( urlstr );
   }
   *prompts = mPrompts;
   return mCount;

}

int OSBpromptTTS::getDateMUrls(Url** prompts)
{
   UtlString mon = "", day = "", year = "";
   return getDateFrom(mText, day, year, prompts);
}

int OSBpromptTTS::getDateYUrls(Url** prompts)
{
   UtlString mon = "", day = "";
   return getDateFrom(mon, day, mText, prompts);
}

int OSBpromptTTS::getDateFrom(UtlString& mon, UtlString& day, UtlString& year, Url** prompts)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getDateFrom(mon = '%s', day = '%s', year = '%s')",
                 mon.data(), day.data(), year.data());
   mCount = 0;
   mPrompts = new Url[3]; // max 3 prompts for month, day, year
   UtlString urlstr;
   if (mDayFirst && !day.isNull())
   {
      getOrdinalUrlHelper(day, urlstr);
      mPrompts[mCount++] = Url(urlstr);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getDateFrom: mPrompts[%d] = '%s'",
                    mCount - 1, mPrompts[mCount - 1].toString().data());
   }
   if (!mon.isNull())
   {
      urlstr = mBaseUrl + UtlString(MONTH[atoi(mon.data()) - 1]);
      mPrompts[mCount++] = Url(urlstr);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getDateFrom: mPrompts[%d] = '%s'",
                    mCount - 1, mPrompts[mCount - 1].toString().data());
   }
   if (!mDayFirst && !day.isNull())
   {
      getOrdinalUrlHelper(day, urlstr);
      mPrompts[mCount++] = Url(urlstr);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getDateFrom: mPrompts[%d] = '%s'",
                    mCount - 1, mPrompts[mCount - 1].toString().data());
   }
   if (!year.isNull())
   {
      urlstr = mBaseUrl + year + UtlString(".wav");
      mPrompts[mCount++] = Url(urlstr);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getDateFrom: mPrompts[%d] = '%s'",
                    mCount - 1, mPrompts[mCount - 1].toString().data());
   }
   *prompts = mPrompts;

   return mCount;
}

/* Pronounce times of day.
 *
 * Various cases:
 *      hh              "hh o'clock"
 *      hh:mm           "hh mm"
 *      hh:0m           "hh oh m"
 *      hh:00           "hh o'clock"
 *      hh:mm:ss        hh:mm as above, then "and ss seconds"
 *      hh:mm:0s        hh:mm as above, then "and s seconds"
 *      hh:mm:00        hh:mm as above, then "and zero seconds"
 *      hh:mm:01        hh:mm as above, then "and 1 second"
 */
int OSBpromptTTS::getTimeFrom(UtlString& h,
                              UtlString& m,
                              UtlString& s,
                              Url** prompts)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getTimeFrom(h = '%s', m = '%s', s = '%s')",
                 h.data(), m.data(), s.data());
   // Array to hold the assembled prompts.
   // We need a maximum of 7 prompts for hour(1), minute(1), "and", second(3)
   // (The minute only requires one prompt because we have combined .wav's
   // for "oh one", "oh two", etc. in TIME[].)
   mPrompts = new Url[7];
   mCount = 0;
   UtlString urlstr;
   // These pointers can be safely saved till the end of this routine, because
   // we don't change h, m, or s.
   const char* hc = h.data();
   const char* mc = m.data();
   const char* sc = s.data();

   // Process hh.
   // If hh < 10, then need one digit to name the .wav file, else two digits.
   urlstr = mBaseUrl +
      UtlString(hc + (hc[0] == '0' ? 1 : 0)) +
      ".wav";
   mPrompts[mCount++] = Url(urlstr);
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                 mCount - 1, mPrompts[mCount - 1].toString().data());

   // Add "o'clock" if mm is absent or 00.
   if (m.isNull() || strcmp(mc, "00") == 0)
   {
      urlstr = mBaseUrl + "o_clock.wav";
      mPrompts[mCount++] = Url(urlstr);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                    mCount - 1, mPrompts[mCount - 1].toString().data());
   }

   // Process mm.
   // mm = 00 acts as if it was absent.
   if (!m.isNull() && strcmp(m.data(), "00") != 0)
   {
      if (mc[0] == '0')
      {
         // Single digit.
         // Use the combined .wav named in TIME[].
         urlstr = mBaseUrl + TIME[mc[1] - '0' - 1];
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
      }
      else
      {
         // Double digit.
         urlstr = mBaseUrl + m + ".wav";
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
      }
   }

   // Process ss.
   if (!s.isNull())
   {
      // Add "and".
      urlstr = mBaseUrl + "and.wav";
      mPrompts[mCount++] = Url(urlstr);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                    mCount - 1, mPrompts[mCount - 1].toString().data());

      // Break out all the cases.
      if (strcmp(sc, "01") == 0)
      {
         urlstr = mBaseUrl + "1.wav";
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
         urlstr = mBaseUrl + "second.wav";
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
      }
      else
      {
         // Group all the other cases together so we can share
         // appending "seconds".
         if (strcmp(sc, "00") == 0)
         {
            urlstr = mBaseUrl + "0.wav";
            mPrompts[mCount++] = Url(urlstr);
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                          "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                          mCount - 1, mPrompts[mCount - 1].toString().data());
         }
         else
         {
            // If ss < 10, then need one digit to name the .wav file,
            // else two digits.
            urlstr = mBaseUrl +
               UtlString(sc + (sc[0] == '0' ? 1 : 0)) +
               ".wav";
            mPrompts[mCount++] = Url(urlstr);
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                          "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                          mCount - 1, mPrompts[mCount - 1].toString().data());
         }
         // Append "seconds".
         urlstr = mBaseUrl + "seconds.wav";
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());

      }
   }

#if 0   // AM/PM and timezone processing is not being done now.
   UtlString am = "", tzone = "";
   unsigned int i, j = mText.index(" ");
   if (j != UTL_NOT_FOUND)
   {
      i = mText.index(" ", (j + 1));
      if (i != UTL_NOT_FOUND)
      {
         am = mText((j + 1), (i - j - 1));
         tzone = mText((i + 1), (mText.length() - i - 1));
      }
      else
      {
         am = mText((j + 1), 2);
      }
   }

   if (!am.isNull())
   {
      am.toLower();
      urlstr = mBaseUrl + am + ".wav";
      mPrompts[mCount++] = Url(urlstr);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                    mCount - 1, mPrompts[mCount - 1].toString().data());
   }
   if (!tzone.isNull())
   {
      tzone.toLower();
      urlstr = mBaseUrl + tzone + ".wav";
      mPrompts[mCount++] = Url(urlstr);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getTimeFrom: mPrompts[%d] = '%s'",
                    mCount - 1, mPrompts[mCount - 1].toString().data());
   }
#endif

   *prompts = mPrompts;
   return mCount;
}

// Example: 8:45:55 am est
int OSBpromptTTS::getTimeHMSUrls(Url** prompts)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getTimeHMSUrls: mText = '%s', mBaseUrl = '%s'",
                 mText.data(), mBaseUrl.data());
   UtlString h = "", m = "", s = "";
   ssize_t i, j;
   j = mText.index(":");
   if (j != UTL_NOT_FOUND)
   {
      h = mText(0, j);
   }
   i = mText.index(":", (j + 1));
   if (i != UTL_NOT_FOUND)
   {
      m = mText((j + 1), (i - j - 1));
      j = mText.index(" ");
      if (j != UTL_NOT_FOUND)
         s = mText((i + 1), (j - i - 1));
      else
         s = mText((i + 1), (mText.length() - i - 1));
   }

   return getTimeFrom(h, m, s, prompts);
}

int OSBpromptTTS::getTimeHMUrls(Url** prompts)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getTimeHMUrls: mText = '%s', mBaseUrl = '%s'",
                 mText.data(), mBaseUrl.data());
   UtlString h = "", m = "", s = "";
   ssize_t i, j;
   j = mText.index(":");
   if (j != UTL_NOT_FOUND)
   {
      h = mText(0, j);
      i = mText.index(" ");
      if (i != UTL_NOT_FOUND)
         m = mText((j + 1), (i - j - 1));
      else
         m = mText((j + 1), (mText.length() - j - 1));
   }

   return getTimeFrom(h, m, s, prompts);
}

int OSBpromptTTS::getTimeHUrls(Url** prompts)
{
  OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                "OSBpromptTTS::getTimeHUrls: mText = '%s', mBaseUrl = '%s'",
                mText.data(), mBaseUrl.data());
  UtlString h = "", m = "", s = "";
  ssize_t i = mText.index(" ");
  if (i != UTL_NOT_FOUND)
    h = mText(0, i);
  else
    h = mText(0, (mText.length() - i - 1));
  return getTimeFrom(h, m, s, prompts);
}

int OSBpromptTTS::getDurationHMSUrls(Url** prompts)
{
   UtlString h = "", m = "", s = "";
   ssize_t i, j;
   j = mText.index(":");
   if (j != UTL_NOT_FOUND)
   {
      h = mText(0, j);
   }
   i = mText.index(":", (j + 1));
   if (i != UTL_NOT_FOUND)
   {
      m = mText((j + 1), (i - j - 1));
      s = mText((i + 1), (mText.length() - i - 1));
   }

   return getDurationFrom(h, m, s, prompts);
}

int OSBpromptTTS::getDurationHMUrls(Url** prompts)
{
   UtlString h = "", m = "", s = "";
   ssize_t j;
   j = mText.index(":");
   if (j != UTL_NOT_FOUND)
   {
      h = mText(0, j);
      m = mText((j + 1), (mText.length() - j - 1));
   }

   return getDurationFrom(h, m, s, prompts);
}

int OSBpromptTTS::getDurationMSUrls(Url** prompts)
{
   UtlString h = "", m = "", s = "";
   ssize_t j;
   j = mText.index(":");
   if (j != UTL_NOT_FOUND)
   {
      m = mText(0, j);
      s = mText((j + 1), (mText.length() - j - 1));
   }

   return getDurationFrom(h, m, s, prompts);
}

int OSBpromptTTS::getDurationHUrls(Url** prompts)
{
   UtlString m = "", s = "";
   return getDurationFrom(mText, m, s, prompts);
}

int OSBpromptTTS::getDurationMUrls(Url** prompts)
{
   UtlString h = "", s = "";
   return getDurationFrom(h, mText, s, prompts);
}

int OSBpromptTTS::getDurationSUrls(Url** prompts)
{
   UtlString h = "", m = "";
   return getDurationFrom(h, m, mText, prompts);
}

int OSBpromptTTS::getDurationFrom(UtlString& h,
                              UtlString& m,
                              UtlString& s,
                              Url** prompts)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getDurationFrom(h = '%s', m = '%s', s = '%s'",
                 h.data(), m.data(), s.data());
   mCount = 0;
   mPrompts = new Url[7]; // max 7 prompts for hour, minute, and, second
   UtlString urlstr;
   if (!h.isNull())
   {
      urlstr = mBaseUrl + h + UtlString(".wav");
      mPrompts[mCount++] = Url(urlstr);
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getDurationFrom: mPrompts[%d] = '%s'",
                    mCount - 1, mPrompts[mCount - 1].toString().data());
      if (h.compareTo("1") != 0)
      {
         urlstr = mBaseUrl + UtlString("hour.wav");
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getDurationFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
      }
      else
      {
         urlstr = mBaseUrl + UtlString("hours.wav");
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getDurationFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
      }
   }
   if (!m.isNull() && (m.compareTo("00") != 0))
   {
      if (((const char*)m)[0] == '0')
      {
         if (((const char*)m)[1] != '0')
         {
            urlstr = mBaseUrl + UtlString(&m.data()[1], 1) + UtlString(".wav");
            mPrompts[mCount++] = Url(urlstr);
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                          "OSBpromptTTS::getDurationFrom: mPrompts[%d] = '%s'",
                          mCount - 1, mPrompts[mCount - 1].toString().data());
         }
      }
      else
      {
         urlstr = mBaseUrl + m + UtlString(".wav");
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getDurationFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
      }
      if (m.compareTo("1") != 0)
      {
         urlstr = mBaseUrl + UtlString("minute.wav");
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getDurationFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
      }
      else
      {
         urlstr = mBaseUrl + UtlString("minutes.wav");
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getDurationFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
      }
   }
   if (!s.isNull() && (s.compareTo("00") != 0))
   {
      if (mCount > 0)
      {
         urlstr = mBaseUrl + UtlString("and.wav");
         mPrompts[mCount++] = Url(urlstr);
      }
      if (((const char*)s)[0] == '0')
      {
         if (((const char*)s)[1] != '0')
         {
            urlstr = mBaseUrl + UtlString(&s.data()[1], 1) + UtlString(".wav");
            mPrompts[mCount++] = Url(urlstr);
            OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                          "OSBpromptTTS::getDurationFrom: mPrompts[%d] = '%s'",
                          mCount - 1, mPrompts[mCount - 1].toString().data());
         }
      }
      else
      {
         urlstr = mBaseUrl + s + UtlString(".wav");
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getDurationFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
      }
      if (s.compareTo("1") != 0)
      {
         urlstr = mBaseUrl + UtlString("seconds.wav");
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getDurationFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
      }
      else
      {
         urlstr = mBaseUrl + UtlString("second.wav");
         mPrompts[mCount++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                       "OSBpromptTTS::getDurationFrom: mPrompts[%d] = '%s'",
                       mCount - 1, mPrompts[mCount - 1].toString().data());
      }
   }

   *prompts = mPrompts;
   return mCount;
}

int OSBpromptTTS::getAcronymUrls(Url** prompts)
{
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getAcronymUrls");
   const char* text = mText.data();
   mCount = mText.length();
   OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                 "OSBpromptTTS::getAcronymUrls: mText = '%s', mBaseUrl = '%s'",
                 mText.data(), mBaseUrl.data());

   if (mCount <= 0) return 0;

   UtlString urlstr;
   mPrompts = new Url[mCount];
   int n = 0 ;
   for (int i = 0; i < mCount; i++)
   {
      char c = tolower(text[i]) ;
      const char *name = NULL ;
      char easy[2] = {0} ;
     
      if (isalnum(c)) // It's alphanumeric, so the file name is the character
      {
         easy[0] = c ;
         name = easy ;
      }
      else
      {
         switch(c) // The file name is not so obvious, we need to handle 
         {         // each one specially
            case '.': name = "dot"; break ;
            case '-': name = "dash"; break ;
            case ':': name = "colon"; break ;
            case '+': name = "plus"; break ;
            case '@': name = "at"; break ;
            default:  name = NULL ; // No prompt for that one.
         }
      }
      
#if 0
      OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getAcronymUrls: text = '%s', text[%d] = '%c' c='%c' name=%s",
                    text, i, text[i], c, name?name:"(NULL)");
#endif
      if (name)
      {
         urlstr = mBaseUrl + name + ".wav";
         mPrompts[n++] = Url(urlstr);
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_DEBUG,
                    "OSBpromptTTS::getAcronymUrls: mPrompts[%d] = '%s'",
                    i, urlstr.data());
      }
      else
      {
         // Skip ones we cannot speak
         OsSysLog::add(FAC_MEDIASERVER_VXI, PRI_WARNING,
                    "OSBpromptTTS::getAcronymUrls: no prompt for '%c'",c);
      }
   }

   *prompts = mPrompts;

   return n;
}


/* ============================ TESTING =================================== */

#ifdef TEST

// Set to true after the tests have been executed once
bool OSBpromptTTS::sIsTested = false;

// Test this class by running all of its assertion tests
void OSBpromptTTS::test()
{

   UtlMemCheck* pUtlMemCheck = 0;
   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   testCreators();
   testManipulators();
   testAccessors();
   testInquiry();

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the creators (and destructor) methods for the class
void OSBpromptTTS::testCreators()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the default constructor (if implemented)
   // test the copy constructor (if implemented)
   // test other constructors (if implemented)
   //    if a constructor parameter is used to set information in an ancestor
   //       class, then verify it gets set correctly (i.e., via ancestor
   //       class accessor method.
   // test the destructor
   //    if the class contains member pointer variables, verify that the
   //    pointers are getting scrubbed.

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the manipulator methods
void OSBpromptTTS::testManipulators()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // test the assignment method (if implemented)
   // test the other manipulator methods for the class

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the accessor methods for the class
void OSBpromptTTS::testAccessors()
{
   UtlMemCheck* pUtlMemCheck  = 0;

   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

// Test the inquiry methods for the class
void OSBpromptTTS::testInquiry()
{
   UtlMemCheck* pUtlMemCheck  = 0;


   pUtlMemCheck = new UtlMemCheck();         // checkpoint for memory leak check

   // body of the test goes here

   assert(pUtlMemCheck->delta() == 0);    // check for memory leak
   delete pUtlMemCheck;
}

#endif //TEST

/* ============================ FUNCTIONS ================================= */
