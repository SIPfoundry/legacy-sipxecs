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

    @Override
    protected void setUp() {
        m_context = new ClassPathXmlApplicationContext("org/sipfoundry/sipxconfig/site/tapestry.xml");

        m_validationMessagesControl = EasyMock.createNiceControl();
        m_validationMessages = m_validationMessagesControl.createMock(ValidationMessages.class);
        m_validationMessagesControl.replay();

        IMocksControl fieldControl = EasyMock.createNiceControl();
        m_field = fieldControl.createMock(IFormComponent.class);
        fieldControl.replay();
    }

    public void testValidPhone() throws ValidatorException {
        Pattern p = (Pattern) m_context.getBean("validPhone");
        p.validate(m_field, m_validationMessages, "123");
        p.validate(m_field, m_validationMessages, "*123");
        p.validate(m_field, m_validationMessages, "*12*3");

        try {
            p.validate(m_field, m_validationMessages, "@123");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
    }

    public void testValidPhoneSequence() throws ValidatorException {
        Pattern p = (Pattern) m_context.getBean("validPhoneSequence");
        p.validate(m_field, m_validationMessages, "123 345");
        p.validate(m_field, m_validationMessages, "*123");
        p.validate(m_field, m_validationMessages, "*12*3 **");

        try {
            p.validate(m_field, m_validationMessages, "@123 12");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
    }

    public void testValidPhoneOrAor() throws ValidatorException {
        Pattern p = (Pattern) m_context.getBean("validPhoneOrAor");
        p.validate(m_field, m_validationMessages, "123");
        p.validate(m_field, m_validationMessages, "*123");
        p.validate(m_field, m_validationMessages, "abc@abc.com");
        p.validate(m_field, m_validationMessages, "a@abc.com");
        p.validate(m_field, m_validationMessages, "a*c@abc.com");
        p.validate(m_field, m_validationMessages, "*c@abc.com");
        p.validate(m_field, m_validationMessages, "$a@abc.com");
    }

    public void testValidHostOrIp() throws ValidatorException {
        Pattern p = (Pattern) m_context.getBean("validHostOrIp");
        p.validate(m_field, m_validationMessages, "10.1.1.25");
        p.validate(m_field, m_validationMessages, "192.1.15.149");
        p.validate(m_field, m_validationMessages, "111.11.111.11");
        p.validate(m_field, m_validationMessages, "0.0.0.0");
        p.validate(m_field, m_validationMessages, "255.255.255.255");
        p.validate(m_field, m_validationMessages, "abc.com");
        p.validate(m_field, m_validationMessages, "www.abc.com");
        p.validate(m_field, m_validationMessages, "server.domain123.com");
        p.validate(m_field, m_validationMessages, "server.domain-name.com");
        p.validate(m_field, m_validationMessages, "abc.us");
        p.validate(m_field, m_validationMessages, "machine1.domain1.co.uk");
        p.validate(m_field, m_validationMessages, "m1.d2.net");
        p.validate(m_field, m_validationMessages, "3com.net");
        p.validate(m_field, m_validationMessages, "www.a.com");
        p.validate(m_field, m_validationMessages, "localhost");
        p.validate(m_field, m_validationMessages, "localhost.localdomain");
        p.validate(m_field, m_validationMessages, "si-px-33.sipfoundry.org");
        p.validate(m_field, m_validationMessages, "sipx-3.sipfoundry.org");
        p.validate(m_field, m_validationMessages, "a.b.c.d.sipx-3.sipfoundry.org");

        // sipX does not support IPv6 - but when it does...
        // p.validate(m_field, m_validationMessages, "2001:0db8:85a3:08d3:1319:8a2e:0370:7334");
        // p.validate(m_field, m_validationMessages, "7:8:9:A:B:CC:DDD:FFFF");

        // invalid hosts and ip's:
        try {
            p.validate(m_field, m_validationMessages, "\'testing-single-quotes\'");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }

        try {
            p.validate(m_field, m_validationMessages, "\"testing-double-quotes\"");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }

        // invalid IPv6:
        try {
            p.validate(m_field, m_validationMessages, "1111:2222:3333:4444:5555:6666:7777");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }

        try {
            p.validate(m_field, m_validationMessages, "GGGG:aaaa:aaaa:aaaa:aaaa:aaaa:aaaa:aaaa");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }

        // invalid Domain:
        try {
            p.validate(m_field, m_validationMessages, "-domain.org");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }

        try {
            p.validate(m_field, m_validationMessages, "www-.domain.org");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
    }

    public void testValidIPv4() throws ValidatorException {
        Pattern p = (Pattern) m_context.getBean("validIPv4");
        p.validate(m_field, m_validationMessages, "10.1.1.0");
        p.validate(m_field, m_validationMessages, "192.1.15.149");
        p.validate(m_field, m_validationMessages, "111.11.111.11");
        p.validate(m_field, m_validationMessages, "0.0.0.0");
        p.validate(m_field, m_validationMessages, "255.255.255.255");
        try {
            p.validate(m_field, m_validationMessages, "192.168.0.256");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "192.168.0.1.1");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "111.11.1");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "111.11.1.abc");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "@.11.1.11");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "user123");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "1-1-1-1");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "111111111");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
    }

    public void testValidExtensions() throws ValidatorException {
        Pattern p = (Pattern) m_context.getBean("validExtensions");
        p.validate(m_field, m_validationMessages, "123");
        p.validate(m_field, m_validationMessages, "123 456");
        p.validate(m_field, m_validationMessages, "123  456");
        try {
            p.validate(m_field, m_validationMessages, " 123");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            // successful test
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "123 @");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "! 456");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "john 456");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "123,456");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            assertTrue(true);
        }
    }

    public void testValidPagingPrefix() throws ValidatorException {
        Pattern p = (Pattern) m_context.getBean("validPagingPrefix");
        p.validate(m_field, m_validationMessages, "*");
        p.validate(m_field, m_validationMessages, "77");
        p.validate(m_field, m_validationMessages, "*77");
        p.validate(m_field, m_validationMessages, "77*");
        p.validate(m_field, m_validationMessages, "7*7");
        p.validate(m_field, m_validationMessages, "*7*7");
        p.validate(m_field, m_validationMessages, "7*7*");
        p.validate(m_field, m_validationMessages, "*77*");
        p.validate(m_field, m_validationMessages, "*7*7*");

        try {
            p.validate(m_field, m_validationMessages, "-7");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            assertTrue(true);
        }

        try {
            p.validate(m_field, m_validationMessages, "$7");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            assertTrue(true);
        }

        try {
            p.validate(m_field, m_validationMessages, "!");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            assertTrue(true);
        }
    }

    public void testValidDidNumber() throws ValidatorException {
        Pattern p = (Pattern) m_context.getBean("validDidNumber");
        p.validate(m_field, m_validationMessages, "");
        p.validate(m_field, m_validationMessages, "+0123");
        p.validate(m_field, m_validationMessages, "0123");
        try {
            p.validate(m_field, m_validationMessages, "7+1");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "-7");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "!7");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            assertTrue(true);
        }
        try {
            p.validate(m_field, m_validationMessages, "+");
            fail("Should throw a ValidatorException");
        } catch (ValidatorException expected) {
            assertTrue(true);
        }

    }
}
