/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.user;

import java.util.List;

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.RegistrationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;

public abstract class UserRegistrations extends UserBasePage {
    public static final String PAGE = "user/UserRegistrations";

    @InjectObject(value = "spring:registrationContext")
    public abstract RegistrationContext getRegistrationContext();

    public abstract void setRegistrationsProperty(List<RegistrationItem> registrations);

    public abstract List<RegistrationItem> getRegistrationsProperty();

    public void pageBeginRender(PageEvent event) {
        super.pageBeginRender(event);
        getRegistrations();
    }

    public List<RegistrationItem> getRegistrations() {
        List<RegistrationItem> registrations = getRegistrationsProperty();
        User user = getUser();
        if (registrations != null) {
            return registrations;
        }
        registrations = getRegistrationContext().getRegistrationsByUser(user);
        setRegistrationsProperty(registrations);
        return registrations;
    }
}
