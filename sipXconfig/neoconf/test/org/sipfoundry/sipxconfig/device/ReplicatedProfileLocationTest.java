/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.device;

import java.io.OutputStream;

import junit.framework.JUnit4TestAdapter;

import org.junit.Test;
import org.sipfoundry.sipxconfig.admin.ConfigurationFile;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.eq;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import static org.junit.Assert.assertEquals;

public class ReplicatedProfileLocationTest {
    public static junit.framework.Test suite() {
        return new JUnit4TestAdapter(ReplicatedProfileLocationTest.class);
    }

    @Test
    public void testGetOutput() throws Exception {
        ConfigurationFile config = new InMemoryConfiguration("/etc/sipxpbx", "orbits.xml", "abcd");
        SipxReplicationContext rc = createMock(SipxReplicationContext.class);
        rc.replicate(eq(config));
        expectLastCall();
        replay(rc);

        ReplicatedProfileLocation location = new ReplicatedProfileLocation();
        location.setName("orbits.xml");
        location.setDirectory("/etc/sipxpbx");
        location.setReplicationContext(rc);

        OutputStream output = location.getOutput("abc.txt");
        for (char c = 'a'; c < 'e'; c++) {
            output.write(c);
        }
        location.closeOutput(output);
        assertEquals("abcd", location.toString());

        verify(rc);
    }
}
