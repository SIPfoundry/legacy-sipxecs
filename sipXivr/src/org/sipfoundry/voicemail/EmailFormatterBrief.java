/*
 * 
 * 
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 */

package org.sipfoundry.voicemail;


/**
 * A formatter for brief e-mail messages (such as SMS) 
 */
public class EmailFormatterBrief extends EmailFormatter {

    /**
     * The subject part of the e-mail
     * @return
     */
    @Override
    public String getSubject() {
        return fmt("SubjectBrief");
    }

    /**
     * The HTML body part of the e-mail
     * @return
     */
    @Override
    public String getHtmlBody() {
        return fmt("HtmlBodyBrief");
    }

    /**
     * The text body part of the e-mail
     * @return
     */
    @Override
    public String getTextBody() {
        return fmt("TextBodyBrief");
    }
}
