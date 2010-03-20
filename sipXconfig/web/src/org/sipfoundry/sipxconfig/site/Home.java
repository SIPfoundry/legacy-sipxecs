/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.site.vm.ManageVoicemail;

public abstract class Home extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "Home";

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    public void pageBeginRender(PageEvent event) {
        if (!getUserSession().isAdmin()) {
            throw new PageRedirectException(ManageVoicemail.PAGE);
        }
    }
}
