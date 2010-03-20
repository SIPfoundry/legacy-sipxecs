/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.html.BasePage;

public abstract class SipxBasePage extends BasePage {
    public String getBorderTitle() {
        String borderTitle = getPage().getMessages().getMessage("title");
        if (borderTitle.equals("[TITLE]")) {
            // display nothing
            return "&nbsp;";
        }
        return borderTitle;
    }
}
