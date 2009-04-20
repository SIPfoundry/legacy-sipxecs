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

import java.io.ByteArrayInputStream;
import java.io.IOException;
import junit.framework.JUnit4TestAdapter;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.admin.PackageUpdateManager.UpdaterState;
import org.sipfoundry.sipxconfig.common.AlarmContext;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class PackageUpdateManagerImplTest {

    private static final String ALARM_TEXT = "# Package Name|Installed Version|Updated Version\n"
            + "sipxcalllib|3.11.8-013951|3.11.8-013967\n" + "sipxconfig-tftp|3.11.6-013602|3.11.8-013967\n"
            + "sipxpage|3.11.6-013602|3.11.8-013967\n" + "sipxproxy-cdr|3.11.6-013602|3.11.8-013967\n"
            + "sipxmediaadapterlib|3.11.6-013602|3.11.8-013967\n" + "sipxbridge|3.11.8-013951|3.11.8-013967\n"
            + "sipxecs|3.11.6-013602|3.11.8-013967\n" + "sipxconfig-agent|3.11.6-013602|3.11.8-013967\n"
            + "sipxecs-doc|3.11.8-013951|3.11.8-013967\n";

    private TestPackageUpdateManagerImpl m_updater;

    public static junit.framework.Test suite() {
        return new JUnit4TestAdapter(PackageUpdateManagerImplTest.class);
    }

    @Before
    public void initUpdater() {
        m_updater = new TestPackageUpdateManagerImpl();
    }

    @Test
    public void testParsePackageInfo() {
        String commentLine = "# This is a comment";
        Assert.assertNull(m_updater.parsePackageInfo(commentLine));

        String packageLine = "sipxecs|3.11.6|3.11.8";
        PackageUpdate packageUpdate = m_updater.parsePackageInfo(packageLine);
        Assert.assertEquals("sipxecs", packageUpdate.getPackageName());
        Assert.assertEquals("3.11.6", packageUpdate.getCurrentVersion());
        Assert.assertEquals("3.11.8", packageUpdate.getUpdatedVersion());

        String invalidLine = "sipxecs|3.11.6*3.11.8";
        Assert.assertNull(m_updater.parsePackageInfo(invalidLine));
    }

    @Test
    public void testUpdatedVersion() {
        String packageLine = "sipxecs|3.11.6|3.11.8";
        m_updater.parsePackageInfo(packageLine);
        Assert.assertEquals("sipxecs 3.11.8", m_updater.getUpdatedVersion());

        packageLine = "sipxconfig|3.11.6|3.11.10";
        m_updater.parsePackageInfo(packageLine);
        Assert.assertEquals("sipxecs 3.11.8", m_updater.getUpdatedVersion());
    }

    @Test
    public void testGetCurrentVersion() {
        String version = "sipxecs 3.11.6-013602";
        m_updater.setProcessOutput(version);

        Assert.assertEquals(version, m_updater.getCurrentVersion());
    }

    @Test
    public void testCheckForUpdates() {
        // Make sure we start in NO_UPDATES_AVAILABLE state
        Assert.assertEquals(UpdaterState.NO_UPDATES_AVAILABLE, m_updater.getState());

        AlarmContext alarmContext = createMock(AlarmContext.class);
        alarmContext.raiseAlarm("SOFTWARE_UPDATE_AVAILABLE", ALARM_TEXT);
        expectLastCall();
        replay(alarmContext);

        m_updater.setAlarmContext(alarmContext);

        // Do the update check.
        m_updater.setProcessOutput(ALARM_TEXT);
        m_updater.checkForUpdates();

        verify(alarmContext);

        Assert.assertEquals(UpdaterState.UPDATES_AVAILABLE, m_updater.getState());
        Assert.assertEquals(9, m_updater.getAvailablePackages().size());
        Assert.assertEquals("sipxecs 3.11.8-013967", m_updater.getUpdatedVersion());
    }

    @Test
    public void testAlarmOnUpdate() throws Exception {

        // Make sure we start in NO_UPDATES_AVAILABLE state
        Assert.assertEquals(UpdaterState.NO_UPDATES_AVAILABLE, m_updater.getState());

        AlarmContext alarmContext = createMock(AlarmContext.class);
        alarmContext.raiseAlarm("SOFTWARE_UPDATE_AVAILABLE", ALARM_TEXT);
        expectLastCall();
        replay(alarmContext);

        m_updater.setAlarmContext(alarmContext);


        m_updater.setProcessOutput(ALARM_TEXT);

        m_updater.checkForUpdates();

        Assert.assertEquals(UpdaterState.UPDATES_AVAILABLE, m_updater.getState());
        Assert.assertEquals(9, m_updater.getAvailablePackages().size());
        Assert.assertEquals("sipxecs 3.11.8-013967", m_updater.getUpdatedVersion());

        verify(alarmContext);
    }

    private class TestPackageUpdateManagerImpl extends PackageUpdateManagerImpl {

        private String m_processOutput;

        public void setProcessOutput(String processOutput) {
            m_processOutput = processOutput;
        }

        @Override
        public void checkForUpdates() {
            new CheckForUpdatesTask().run(); // run the task in the current thread
        }

        @Override
        protected Process runPackageCommand(String argument) throws IOException {
            Process process = createMock(Process.class);

            process.getInputStream();
            expectLastCall().andReturn(new ByteArrayInputStream(m_processOutput.getBytes()));

            try {
                process.waitFor();
                expectLastCall().andReturn(0);
            } catch (Exception e) {
            }

            org.easymock.classextension.EasyMock.replay(process);

            return process;
        }
    }
}
