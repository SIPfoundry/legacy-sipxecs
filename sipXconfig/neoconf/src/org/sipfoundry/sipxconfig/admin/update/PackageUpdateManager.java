/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.admin.update;

import java.util.Date;
import java.util.List;
import java.util.concurrent.Future;

import org.sipfoundry.sipxconfig.common.UserException;

public interface PackageUpdateManager {

    public enum UpdaterState {
        UPDATES_NOT_CHECKED(false), // Updates were not searched yet
        NO_UPDATES_AVAILABLE(false), // System up-to-date, no updates available
        CHECKING_FOR_UPDATES(true), // Checking sipXecs repository for updated packages
        UPDATES_AVAILABLE(false), // Check complete, updated packages were found
        INSTALLING(true), // Installing downloaded packages
        UPDATE_COMPLETED(false), // Update complete; need to reboot
        ERROR(false); // Error occurred during update

        private final boolean m_showProgressBar;

        private UpdaterState(boolean showProgressBar) {
            m_showProgressBar = showProgressBar;
        }

        public boolean getShowProgressBar() {
            return m_showProgressBar;
        }
    }

    Date getLastCheckedDate();

    List<PackageUpdate> getAvailablePackages();

    String getCurrentVersion();

    String getUpdatedVersion();

    UpdaterState getState();

    Future< ? > installUpdates();

    Future<Boolean> checkForUpdates();

    boolean isYumCapable();

    UserException getUserException();
}
