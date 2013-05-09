package org.sipfoundry.sipxconfig.common;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import org.junit.Before;
import org.junit.Test;

public class BatchCommandRunnerTest {
    private int twoSeconds = 2000;
    private BatchCommandRunner m_runner;
    private SimpleCommandRunner m_c1;
    private SimpleCommandRunner m_c2;
    
    @Before
    public void setUp() {
        m_runner = new BatchCommandRunner("test"); 
        m_c1 = new SimpleCommandRunner();
        m_c2 = new SimpleCommandRunner();
    }

    @Test
    public void testOne() {
        m_runner.setForegroundTimeout(twoSeconds);
        m_c1.setRunParameters("/bin/echo one", twoSeconds, 0);
        m_runner.add(m_c1);
        assertTrue(m_runner.run());
        assertEquals("one\n", m_runner.getStdout());
    }

    @Test
    public void testTwo() {
        m_runner.setForegroundTimeout(twoSeconds);
        m_c1.setRunParameters("/bin/echo one", twoSeconds, 0);
        m_c2.setRunParameters("/bin/echo two", twoSeconds, 0);
        m_runner.add(m_c1);
        m_runner.add(m_c2);
        assertTrue(m_runner.run());
        assertEquals("one\ntwo\n", m_runner.getStdout());
    }
}
