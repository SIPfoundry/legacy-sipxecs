/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.vm;

import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public abstract class MailboxPreferencesPage extends UserBasePage {
    public static final String PAGE = "vm/MailboxPreferencesPage";

    public abstract MailboxPreferences getMailboxPreferences();

    public abstract void setMailboxPreferences(MailboxPreferences preferences);

    @Persist
    public abstract boolean isAdvanced();

    @Override
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);
        if (getMailboxPreferences() == null) {
            User user = getUser();
            setMailboxPreferences(new MailboxPreferences(user));
        }
    }

    public void onApply() {
        User user = getUser();
        getMailboxPreferences().updateUser(user);
        getCoreContext().saveUser(user);
    }
}
