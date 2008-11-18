/*
 * 
 * 
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */

package org.sipfoundry.sipxconfig.site.admin.softwareupdates;

import java.text.DateFormat;
import java.text.Format;
import java.util.List;

import org.apache.tapestry.IAsset;
import org.apache.tapestry.IPage;
import org.apache.tapestry.annotations.Asset;
import org.apache.tapestry.annotations.InitialValue;
import org.apache.tapestry.annotations.InjectObject;
import org.apache.tapestry.annotations.InjectPage;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.admin.PackageUpdate;
import org.sipfoundry.sipxconfig.admin.PackageUpdateManager;
import org.sipfoundry.sipxconfig.admin.PackageUpdateManager.UpdaterState;
import org.sipfoundry.sipxconfig.admin.SynchronousPackageUpdate;
import org.sipfoundry.sipxconfig.site.admin.WaitingPage;

public abstract class SoftwareUpdatesPage extends BasePage {

    @Asset("/images/progressBar.gif")
    public abstract IAsset getProgressBar();

    @Asset("/images/package.png")
    public abstract IAsset getPackageIcon();
    
    @Asset("/images/updatePackage.png")
    public abstract IAsset getUpdatePackageIcon();
    
    @Asset("/images/new-software.png")
    public abstract IAsset getNewSoftwareIcon();
    
    @Asset("context:/WEB-INF/admin/softwareupdates/SoftwareUpdatesPage.script")
    public abstract IAsset getScript();    
    
    @InjectObject("spring:packageUpdateManager")
    public abstract PackageUpdateManager getPackageUpdateManager();
    
    @InjectPage(value = WaitingPage.PAGE)
    public abstract WaitingPage getWaitingPage();    
    
    @InitialValue(value = "ognl:{}")
    public abstract List<PackageUpdate> getUpdatedPackages();
    public abstract void setUpdatedPackages(List<PackageUpdate> updatedPackages);
    
    public abstract PackageUpdate getPackage();

    public Format getDateFormat() {
        return DateFormat.getDateTimeInstance(DateFormat.DEFAULT, DateFormat.SHORT, getPage().getLocale());
    }
    
    public boolean getReadyToInstall() {
        return getPackageUpdateManager().getState().equals(UpdaterState.UPDATES_AVAILABLE);
    }
    
    public String getCurrentVersion() {
        return getPackageUpdateManager().getCurrentVersion();
    }
    
    public void checkForUpdates() {
        getPackageUpdateManager().checkForUpdates();
    }
    
    public IPage installUpdates() {
//        getPackageUpdateManager().installUpdates();
        WaitingPage waitingPage = getWaitingPage();
        waitingPage.setWaitingListener(new SynchronousPackageUpdate(getPackageUpdateManager().getUpdateBinaryPath()));
        return waitingPage;
    }
}
