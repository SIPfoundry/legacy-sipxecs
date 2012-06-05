/*
 *
 *
 * Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.moh;

import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class MusicOnHoldPage extends UserBasePage {
    public static final String PAGE = "moh/MusicOnHoldPage";

    public abstract User getLoadedUser();
    public abstract void setLoadedUser(User user);

    @Override
    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);

        if (getLoadedUser() == null) {
            setLoadedUser(getUser());
        }
    }

}
