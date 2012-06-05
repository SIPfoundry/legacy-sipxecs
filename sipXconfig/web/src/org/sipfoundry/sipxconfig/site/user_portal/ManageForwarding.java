/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;

public abstract class ManageForwarding extends UserBasePage {

    @Bean
    public abstract SipxValidationDelegate getValidator();

    @Persist
    @InitialValue("literal:forwarding")
    public abstract String getSelectedTab();
    public abstract void setSelectedTab(String selectedTabName);

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
