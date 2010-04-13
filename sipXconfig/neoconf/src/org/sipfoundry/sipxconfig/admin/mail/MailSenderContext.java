/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.mail;

import java.io.File;

public interface MailSenderContext {
    public void sendMail(String to, String from, String subject, String body);
    public void sendMail(String to, String from, String subject, String body, File... files);
}
