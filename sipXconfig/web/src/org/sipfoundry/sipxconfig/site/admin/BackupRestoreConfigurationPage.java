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

import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.Persist;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.ftp.FtpConfiguration;

public abstract class BackupRestoreConfigurationPage extends BasePage implements PageBeginRenderListener {

    public static final String PAGE = "admin/BackupRestoreConfigurationPage";

    @InjectObject(value = "spring:adminContext")
    public abstract AdminContext getAdminContext();

    @Persist(value = "session")
    public abstract String getLaunchingPage();
    public abstract void setLaunchingPage(String page);

    public abstract String getAddress();
    public abstract void setAddress(String address);

    public abstract String getPassword();
    public abstract void setPassword(String password);
    public abstract String getUser();
    public abstract void setUser(String user);
    public abstract FtpConfiguration getFtpConfiguration();
    public abstract void setFtpConfiguration(FtpConfiguration configuration);

    public void pageBeginRender(PageEvent event) {
        FtpConfiguration savedConfiguration = getAdminContext().getFtpConfiguration();
        FtpConfiguration ftpConfiguration = savedConfiguration == null ? new FtpConfiguration() : savedConfiguration;
        setFtpConfiguration(ftpConfiguration);
        setAddress(ftpConfiguration.getHost());
        setUser(ftpConfiguration.getUserId());
        setPassword(ftpConfiguration.getPassword());
    }

    public String onOk() {

        getFtpConfiguration().setHost(getAddress());
        getFtpConfiguration().setUserId(getUser());
        getFtpConfiguration().setPassword(getPassword());
        getAdminContext().storeFtpConfiguration(getFtpConfiguration());

        return getLaunchingPage();
    }

    public String onCancel() {
        return getLaunchingPage();

    }



}
