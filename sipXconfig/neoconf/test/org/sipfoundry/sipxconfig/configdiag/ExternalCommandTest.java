/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.configdiag;

import java.io.File;
import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;

public class ExternalCommandTest extends TestCase {
    private ExternalCommand m_out;
    private String m_command;

    public void setUp() {
        m_out = new ExternalCommand();
        m_command = getClass().getClassLoader().getResource(
                "org/sipfoundry/sipxconfig/admin/configdiag/external-command-test.sh").getFile();
    }

    public void testExecuteExpectSuccess() {
        m_out.setCommand(m_command);
        m_out.addArgument("-s");
        int exitStatus = m_out.execute();
        assertEquals(0, exitStatus);
    }

    public void testExecuteExpectFailure() {
        m_out.setCommand(m_command);
        int exitStatus = m_out.execute();
        assertEquals(1, exitStatus);
        assertEquals("called without -s flag\n", m_out.getStdout());
    }

    public void testExecuteWithArgumentSubstitution() {
        File commandFile = new File(m_command);
        String commandName = commandFile.getName();
        final String path = commandFile.getParent();
        m_out.setCommand(commandName);
        m_out.addArgument("${success}");
        m_out.setContext(new ExternalCommandContext() {
            public String resolveArgumentString(String key) {
                Map<String, String> argMap = new HashMap<String, String>();
                argMap.put("success", "-s");
                return argMap.get(key);
            }

            public String getBinDirectory() {
                return path;
            }
        });

        int exitStatus = m_out.execute();
        assertEquals(0, exitStatus);
    }
}
