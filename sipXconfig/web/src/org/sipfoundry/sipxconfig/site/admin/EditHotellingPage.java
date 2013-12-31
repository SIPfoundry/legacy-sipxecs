/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class EditHotellingPage extends UserBasePage implements PageBeginRenderListener {
    public static final String PAGE = "admin/EditHotellingPage";

    public abstract User getEditedUser();

    public abstract void setEditedUser(User user);

    public abstract Setting getHotellingSetting();

    public abstract void setHotellingSetting(Setting value);

    public void pageBeginRender(PageEvent event_) {
        if (getEditedUser() == null) {
            setEditedUser(getUser());
        }
        setHotellingSetting(getEditedUser().getSettings().getSetting("hotelling"));
    }

    public void onApply() {
        getCoreContext().saveUser(getEditedUser());
    }

}
