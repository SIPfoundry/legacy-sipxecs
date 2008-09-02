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

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.AdminContext;
import org.sipfoundry.sipxconfig.admin.ftp.FtpConfiguration;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

@ComponentClass
public abstract class BackupRestoreConfigurationPanel extends BaseComponent implements PageBeginRenderListener {

    @InjectObject(value = "spring:adminContext")
    public abstract AdminContext getAdminContext();

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
    public void onApply() {
        TapestryUtils.getValidator(this).clear();
        getFtpConfiguration().setHost(getAddress());
        getFtpConfiguration().setUserId(getUser());
        getFtpConfiguration().setPassword(getPassword());
        getAdminContext().storeFtpConfiguration(getFtpConfiguration());
    }

}
