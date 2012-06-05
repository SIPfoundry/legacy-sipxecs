/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.user_portal;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Parameter;
import org.sipfoundry.commons.userdb.profile.Address;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.site.UserSession;

@ComponentClass(allowBody = false, allowInformalParameters = true)
public abstract class ExtendedUserInfoComponent extends BaseComponent {

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Parameter(required = true)
    public abstract User getUser();

    @Parameter(required = true)
    public abstract UserSession getUserSession();

    public abstract UserProfile getUserProfile();

    public abstract void setUserProfile(UserProfile abe);

    public abstract ImAccount getImAccount();

    public abstract void setImAccount(ImAccount imAccount);

    public void prepareForRender(IRequestCycle cycle) {
        if (null == getUserProfile()) {
            UserProfile profile = getUser().getUserProfile();
            if (profile == null) {
                profile = new UserProfile();
            }
            if (profile.getHomeAddress() == null) {
                profile.setHomeAddress(new Address());
            }
            if (profile.getOfficeAddress() == null) {
                profile.setOfficeAddress(new Address());
            }
            setUserProfile(profile);
        }

        if (getImAccount() == null) {
            setImAccount(new ImAccount(getUser()));
        }
    }

    public void commit() {
        if (!TapestryUtils.isValid(this)) {
            return;
        }

        User user = getUser();
        user.setUserProfile(getUserProfile());

        getCoreContext().saveUser(user);
    }
}
