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

import java.util.Collection;
import java.util.List;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.commserver.RegistrationContext;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.site.user_portal.UserBasePage;
import org.springframework.dao.support.DataAccessUtils;

public abstract class UserRegistrations extends UserBasePage {
    public static final String PAGE = "user/UserRegistrations";

    private static final String COUNTERPATH_MODEL = "counterpathCMCEnterprise";

    @InjectObject(value = "spring:registrationContext")
    public abstract RegistrationContext getRegistrationContext();

    @InjectObject("spring:locationsManager")
    public abstract LocationsManager getLocationsManager();

    @InjectObject(value = "spring:phoneContext")
    public abstract PhoneContext getPhoneContext();

    public abstract void setRegistrationsProperty(List<RegistrationItem> registrations);

    public abstract List<RegistrationItem> getRegistrationsProperty();

    @Override
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

    public boolean isCounterpath() {
        return null != getCounterpathPhone();
    }

    public String getCounterpathHelp() {
        Phone phone = getCounterpathPhone();
        if (phone == null) {
            return StringUtils.EMPTY;
        }
        String host = getLocationsManager().getPrimaryLocation().getFqdn();
        return getMessages().format("helpText.counterpath", host, phone.getModelLabel());
    }

    private Phone getCounterpathPhone() {
        Collection<Phone> phones = getPhoneContext().getPhonesByUserIdAndPhoneModel(getUser().getId(),
                COUNTERPATH_MODEL);
        return (Phone) DataAccessUtils.singleResult(phones);
    }
}
