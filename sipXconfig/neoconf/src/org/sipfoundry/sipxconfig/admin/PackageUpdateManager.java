/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin;

import java.util.Date;
import java.util.List;

/**
 * Handles package upgrade tasks, and makes use of the sipx-package.py script which talks to the Yum utility.
 */
public interface PackageUpdateManager {
    
    public enum UpdaterState {
        NO_UPDATES_AVAILABLE(false), // System up-to-date, no updates available
        CHECKING_FOR_UPDATES(true),  // Checking sipXecs repository for updated packages
        UPDATES_AVAILABLE(false),    // Check complete, updated packages were found
        DOWNLOADING(true),           // Downloading package files
        INSTALLING(true),            // Installing downloaded packages
        UPDATE_COMPLETED(false),     // Update complete; need to reboot
        ERROR(false);                // Error occurred during update
        
        private boolean m_showProgressBar;
        
        private UpdaterState(boolean showProgressBar) {
            m_showProgressBar = showProgressBar;
        }
        
        public boolean getShowProgressBar() {
            return m_showProgressBar;
        }
    }    
    
    public Date getLastCheckedDate();
    public List<PackageUpdate> getAvailablePackages();
    public String getCurrentVersion();
    public String getUpdatedVersion();
    public UpdaterState getState();
//    public void installUpdates();
    public void checkForUpdates();
    public String getUpdateBinaryPath();
}
