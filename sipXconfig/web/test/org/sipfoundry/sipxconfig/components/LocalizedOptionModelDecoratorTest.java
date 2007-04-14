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
    
    private LocalizedOptionModelDecorator m_localized;
    
    protected void setUp() {
        String[] options = { "a", "b", "c" };
        IPropertySelectionModel model = new StringPropertySelectionModel(options);
        m_localized = new LocalizedOptionModelDecorator();        
        m_localized.setModel(model);
    }
    
    public void testReturnKeyOnNullMessagesObject() {
        assertEquals("a", m_localized.getLabel(0));
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
        
        m_localized.setMessages(messages);
        m_localized.setResourcePrefix("xyz.");
        
        assertEquals("localized a", m_localized.getLabel(0));
        assertEquals("b", m_localized.getLabel(1));
        
        m_localized.setResourcePrefix(null);
        assertEquals("localized c", m_localized.getLabel(2));        

        messagesControl.verify();
    }
}
