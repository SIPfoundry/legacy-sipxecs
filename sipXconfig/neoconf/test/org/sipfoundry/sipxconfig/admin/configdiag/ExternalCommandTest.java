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

import junit.framework.TestCase;

public class ExternalCommandTest extends TestCase {
    private ExternalCommand m_out;
    
    public void setUp() {
        m_out = new ExternalCommand();
        String command = getClass().getClassLoader().getResource("org/sipfoundry/sipxconfig/admin/configdiag/external-command-test.sh").getFile();
        m_out.setCommand(command);
    }
    
    public void testExecuteExpectSuccess() {
        m_out.addArgument("-s");
        int exitStatus = m_out.execute();
        assertEquals(0, exitStatus);
    }
    
    public void testExecuteExpectFailure() {
        int exitStatus = m_out.execute();
        assertEquals(1, exitStatus);
    }
}
