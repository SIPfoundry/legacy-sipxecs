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
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectState;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.login.LoginContext;
import org.sipfoundry.sipxconfig.site.UserSession;
import org.sipfoundry.sipxconfig.site.user.ManageUsers;

/**
 * Base page for all the user specific pages.
 *
 * User specific pages can be displayed in User Portal, in Admin Portal or in both places. The
 * important characteristics of such pages is that they display some data specific to the user.
 * When they are displayed in User portal the page will only present the data for logged in user.
 * Administrator can of course display/edit the data for any user.
 */
public abstract class UserBasePage extends PageWithCallback implements PageBeginRenderListener {

    @InjectObject(value = "spring:loginContext")
    public abstract LoginContext getLoginContext();

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @Persist
    public abstract Integer getUserId();

    public abstract void setUserId(Integer userId);

    @InjectState(value = "userSession")
    public abstract UserSession getUserSession();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    /**
     * Determine the id of the user for which page will be changing call forwarding setting.
     *
     * If current login user has admin privilidges he can change call forwarding for any user.
     * However user without admin privilidges can only edit settings for logged in user.
     *
     * @return id of the user for which page will be changing call forwarding setting
     */
    protected Integer getActiveUserId() {
        Integer userId = getUserId();
        Integer loginUserId = getUserSession().getUserId();

        if (userId == null) {
            // No userId has been set yet, so make it the logged-in user
            return loginUserId;
        }
        // if they are the same it does not matter which one we return
        if (userId.equals(loginUserId)) {
            return userId;
        }

        // If the userId is not that of the logged-in user, then make sure
        // that the logged-in user has admin privileges. If not, then
        // force the userId to be the one for the logged-in user, so non-admin
        // users can only see/modify their own settings.
        if (getLoginContext().isAdmin(loginUserId)) {
            return userId;
        }
        return loginUserId;
    }

    public User getUser() {
        Integer userId = getUserId();
        User user = getCoreContext().loadUser(userId);
        return user;
    }

    public void pageBeginRender(PageEvent event_) {
        if (getCallback() == null && getUserSession().isAdmin()) {
            setReturnPage(ManageUsers.PAGE);
        }

        Integer userId = getActiveUserId();
        setUserId(userId);
    }
}
