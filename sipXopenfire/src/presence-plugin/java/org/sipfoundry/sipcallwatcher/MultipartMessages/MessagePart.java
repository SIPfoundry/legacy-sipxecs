package org.sipfoundry.sipcallwatcher.MultipartMessages;

import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.lang.Exception;

/**
 * Generic MessagePart parser. Specialized Message Part parsers
 * are expected to be derived from this class.
 *
 */
public class MessagePart 
{
    private String contentType;
    private String contentId;
    private String contentEncoding;
    private String content;
    
    final private static Pattern contentTypeRegex;
    final private static Pattern contentIdRegex;
    final private static Pattern contentEncodingRegex;
    final private static Pattern contentXmlRegex;
    static
    {
        contentTypeRegex     = Pattern.compile(".*CONTENT-TYPE:\\s*(\\p{Print}+)\\s*.*", Pattern.DOTALL | Pattern.MULTILINE);
        contentIdRegex       = Pattern.compile(".*CONTENT-ID:\\s*<(\\p{Print}+)>\\s*.*", Pattern.DOTALL | Pattern.MULTILINE);
        contentEncodingRegex = Pattern.compile(".*CONTENT-TRANSFER-ENCODING:\\s*(\\p{Print}+)\\s*.*", Pattern.DOTALL | Pattern.MULTILINE);
        contentXmlRegex      = Pattern.compile(".*(<\\?xml.+>).*", Pattern.DOTALL | Pattern.MULTILINE);
    }
    
    public MessagePart( String messagePart ) throws Exception
    {
        Matcher m1 = contentTypeRegex.matcher( messagePart );
        Matcher m2 = contentIdRegex.matcher( messagePart );
        Matcher m3 = contentEncodingRegex.matcher( messagePart );
        Matcher m4 = contentXmlRegex.matcher( messagePart );
        
        if( m1.matches() && m2.matches() && m3.matches() && m4.matches() )
        {
            contentType     = m1.group(1);
            contentId        = m2.group(1);
            contentEncoding = m3.group(1);
            content            = m4.group(1);
        }
        else
        {
            throw new Exception( "MessagePart have unexpected format: " + messagePart );
        }
    }

    public String getContentType() {
        return contentType;
    }

    public void setContentType(String contentType) {
        this.contentType = contentType;
    }

    public String getContentId() {
        return contentId;
    }

    public void setContentId(String contentId) {
        this.contentId = contentId;
    }

    public String getContentEncoding() {
        return contentEncoding;
    }

    public void setContentEncoding(String contentEncoding) {
        this.contentEncoding = contentEncoding;
    }

    public String getContent() {
        return content;
    }

    public void setContent(String content) {
        this.content = content;
    }
    
}
