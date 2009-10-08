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

import java.util.Arrays;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.ValidationMessages;
import org.apache.tapestry.form.validator.Pattern;
import org.apache.tapestry.form.validator.Required;
import org.apache.tapestry.form.validator.Validator;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.common.NamedObject;
import org.sipfoundry.sipxconfig.gateway.GatewayModel;

/**
 * Comments
 */
public class TapestryUtilsTest extends TestCase {

    Collection m_autoComplete;

    @Override
    protected void setUp() {
        AutoCompleteItems[] itemsData = new AutoCompleteItems[] {
            new AutoCompleteItems("robin"), new AutoCompleteItems("bluejay"), new AutoCompleteItems("hawk"),
            new AutoCompleteItems("harrier"), new AutoCompleteItems("grayjay")
        };
        m_autoComplete = Arrays.asList(itemsData);
    }

    public void testAssertParameterArrayBounds() {
        try {
            TapestryUtils.assertParameter(String.class, null, 0);
            fail();
        } catch (IllegalArgumentException expected) {
            assertTrue(true);
        }
    }

    public void testAssertParameterWrongClass() {
        Object[] strings = new Object[] {
            "i'm a string"
        };
        try {
            TapestryUtils.assertParameter(Integer.class, strings, 0);
            fail();
        } catch (IllegalArgumentException expected) {
            assertTrue(true);
        }
    }

    public void testAssertParameterIsAssignableFrom() {
        Object[] strings = new Object[] {
            "i'm a string"
        };
        TapestryUtils.assertParameter(Object.class, strings, 0);
        assertTrue(true);
    }

    public void testAssertParameterNotAssignableFrom() {
        Object[] objects = new Object[] {
            new Object()
        };
        try {
            TapestryUtils.assertParameter(String.class, objects, 0);
            fail();
        } catch (IllegalArgumentException expected) {
            assertTrue(true);
        }
    }

    public void testGetAutoCompleteCandidates() {
        Collection actual;
        Iterator i;

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, null);
        assertEquals(5, actual.size());

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "");
        assertEquals(5, actual.size());

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, " ");
        assertEquals(5, actual.size());

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "n");
        assertEquals(0, actual.size());

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "r");
        assertEquals(1, actual.size());
        assertEquals("robin", actual.iterator().next());

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "robi");
        assertEquals(1, actual.size());
        assertEquals("robin", actual.iterator().next());

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "robin");
        assertEquals(1, actual.size());
        assertEquals("robin", actual.iterator().next());

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "h");
        assertEquals(2, actual.size());
        i = actual.iterator();
        assertEquals("hawk", i.next());
        assertEquals("harrier", i.next());

        // multi-value
        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "peacock");
        assertEquals(0, actual.size());

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "peacock ");
        assertEquals(5, actual.size());

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "peacock x");
        assertEquals(0, actual.size());

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "peacock h");
        assertEquals(2, actual.size());
        i = actual.iterator();
        assertEquals("peacock hawk", i.next());
        assertEquals("peacock harrier", i.next());

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "bluejay r");
        assertEquals(1, actual.size());
        i = actual.iterator();
        assertEquals("bluejay robin", i.next());
    }

    public void testAutoCompleteCaseInsensitive() {
        Collection actual;
        Iterator i;

        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "Bluejay");
        assertEquals(1, actual.size());
        i = actual.iterator();
        assertEquals("bluejay", i.next());
    }

    public void testAutoCompleteRemoveDuplicates() {
        Collection actual;
        actual = TapestryUtils.getAutoCompleteCandidates(m_autoComplete, "bluejay hawk blue");
        assertEquals(0, actual.size());
    }

    class AutoCompleteItems implements NamedObject {
        String m_name;

        AutoCompleteItems(String name) {
            m_name = name;
        }

        public String getName() {
            return m_name;
        }

        public void setName(String name_) {
        }
    }

    public void testSetExtensionsString() {
        String[] array = TapestryUtils.splitBySpace("100    200");
        assertEquals(2, array.length);
        assertEquals("100", array[0]);
        assertEquals("200", array[1]);
    }

    public void testGetExtensionsString() {
        String s = TapestryUtils.joinBySpace(new String[] {
            "100 200"
        });
        assertEquals("100 200", s);
    }

    public void testGetSerialNumberValidators() throws Exception {
        ValidationMessages messages = EasyMock.createNiceMock(ValidationMessages.class);
        IFormComponent fc = EasyMock.createNiceMock(IFormComponent.class);

        EasyMock.replay(messages, fc);

        GatewayModel model = new GatewayModel();
        model.setSerialNumberPattern("^FT\\d{7,}$");
        String serialNumber = "FT0123456";

        List<Validator> serialNumberValidators = TapestryUtils.getSerialNumberValidators(model);
        assertEquals(2, serialNumberValidators.size());
        assertTrue(serialNumberValidators.get(0) instanceof Required);
        assertTrue(serialNumberValidators.get(1) instanceof Pattern);
        for (Validator v : serialNumberValidators) {
            v.validate(fc, messages, serialNumber);
        }

        EasyMock.verify(messages, fc);
    }

    public void testGetSerialNumberValidatorsNoPattern() throws Exception {
        GatewayModel model = new GatewayModel();
        model.setSerialNumberPattern("");

        List<Validator> serialNumberValidators = TapestryUtils.getSerialNumberValidators(model);
        assertEquals(1, serialNumberValidators.size());
        assertTrue(serialNumberValidators.get(0) instanceof Required);
    }
}
