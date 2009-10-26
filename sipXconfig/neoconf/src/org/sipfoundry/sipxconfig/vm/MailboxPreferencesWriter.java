/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.vm;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang.StringUtils;
import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.vm.MailboxManagerImpl.YesNo;

public class MailboxPreferencesWriter extends XmlWriterImpl<MailboxPreferences> {
    public MailboxPreferencesWriter() {
        setTemplate("mailbox/mailboxprefs.vm");
    }

    @Override
    protected void addContext(VelocityContext context, MailboxPreferences prefs) {
        boolean altEmail = StringUtils.isNotBlank(prefs.getAlternateEmailAddress());
        context.put("hasAlternateEmailAddress", altEmail);
        context.put("preferences", prefs);
        context.put("yesNo", new YesNo());
        context.put("ifEmailServer", prefs.isImapServerConfigured());
        // FIXME: this code is using default platform encoding - not safe
        String pwd = StringUtils.defaultString(prefs.getImapPassword());
        String encodedPwd = new String(Base64.encodeBase64(pwd.getBytes()));
        context.put("pwd", encodedPwd);
    }
}
