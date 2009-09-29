/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

import org.apache.commons.lang.enums.Enum;
import org.apache.hivemind.Messages;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class EnumFormatTest extends TestCase {

    public void testFormat() throws Exception {
        EnumFormat format = new EnumFormat();
        assertEquals("bongo", format.format(FakeEnum.BONGO));
        assertEquals("kuku", format.format(FakeEnum.KUKU));
        assertEquals("kuku bongo",  format.format(FakeEnum.SPACE));
    }

    public void testLocizedFormat() throws Exception {
        IMocksControl messagesCtrl = EasyMock.createControl();
        Messages messages = messagesCtrl.createMock(Messages.class);
        messages.getMessage("fake.bongo");
        messagesCtrl.andReturn("localized bongo");
        messages.getMessage("fake.kuku");
        messagesCtrl.andReturn("localized kuku");
        messagesCtrl.replay();

        EnumFormat format = new EnumFormat();
        format.setMessages(messages);
        format.setPrefix("fake");

        assertEquals("localized bongo", format.format(FakeEnum.BONGO));
        assertEquals("localized kuku", format.format(FakeEnum.KUKU));
        messagesCtrl.verify();
    }

    public void testSpaceInEnumName() throws Exception {
        IMocksControl messagesCtrl = EasyMock.createControl();
        Messages messages = messagesCtrl.createMock(Messages.class);
        messages.getMessage("fake.kuku_bongo");
        messagesCtrl.andReturn("localized space");
        messagesCtrl.replay();

        EnumFormat format = new EnumFormat();
        format.setMessages(messages);
        format.setPrefix("fake");

        assertEquals("localized space", format.format(FakeEnum.SPACE));
        messagesCtrl.verify();
    }

    static class FakeEnum extends Enum {
        public static final FakeEnum BONGO = new FakeEnum("bongo");
        public static final FakeEnum KUKU = new FakeEnum("kuku");
        public static final FakeEnum SPACE = new FakeEnum("kuku bongo");

        private FakeEnum(String name) {
            super(name);
        }
    }
}
