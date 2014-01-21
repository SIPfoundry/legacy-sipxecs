package org.sipfoundry.sipxconfig.backup;

import static org.junit.Assert.assertTrue;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Map;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class BackupRunnerTest {
    private BackupRunnerImpl m_runner;
    
    @Before
    public void setUp() {
        m_runner = new BackupRunnerImpl();
        String script = getClass().getResource("mock-backup.sh").getFile();
        m_runner.setBackupScript(script);
    }

    @Test
    public void list() throws IOException {
        String plan = getClass().getResource("expected-auto-backup.yaml").getFile();        
        Map<String, List<String>> actual = m_runner.list(new File(plan));
        String expected = IOUtils.toString(getClass().getResourceAsStream("expected-listing.json"));
        TestHelper.assertEquals(expected, actual);
    }

    @Test
    public void backup() {
        assertTrue(m_runner.backup(new File("foo")));
    }
}
