/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin.time;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.time.NtpManager;

public abstract class EditTimeZonePage extends UserBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/time/EditTimeZonePage";

    public abstract User getEditedUser();

    public abstract void setEditedUser(User user);

    @InjectObject("spring:ntpManager")
    public abstract NtpManager getTimeManager();

    @InjectObject("spring:coreContext")
    public abstract CoreContext getCoreContext();

    public void pageBeginRender(PageEvent event_) {
        if (getEditedUser() == null) {
            setEditedUser(getUser());
        }

    }
}
