//
// Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//
// $$
////////////////////////////////////////////////////////////////////////
//////

// MailAttachment class declaration for Mailer

#ifndef __MAILATTACHMENT_H__
#define __MAILATTACHMENT_H__

#include "os/OsDefs.h"
#include "os/OsFS.h"

class MailAttachment
{
public:
    MailAttachment() { m_Base64 = NULL; }
    ~MailAttachment();
    MailAttachment(const MailAttachment &original);
    bool Load(const UtlString &filename);
    bool Load(const unsigned char *data, const size_t& rDatalength, const UtlString &rFilename );
    UtlString Base64Data() const { return UtlString(m_Base64); }
    UtlString Filename() const { return m_Filename; }
    UtlString MIMEtype() const { return m_MIMEtype; }

private:
    void Base64Encode(const unsigned char *buffer, unsigned long buflen);
    char *m_Base64;

    UtlString m_Filename;
    UtlString m_MIMEtype;
};

#endif
