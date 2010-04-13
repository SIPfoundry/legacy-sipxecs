//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

//Example:
/*

        MailMessage message("XXX",
                        "XXX@pingtel.com",
                        "XXX.pingtel.com");
        message.Body("this is a test message");
        message.Subject("Hello World!");
        message.To("XXX","XXX@pingtel.com");
        message.Send();


*/


// MailMessage class declaration for Mailer

#ifndef __MAILMESSAGE_H__
#define __MAILMESSAGE_H__

#include "os/OsDefs.h"
#include "MailAttachment.h"

#include <vector>
using namespace std;

class MailMessage
{
public:
    MailMessage(const UtlString &rFromName,
                         const UtlString &rFromAddress,
                         const UtlString &rSmtpServer)
    {
        UtlString fromName = rFromName.data();
        m_From.Name = fromName.data();
        UtlString fromAddress = rFromAddress.data();
        m_From.Address = fromAddress;
        UtlString smtpServer = rSmtpServer.data();
        m_Server = smtpServer;
    }

    void To(const UtlString &rName, const UtlString &rAddress)
    {
        UtlString name = rName.data();
        UtlString address = rAddress.data();
        m_vecTo.push_back(MailAddress(name,address));
    }

    void Cc(const UtlString &rName, const UtlString &rAddress)
    {
        UtlString name = rName.data();
        UtlString address = rAddress.data();
        m_vecCc.push_back(MailAddress(name,address));
    }

    void Bcc(const UtlString &rName, const UtlString &rAddress)
    {
        UtlString name = rName.data();
        UtlString address = rAddress.data();
        m_vecBcc.push_back(MailAddress(name,address));
    }

    void Subject(const UtlString &rSubject)
    {
        UtlString subject = rSubject.data();
        m_Subject = subject;
    }

    void Body(const UtlString &rText);

    void Body(const UtlString &rText, const UtlString &rHtml);

    bool Attach(const UtlString &rFilename);

    bool Attach(const unsigned char *data, const int& rDatalength, const UtlString &rFilename );

    // Send the message to the SMTP server specified in the constructor.
    // Return either "" or an error message.
    UtlString Send();

private:
    UtlString FormatForSending();

    struct MailAddress
    {
        MailAddress() {;}
        MailAddress(const UtlString &name, const UtlString &address)
            { Name=name; Address=address; }
        UtlString toString() const
            {
                UtlString str = "\"";
                str += Name;
                str += "\" <";
                str += Address;
                str += ">";
                return str;
            }
        UtlString Name;
        UtlString Address;
    };

    MailAddress m_From;
    vector<MailAddress> m_vecTo;
    vector<MailAddress> m_vecCc;
    vector<MailAddress> m_vecBcc;
    UtlString m_Subject;
    UtlString m_ContentType;
    UtlString m_Body;
    vector<MailAttachment> m_vecAttachment;
    UtlString m_Server;
};

#endif
