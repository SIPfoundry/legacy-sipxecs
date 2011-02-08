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

import java.util.ArrayList;
import java.util.List;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.VersionInfo;
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

    @InjectObject(value = "spring:versionInfo")
    public abstract VersionInfo getVersionInfo();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public abstract String getPin();

    public abstract void setPin(String pin);

    @Persist
    public abstract boolean isLicenseAccepted();

    public abstract int getPos();

    public abstract void setRenderLicense(boolean renderLicense);

    public abstract boolean getRenderLicense();

    @Asset("context:/WEB-INF/user/FirstUser.script")
    public abstract IAsset getFirstUserScript();

    public  String getLicense(int i) {
        return getTapestry().getLicense(i);
    }

    public String getLicenseTitle(int i) {
        String prefix = "";
        if (i > 0) {
            prefix = i + "_";
        }
        return getMessages().getMessage(prefix + "license.title");
    }

    public List<Integer> getLicensesNumber() {
        List<Integer> list = new ArrayList<Integer>();
        for (int i = 0; i < getTapestry().getLicensesNumber(); i++) {
            list.add(new Integer(i));
        }
        return list;
    }

    public String getDetails() {
        return getMessages().format("product.details", getVersionInfo().getVersionDetails());
    }

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
