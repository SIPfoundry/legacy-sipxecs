package org.sipfoundry.sipxconfig.cfgmgt;

import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.util.regex.Pattern;

import org.junit.Test;

public class ConfigManagerTest {

    @Test
    public void test() {
        ConfigManager cfg = new ConfigManagerImpl();
        String expected = "/usr/bin/ssh -i /.+/.cfagent/ppkeys/localhost.nopass.priv root@foo";
        String actual = cfg.getRemoteCommand("foo");
        if (!Pattern.matches(expected, actual)) {
            fail(String.format("'%s' ~= '%s'", expected, actual));            
        }
        assertTrue(true);
    }
}
