/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */

package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;
import org.apache.hivemind.Messages;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class CompositeMessagesTest extends TestCase {
    public void testEmpty() throws Exception {
        CompositeMessages cm = new CompositeMessages();
        assertEquals("[BIRD]", cm.getMessage("bird"));
        assertEquals("[BIRD]", cm.format("bird", 1, 2));
    }

    public void testOne() throws Exception {
        Messages m = createMock(Messages.class);
        m.getMessage("bird");
        expectLastCall().andReturn("Eagle");
        m.getMessage("message");
        expectLastCall().andReturn("Lucky numbers are: {0}, {1}");

        replay(m);

        CompositeMessages cm = new CompositeMessages(m);
        assertEquals("Eagle", cm.getMessage("bird"));
        assertEquals("Lucky numbers are: 1, 2", cm.format("message", 1, 2));

        verify(m);
    }

    public void testMany() throws Exception {
        Messages m1 = createMock(Messages.class);
        m1.getMessage("message");
        expectLastCall().andReturn("Lucky numbers are: {0}, {1}");
        m1.getMessage("bird");
        expectLastCall().andReturn("[BIRD]");

        Messages m2 = createMock(Messages.class);
        m2.getMessage("bird");
        expectLastCall().andReturn("Eagle");

        replay(m1, m2);

        CompositeMessages cm = new CompositeMessages(m1, m2);
        assertEquals("Eagle", cm.getMessage("bird"));
        assertEquals("Lucky numbers are: 1, 2", cm.format("message", 1, 2));

        verify(m1, m2);
    }
}
