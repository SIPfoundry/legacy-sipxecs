// Class to write from TinyXML structures to UtlStrings.

#ifndef _TiXmlUtlStringWriter_h_
#define _TiXmlUtlStringWriter_h_

#include "utl/UtlString.h"
#include "xmlparser/tinystr.h"

class TiXmlUtlStringWriter : public TiXmlOutStream
{
  public:

   virtual ~TiXmlUtlStringWriter()
      {
      }

   /** Create a TiXmlUtlStringWriter.  Any output sent to it via '<<'
    *  will be appended to *string.
    *  In particular,
    *        TiXmlDocument doc;
    *        UtlString s;
    *        TiXmlUtlStringWriter w(&s);
    *
    *        w << doc
    *  will append the text representation of 'doc' to 's'.
    */
   TiXmlUtlStringWriter(UtlString* string) :
      mString(string)
      {
      }

   //! TiXmlOutStream << operator. Maps to UtlString::append
   virtual TiXmlOutStream & operator << (const char * in)
      {
         mString->append(in);
         return (* this);
      }

   //! TiXmlOutStream << operator. Maps to UtlString::append
   virtual TiXmlOutStream & operator << (const TiXmlString & in)
      {
         mString->append(in . c_str ());
         return (* this);
      }

  private:

   UtlString* mString;
};

#endif    // _TiXmlUtlStringWriter_h_
