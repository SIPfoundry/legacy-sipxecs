//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////


// MailMessage class definition for Mailer

//Example:
/*

        MailMessage message("XXX",
                        "XXX@pingtel.com",
                        "XXX.pingtel.com");
        message.Body("this is a test message");
        message.Subject("Hello World!");
        message.To("XXX","XXXr@pingtel.com");
        message.Send();


*/



// NOTE: String concatenation is expensive; this code is written for
//       clarity, not speed.

#include "net/MailMessage.h"
#include "os/OsConnectionSocket.h"

UtlString CRLF = "\r\n";
UtlString CRLFCRLF = "\r\n\r\n";

void MailMessage::Body(const UtlString &rText)
{
    UtlString text = rText.data();
    m_ContentType = "text/plain; charset=UTF-8";
    m_Body = text;
}

void MailMessage::Body(const UtlString &rText, const UtlString &rHtml)
{
    UtlString text = rText.data();
    UtlString html = rHtml.data();

    UtlString boundary = "---=_Next_Part_of_Text_HTML_Alternatives_13579820350782";

    m_ContentType = "multipart/alternative; boundary=\"" + boundary + "\"";

    m_Body  = CRLF + "--" + boundary + CRLF;
    m_Body += "Content-Type: text/plain; charset=UTF-8" + CRLF;
    m_Body += "Content-Transfer-Encoding: 8bit" + CRLF;
    m_Body += CRLF;
    m_Body += text;
    m_Body += CRLF + "--" + boundary + CRLF;
    m_Body += "Content-Type: text/html; charset=UTF-8" + CRLF;
    m_Body += "Content-Transfer-Encoding: 8bit" + CRLF;
    m_Body += CRLF;
    m_Body += html;
    m_Body += CRLF + "--" + boundary + "--";
}

bool MailMessage::Attach(const UtlString &rFilename)
{
    UtlString filename = rFilename.data();

    MailAttachment att;
    if (att.Load(filename.data()))
    {
        m_vecAttachment.push_back(att);
        return true;
    }
    else return false;
}

bool MailMessage::Attach( const unsigned char *data, const int& rDatalength, const UtlString &rFilename )
{
    MailAttachment att;
    if ( att.Load( data, rDatalength, rFilename ) )
    {
        m_vecAttachment.push_back(att);
        return true;
    }
    else return false;
}

#define MAILBUFLEN 4096
UtlString MailMessage::Send()
{
    char receiveBuf[MAILBUFLEN];
    UtlString str;
    UtlString errorMsg;

    // Connect to the SMTP server
    OsConnectionSocket s(25,m_Server.data());

    if (!s.isConnected())
        return "Could not connect to server";

    // Receive the banner
    s.read(receiveBuf,MAILBUFLEN);

    // Send the HELO command
    str = "HELO localhost";
    str += CRLF.data();
    s.write(str.data(),str.length());

    // Receive a 250 response
    s.read(receiveBuf,MAILBUFLEN);
    if (memcmp(receiveBuf,"250",3) != 0)
    {
        errorMsg = "Unacceptable response to HELO: ";
        errorMsg += receiveBuf;
        return errorMsg.data();
    }

    // Send the MAIL FROM command
    str = "MAIL FROM:";
    str += m_From.Address.data();
    str += CRLF.data();
    s.write(str.data(),str.length());

    // Receive a 250 response
    s.read(receiveBuf,MAILBUFLEN);
    if (memcmp(receiveBuf,"250",3) != 0)
    {
        errorMsg = "Unacceptable response to MAIL FROM: ";
        errorMsg += receiveBuf;
        return errorMsg.data();
    }

    // Send an RCPT TO for all recipients (To, Cc, and Bcc)
    unsigned int i;
    if (m_vecTo.size() + m_vecCc.size() + m_vecBcc.size() == 0)
    {
        errorMsg = "No TO/CC/BCC list given";
        return errorMsg.data();
    }
    for (i = 0; i < m_vecTo.size(); i++)
    {
        // Send an RCPT TO command
        str = "RCPT TO:";
        str += m_vecTo[i].Address.data();
        str += CRLF.data();
        s.write(str.data(),str.length());

        // Receive a 250 response
        s.read(receiveBuf,MAILBUFLEN);
        if (memcmp(receiveBuf,"250",3) != 0)
        {
            errorMsg = "Unacceptable response to RCPT TO: ";
            errorMsg += receiveBuf;
            return errorMsg.data();
        }
    }
    for (i = 0; i < m_vecCc.size(); i++)
    {
        // Send an RCPT TO command
        str = "RCPT TO:";
        str += m_vecCc[i].Address.data();
        str += CRLF.data();
        s.write(str.data(),str.length());

        // Receive a 250 response
        s.read(receiveBuf,MAILBUFLEN);
        if (memcmp(receiveBuf,"250",3) != 0)
        {
            errorMsg = "Unacceptable response to RCPT TO: ";
            errorMsg += receiveBuf;
            return errorMsg.data();
        }
    }
    for (i = 0; i < m_vecBcc.size(); i++)
    {
        // Send an RCPT TO command
        str = "RCPT TO:";
        str += m_vecBcc[i].Address.data();
        str += CRLF.data();
        s.write(str.data(),str.length());

        // Receive a 250 response
        s.read(receiveBuf,MAILBUFLEN);
        if (memcmp(receiveBuf,"250",3) != 0)
        {
            errorMsg = "Unacceptable response to RCPT TO: ";
            errorMsg += receiveBuf;
            return errorMsg.data();
        }
    }

    // Send the DATA command
    str = "DATA";
    str += CRLF.data();
    s.write(str.data(),str.length());

    // Receive a 354 response
    s.read(receiveBuf,MAILBUFLEN);
    if (memcmp(receiveBuf,"354",3) != 0)
    {
        errorMsg = "Unacceptable response to DATA: ";
        errorMsg += receiveBuf;
        return errorMsg.data();
    }

    // Format the data
    UtlString data = FormatForSending();

    // Send the message, terminated with \r\n.\r\n
    str = data.data();
    str += CRLF.data();
    s.write(str.data(),str.length());

    // Receive a 250 response
    s.read(receiveBuf,MAILBUFLEN);
    if (memcmp(receiveBuf,"250",3) != 0)
    {
        errorMsg = "Unacceptable response to body: ";
        errorMsg += receiveBuf;
        return errorMsg.data();
    }

    return "";
}

