package org.sipfoundry.sipxconfig.admin.parkorbit;

import java.io.File;

import org.apache.commons.io.FileUtils;
import org.sipfoundry.sipxconfig.IntegrationTestCase;

public class ParkOrbitMigrationTaskTestIntegration extends IntegrationTestCase {

    private static final String DEFAULT_WAV_FILE = "/default.wav";
    private ParkOrbitMigrationTask m_parkOrbitMigrationTask;
    private File m_parkServerDirectory;
    private File m_systemMohDirectory;
    private File m_defaultWav;
    private File m_systemMohDefaultWav;

    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        m_parkServerDirectory = new File(m_parkOrbitMigrationTask.getOrbitsGenerator().getAudioDirectory());
        assertNotNull(m_parkServerDirectory);
        if (!m_parkServerDirectory.exists()) {
            System.out.println("creating dir " + m_parkServerDirectory.getAbsolutePath());
            assertTrue(m_parkServerDirectory.mkdirs());
        }
        m_defaultWav = new File(m_parkServerDirectory + DEFAULT_WAV_FILE);
        m_defaultWav.createNewFile();

        m_systemMohDirectory = new File(m_parkOrbitMigrationTask.getMusicOnHoldManager().getAudioDirectoryPath());
        assertNotNull(m_systemMohDirectory);
        m_systemMohDefaultWav = new File(m_systemMohDirectory.getAbsolutePath() + DEFAULT_WAV_FILE);

        if (!m_systemMohDirectory.exists()) {
            assertTrue(m_systemMohDirectory.mkdirs());
        }
        else if (m_systemMohDefaultWav.exists()) {
            m_systemMohDefaultWav.delete();
        }
    }

    @Override
    protected void onTearDownInTransaction() throws Exception {
        // Try to clean up parkserver and vxml folders
        FileUtils.deleteDirectory(m_parkServerDirectory);
        FileUtils.deleteDirectory(m_systemMohDirectory);
        super.onTearDownInTransaction();
    }

    public void testMigrateParkServer() {
        assertTrue(m_defaultWav.exists());
        assertTrue(!m_systemMohDefaultWav.exists());
        m_parkOrbitMigrationTask.onInitTask("legacy_park_server_migration");
        assertTrue(m_defaultWav.exists());
        assertTrue(m_systemMohDefaultWav.exists());
    }

    public void setParkOrbitMigrationTask(ParkOrbitMigrationTask parkOrbitMigrationTask) {
        m_parkOrbitMigrationTask = parkOrbitMigrationTask;
    }
}
