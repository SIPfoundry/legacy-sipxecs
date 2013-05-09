package org.sipfoundry.sipxconfig.common;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import org.apache.commons.lang.StringUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.common.SimpleCommandRunner;

public class SimpleCommandRunnerTest {
    private SimpleCommandRunner m_runner;
    
    @Before
    public void setUp() {
        m_runner = new SimpleCommandRunner();        
    }
    
    @Test
    public void stdin() throws InterruptedException {
        m_runner.setStdin("hello");
        m_runner.run(new String[] {"/bin/cat"}, 100);
        assertEquals("hello", m_runner.getStdout());        
    }
    
    @Test
    public void stdout() throws InterruptedException {
        m_runner.run(new String[] {"/bin/echo", "hi"}, 100);
        assertEquals("hi\n", m_runner.getStdout());        
    }    

    @Test
    public void finishedInForeground() throws InterruptedException {
        assertTrue(m_runner.run(new String[] {"/bin/sleep", ".5"}, 2000));
        assertFalse(m_runner.isInProgress());
        assertNotNull(m_runner.getExitCode());
    }
    
    @Test
    public void finishedInBackground() throws InterruptedException {
        // if this test fails, it's possible machine is so slow, the timing
        // doesn't work out.  if so increase some of the time margins, i just
        // kept it small so test is reasonably fast --Douglas
        assertFalse(m_runner.run(new String[] {"/bin/sleep", ".25"}, 10, 1000));
        assertTrue(m_runner.isInProgress());
        Thread.sleep(500);
        assertFalse(m_runner.isInProgress());
        assertNotNull(m_runner.getExitCode());        
    }
    
    @Test
    public void killedInBackground() throws InterruptedException {
        assertFalse(m_runner.run(new String[] {"/bin/sleep", "5"}, 100, 150));
        assertTrue(m_runner.isInProgress());
        Thread.sleep(500);
        assertNull(m_runner.getExitCode());    
        assertFalse(m_runner.isInProgress());
    }
    
    @Test
    public void killedInForeground() throws InterruptedException {
        assertFalse(m_runner.run(new String[] {"/bin/sleep", ".5"}, 100));
        assertFalse(m_runner.isInProgress());
        assertNull(m_runner.getExitCode());    
    }
    
    @Test
    public void splitCommand() {
        assertEquals("aaa|bbb", StringUtils.join(SimpleCommandRunner.split("aaa bbb"), "|"));
        assertEquals("aaa|bbb", StringUtils.join(SimpleCommandRunner.split("aaa  bbb"), "|"));
        assertEquals("aaa|bbb ccc", StringUtils.join(SimpleCommandRunner.split("aaa 'bbb ccc'"), "|"));
        assertEquals("aaa|bbb ccc|ddd", StringUtils.join(SimpleCommandRunner.split("aaa 'bbb ccc' ddd"), "|"));
    }
}