UtlString MailMessage::FormatForSending()
{
    unsigned int i;
    int lineLength = 4;

    // From: "Name" <Address>
    UtlString data = "From: " + m_From.toString() + CRLF;

    // To: "Name" <Address>, "Name" <Address>
    data += "To: ";
    for (i = 0; i < m_vecTo.size(); i++)
    {
        if (lineLength + m_vecTo[i].toString().length() > 998)
        {
            data += "\r\n ";
            lineLength = 1;
        }

        data += m_vecTo[i].toString();
        lineLength += m_vecTo[i].toString().length();

        if (i < m_vecTo.size()-1)
        {
            data += ", ";
            lineLength += 2;
        }
    }
    data += CRLF;

    // Cc: "Name" <Address>, "Name" <Address>
    data += "Cc: ";
    for (i = 0; i < m_vecCc.size(); i++)
    {
        if (lineLength + m_vecCc[i].toString().length() > 998)
        {
            data += "\r\n ";
            lineLength = 1;
        }

        data += m_vecCc[i].toString();
        lineLength += m_vecCc[i].toString().length();

        if (i < m_vecCc.size()-1)
        {
            data += ", ";
            lineLength += 2;
        }
    }
    data += CRLF;

    // Subject: Subject text
    data += "Subject: " + m_Subject + CRLF;

    // Date: Mon, 25 Sep 2001 11:11:11 -0600\r\n
    UtlString dateString;
    OsDateTime now;
    OsDateTime::getCurTime(now);
    now.getHttpTimeString(dateString);
/*
    SYSTEMTIME now; GetLocalTime(&now);
    TIME_ZONE_INFORMATION tzinfo; GetTimeZoneInformation(&tzinfo);
    char *day[] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
    char *month[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul",
                      "Aug","Sep","Oct","Nov","Dec" };
    char buf[128];
    wsprintf(buf, "Date: %s, %u %s %u %02u:%02u:%02u %c%02u%02u",
        day[now.wDayOfWeek],
        now.wDay,
        month[now.wMonth],
        now.wYear,
        now.wHour,
        now.wMinute,
        now.wSecond,
        tzinfo.Bias > 0 ? '-' : '+',
        abs(tzinfo.Bias/60),
        abs(tzinfo.Bias%60));
*/
    char buf[128];
    sprintf(buf, "Date: %s",dateString.data());

    data += buf + CRLF;

    // MIME version header
    data += "Mime-Version: 1.0" + CRLF;

    // If attachments...
    if (m_vecAttachment.size() > 0)
    {
        UtlString boundary = "---=_Next_Part_of_Message_987456321147852369";

        // MIME content type header and separator line
        data += "Content-Type: multipart/mixed; boundary=\"" + boundary + "\""
                + CRLFCRLF;

        // Message body
        data += "--" + boundary + CRLF;
        data += "Content-Type: " + m_ContentType + CRLFCRLF;
        data += m_Body + CRLF;

        // Attachments
        for (int i = 0; i < ((int)(m_vecAttachment.size())); i++)
        {
            data += "--" + boundary + CRLF;
            data += "Content-Type: " + m_vecAttachment[i].MIMEtype()
                    + "; name=\"" + m_vecAttachment[i].Filename() + "\"" + CRLF;
            data += "Content-Transfer-Encoding: base64" + CRLF;
            data += "Content-Disposition: attachment; filename=\""
                    + m_vecAttachment[i].Filename() + "\"" + CRLFCRLF;
            data += m_vecAttachment[i].Base64Data() + CRLF;
        }
        data += "--" + boundary + "--";
    }
    // No attachments
    else
    {
        // MIME content type header
        data += "Content-Type: " + m_ContentType + CRLF;

        // Separator line, body
        data += CRLF + m_Body;
    }
    data += CRLF + ".";
    data += CRLF;
    return data;
}
