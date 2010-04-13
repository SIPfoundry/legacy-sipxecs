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

import org.apache.tapestry.IAsset;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryContext;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.site.LoginPage;

public abstract class FirstUser extends SipxBasePage implements PageBeginRenderListener {
    public static final String PAGE = "user/FirstUser";

    @InjectObject(value = "spring:coreContext")
    public abstract CoreContext getCoreContext();

    @InjectObject(value = "spring:tapestry")
    public abstract TapestryContext getTapestry();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract String getPin();

    public abstract void setPin(String pin);

    @Persist
    public abstract boolean isLicenseAccepted();

    public abstract void setLicense(String license);

    public abstract void setRenderLicense(boolean renderLicense);

    public abstract boolean getRenderLicense();

    public void pageBeginRender(PageEvent event) {
        // This page runs only when there are no users, and the first user
        // needs to be created. If a user exists, then bail out to the login page.
        // After we create the user, we'll land here and go to login.
        if (getCoreContext().getUsersCount() != 0) {
            LoginPage loginPage = (LoginPage) event.getRequestCycle().getPage(LoginPage.PAGE);
            throw new PageRedirectException(loginPage);
        }
        // Render licence only if it's required but not yet accepted.
        if (getTapestry().isLicenseRequired() && !isLicenseAccepted()) {
            setRenderLicense(true);
            setLicense(getTapestry().getLicense());
        } else {
            setRenderLicense(false);
        }
    }

    public void commit() {
        if (TapestryUtils.isValid(this)) {
            // Create the superadmin user
            getCoreContext().createAdminGroupAndInitialUser(getPin());
        }
    }

    public IAsset[] getStylesheets() {
        return getTapestry().getStylesheets(this);
    }
}
