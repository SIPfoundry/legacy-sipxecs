/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */

package org.sipfoundry.sipxconfig.update;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.easymock.classextension.EasyMock.createMock;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import java.util.Arrays;
import java.util.concurrent.Future;

import junit.framework.JUnit4TestAdapter;

import org.junit.Test;
import org.sipfoundry.sipxconfig.update.PackageUpdateManager.UpdaterState;

public class PackageUpdateManagerImplTest {
    public static junit.framework.Test suite() {
        return new JUnit4TestAdapter(PackageUpdateManagerImplTest.class);
    }

    @Test
    public void testGetCurrentVersion() throws Exception {
        String version = "sipxecs 3.11.6-013602";

        UpdateApi updateApi = createMock(UpdateApi.class);
        updateApi.getCurrentVersion();
        expectLastCall().andReturn(version);
        replay(updateApi);

        PackageUpdateManagerImpl updater = new PackageUpdateManagerImpl();
        updater.setUpdateApi(updateApi);

        assertEquals(version, updater.getCurrentVersion());
        verify(updateApi);
    }

    @Test
    public void testCheckForUpdates() throws Exception {
        PackageUpdate update = new PackageUpdate("sipxecs", "3.11.7", "3.11.8-013967");


        UpdateApi updateApi = createMock(UpdateApi.class);
        updateApi.getAvailableUpdates();
        expectLastCall().andReturn(Arrays.asList(update));

        replay(updateApi);

        PackageUpdateManagerImpl updater = new PackageUpdateManagerImpl();
        assertEquals(UpdaterState.UPDATES_NOT_CHECKED, updater.getState());
        updater.setUpdateApi(updateApi);

        // Do the update check.
        Future<Boolean> future = updater.checkForUpdates();
        assertTrue(future.get());

        verify(updateApi);

        assertEquals(UpdaterState.UPDATES_AVAILABLE, updater.getState());
        assertEquals(1, updater.getAvailablePackages().size());
        assertEquals("sipxecs 3.11.8-013967", updater.getUpdatedVersion());
    }

    @Test
    public void testInstall() throws Exception {
        UpdateApi updateApi = createMock(UpdateApi.class);
        updateApi.installUpdates();
        replay(updateApi);

        PackageUpdateManagerImpl updater = new PackageUpdateManagerImpl();
        updater.setUpdateApi(updateApi);
        assertEquals(UpdaterState.UPDATES_NOT_CHECKED, updater.getState());
        Future< ? > future = updater.installUpdates();
        future.get();

        // this scenario appears only if an error occurred on installation
        assertEquals(UpdaterState.UPDATES_AVAILABLE, updater.getState());
        verify(updateApi);
    }
}
