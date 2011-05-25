/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.polycom;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.intercom.Intercom;
import org.sipfoundry.sipxconfig.setting.BeanValueStorage;

public class PolycomIntercomDefaultsTest extends TestCase {
    private PolycomIntercomDefaultsDummy m_noItercomDefaults;

    @Override
    protected void setUp() throws Exception {
        m_noItercomDefaults = new PolycomIntercomDefaultsDummy(null);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testGetAlertInfoValue() {
        Intercom intercom = new Intercom();
        intercom.setEnabled(true);
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom);
        intercom.setCode("abc");
        assertEquals("abc", defaults.getAlertInfoValue());
    }

    public void testGetAlertInfoValueNoIntercom() {
        try {
            m_noItercomDefaults.getAlertInfoValue();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
    }

    public void testGetAlertInfoValueDisabled() {
        Intercom intercom = new Intercom();
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom);
        intercom.setCode("abc");
        try {
            defaults.getAlertInfoValue();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
    }

    public void testGetAlertInfoClass() {
        Intercom intercom = new Intercom();
        intercom.setEnabled(true);
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom);
        intercom.setTimeout(0);
        assertEquals(PolycomIntercomDefaults.AUTO_ANSWER_RING_CLASS, defaults.getAlertInfoClass());
        intercom.setTimeout(1000);
        assertEquals(PolycomIntercomDefaults.RING_ANSWER_RING_CLASS, defaults.getAlertInfoClass());
    }

    public void testGetAlertInfoClassNoIntercom() {
        try {
            m_noItercomDefaults.getAlertInfoClass();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
    }

    public void testGetAlertInfoClassDisabled() {
        Intercom intercom = new Intercom();
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom);
        intercom.setTimeout(0);
        try {
            defaults.getAlertInfoClass();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
        intercom.setTimeout(1000);
        try {
            defaults.getAlertInfoClass();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
    }

    public void testGetRingAnswerTimeout() {
        Intercom intercom = new Intercom();
        intercom.setEnabled(true);
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom);
        intercom.setTimeout(0);
        try {
            defaults.getRingAnswerTimeout();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
        intercom.setTimeout(1000);
        assertEquals(1000, defaults.getRingAnswerTimeout());
    }

    public void testGetRingAnswerTimeoutNoIntercom() {
        try {
            m_noItercomDefaults.getRingAnswerTimeout();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
    }

    public void testGetRingAnswerTimeoutDisabled() {
        Intercom intercom = new Intercom();
        intercom.setTimeout(0);
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom);
        try {
            defaults.getRingAnswerTimeout();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
        intercom.setTimeout(1000);
        try {
            defaults.getRingAnswerTimeout();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
    }

    public static class PolycomIntercomDefaultsDummy extends PolycomIntercomDefaults {
        private Intercom m_itercom;

        public PolycomIntercomDefaultsDummy(Intercom intercom) {
            super(null);
            m_itercom = intercom;
        }

        @Override
        protected Intercom getIntercom() {
            return m_itercom;
        }
    }
}
