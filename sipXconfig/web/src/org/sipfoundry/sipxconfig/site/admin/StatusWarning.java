/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.admin;

import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.annotations.ComponentClass;
import org.apache.tapestry.annotations.InjectObject;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.update.PackageUpdateManager;
import org.sipfoundry.sipxconfig.job.JobContext;
import org.sipfoundry.sipxconfig.site.admin.commserver.ReloadNeededServicesPage;
import org.sipfoundry.sipxconfig.site.admin.commserver.RestartNeededServicesPage;
import org.sipfoundry.sipxconfig.site.admin.softwareupdates.SoftwareUpdatesPage;

@ComponentClass(allowBody = false, allowInformalParameters = false)
public abstract class StatusWarning extends BaseComponent {
    @InjectObject(value = "spring:jobContext")
    public abstract JobContext getJobContext();

    @InjectObject(value = "spring:sipxProcessContext")
    public abstract SipxProcessContext getSipxProcessContext();

    @InjectObject(value = "spring:packageUpdateManager")
    public abstract PackageUpdateManager getPackageUpdateManager();

    /**
     * Show only if there was a failure AND we are NOT on JobStatus page
     *
     * @return true if error should be shown
     */
    public boolean isShow() {
        return getJobContext().isFailure() && !JobStatusPage.PAGE.equals(getPage().getPageName());
    }

    /**
     * Show only if there is least a service mark for restart AND we are NOT on
     * RestartNeededServicesPage page
     *
     * @return true if warning should be shown
     */
    public boolean showRestartWarning() {
        return getSipxProcessContext().needsRestart()
                && !RestartNeededServicesPage.PAGE.equals(getPage().getPageName());
    }

    /**
     * Show only if there is package(s) need to be updated AND we are NOT on
     * RestartNeededServicesPage page
     *
     * @return true if warning should be shown
     */
    public boolean showPackageUpdateWarning() {
        return getPackageUpdateManager().getState() == PackageUpdateManager.UpdaterState.UPDATES_AVAILABLE
                && !SoftwareUpdatesPage.PAGE.equals(getPage().getPageName());

    }

    /**
     * Show only if there is least a service mark for reload AND we are NOT on
     * ReloadNeededServicesPage page
     *
     * @return true if warning should be shown
     */
    public boolean showReloadWarning() {
        return getSipxProcessContext().needsReload()
                && !ReloadNeededServicesPage.PAGE.equals(getPage().getPageName());
    }
}
