/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.site.admin.softwareupdates;

import java.text.DateFormat;
import java.text.Format;
import java.util.List;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.Bean;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.event.PageEvent;
import org.sipfoundry.sipxconfig.admin.update.PackageUpdate;
import org.sipfoundry.sipxconfig.admin.update.PackageUpdateManager;
import org.sipfoundry.sipxconfig.admin.update.UpdateApi;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.components.SipxBasePage;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.site.admin.WaitingPage;

import static org.sipfoundry.sipxconfig.admin.update.PackageUpdateManager.UpdaterState.INSTALLING;
import static org.sipfoundry.sipxconfig.admin.update.PackageUpdateManager.UpdaterState.UPDATES_AVAILABLE;

public abstract class SoftwareUpdatesPage extends SipxBasePage implements PageBeginRenderListener {

    public static final Object PAGE = "admin/softwareupdates/SoftwareUpdatesPage";

    @Asset("/images/progressBar.gif")
    public abstract IAsset getProgressBar();

    @Asset("/images/installationProgressBar.gif")
    public abstract IAsset getInstallationProgressBar();

    @Asset("/images/package.png")
    public abstract IAsset getPackageIcon();

    @Asset("/images/updatePackage.png")
    public abstract IAsset getUpdatePackageIcon();

    @Asset("/images/new-software.png")
    public abstract IAsset getNewSoftwareIcon();

    @Asset("context:/WEB-INF/admin/softwareupdates/SoftwareUpdatesPage.script")
    public abstract IAsset getScript();

    @Asset("context:/WEB-INF/admin/softwareupdates/DisplayUpdateLog.script")
    public abstract IAsset getDisplayLogScript();

    @InjectObject("spring:packageUpdateManager")
    public abstract PackageUpdateManager getPackageUpdateManager();

    @InjectPage(value = WaitingPage.PAGE)
    public abstract WaitingPage getWaitingPage();

    @InitialValue(value = "ognl:{}")
    public abstract List<PackageUpdate> getUpdatedPackages();

    public abstract void setUpdatedPackages(List<PackageUpdate> updatedPackages);

    public abstract PackageUpdate getPackage();

    public abstract void setCurrentVersion(String version);

    public abstract String getCurrentVersion();

    @Bean
    public abstract SipxValidationDelegate getValidator();

    public Format getDateFormat() {
        return DateFormat.getDateTimeInstance(DateFormat.DEFAULT, DateFormat.SHORT, getPage().getLocale());
    }

    public boolean getReadyToInstall() {
        return getPackageUpdateManager().getState().equals(UPDATES_AVAILABLE);
    }

    public boolean getCheckUpdates() {
        return getCurrentVersion() != null && !getCurrentVersion().equals(UpdateApi.VERSION_NOT_DETERMINED);
    }

    public void pageBeginRender(PageEvent event) {
        if (getCurrentVersion() == null) {
            setCurrentVersion(getPackageUpdateManager().getCurrentVersion());
        }
        UserException passedException = getPackageUpdateManager().getUserException();
        if (passedException != null) {
            getValidator().record(passedException, getMessages());
        }
    }

    public void checkForUpdates() {
        getPackageUpdateManager().checkForUpdates();
    }

    public void installUpdates() {
        getPackageUpdateManager().installUpdates();
    }

    public boolean getIsInstalling() {
        return getPackageUpdateManager().getState().equals(INSTALLING);
    }

    public String getFormattedCurrentVersion() {
        String currentVersion = getCurrentVersion();
        if (currentVersion.equals(UpdateApi.VERSION_NOT_DETERMINED)) {
            return getMessages().getMessage("label.version.undetermined");
        }
        return getMessages().format("label.version", getCurrentVersion());
    }
}
