/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site;

import junit.framework.TestCase;

import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.ValidationMessages;
import org.apache.tapestry.form.validator.Pattern;
import org.apache.tapestry.valid.ValidatorException;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.springframework.context.ApplicationContext;
import org.springframework.context.support.ClassPathXmlApplicationContext;

/**
 * Test regular expressions for validators defined in tapestry.xml
 */
public class ValidatorsTest extends TestCase {
    private IMocksControl m_validationMessagesControl;
    private ValidationMessages m_validationMessages;
    private ApplicationContext m_context;
    private IFormComponent m_field;
    
    
    protected void setUp() { 
        m_context = new ClassPathXmlApplicationContext("org/sipfoundry/sipxconfig/site/tapestry.xml");
        
        m_validationMessagesControl = EasyMock.createNiceControl();
        m_validationMessages = m_validationMessagesControl.createMock(ValidationMessages.class);
        m_validationMessagesControl.replay();
        
        IMocksControl fieldControl = EasyMock.createNiceControl();
        m_field = fieldControl.createMock(IFormComponent.class);        
        fieldControl.replay();
    }
    
    public void testValidPhoneOrAor() throws ValidatorException {
        Pattern p = (Pattern) m_context.getBean("validPhoneOrAor");
        p.validate(m_field, m_validationMessages, "123");
        p.validate(m_field, m_validationMessages, "abc@abc.com");
        p.validate(m_field, m_validationMessages, "a@abc.com");        
    }
}
