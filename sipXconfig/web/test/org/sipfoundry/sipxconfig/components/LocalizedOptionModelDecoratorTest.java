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

import org.apache.hivemind.Messages;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class LocalizedOptionModelDecoratorTest extends TestCase {

    private LocalizedOptionModelDecorator m_out;

    protected void setUp() {
        String[] options = { "a", "b", "c" };
        IPropertySelectionModel model = new StringPropertySelectionModel(options);
        m_out = new LocalizedOptionModelDecorator();
        m_out.setModel(model);
    }

    public void testReturnKeyOnNullMessagesObject() {
        assertEquals("a", m_out.getLabel(0));
    }

    public void testLocalization() {
        IMocksControl messagesControl = EasyMock.createStrictControl();
        Messages messages = messagesControl.createMock(Messages.class);
        messages.getMessage("xyz.a");
        messagesControl.andReturn("localized a");
        messages.getMessage("xyz.b");
        messagesControl.andReturn("b");
        messages.getMessage("c");
        messagesControl.andReturn("localized c");
        messagesControl.replay();

        m_out.setMessages(messages);
        m_out.setResourcePrefix("xyz.");

        assertEquals("localized a", m_out.getLabel(0));
        assertEquals("b", m_out.getLabel(1));

        m_out.setResourcePrefix(null);
        assertEquals("localized c", m_out.getLabel(2));

        messagesControl.verify();
    }
}
