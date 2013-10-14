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

import org.sipfoundry.sipxconfig.intercom.Intercom;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.BeanValueStorage;

public class PolycomIntercomDefaultsTest extends TestCase {
    Phone m_phone;
    private PolycomIntercomDefaultsDummy m_noItercomDefaults;

    @Override
    protected void setUp() throws Exception {
        m_phone = new PolycomPhone();
        PolycomModel model = PolycomXmlTestCase.phoneModelBuilder("polycomVVX500", getClass());
        m_phone.setModel(model);
        m_phone.setDeviceVersion(PolycomModel.VER_3_2_X);
        m_phone.setModelId("polycomVVX500");
        m_noItercomDefaults = new PolycomIntercomDefaultsDummy(null, m_phone);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testGetAlertInfoValue() {
        Intercom intercom = new Intercom();
        intercom.setEnabled(true);
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom, m_phone);
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
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom, m_phone);
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
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom, m_phone);
        intercom.setTimeout(0);
        assertEquals(PolycomIntercomDefaults.AUTO_ANSWER_RING_CLASS_32, defaults.getAlertInfoClass());
        intercom.setTimeout(1000);
        assertEquals(PolycomIntercomDefaults.RING_ANSWER_RING_CLASS_32, defaults.getAlertInfoClass());
    }
    
    public void testGetAlertInfoClass40() throws Exception {
        Intercom intercom = new Intercom();
        intercom.setEnabled(true);

        Phone phone = new PolycomPhone();
        PolycomModel model = PolycomXmlTestCase.phoneModelBuilder("polycomVVX500", getClass());
        phone.setModel(model);
        phone.setDeviceVersion(PolycomModel.VER_4_0_X);
        phone.setModelId("polycomVVX500");
        
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom, phone);
        intercom.setTimeout(0);
        assertEquals(PolycomIntercomDefaults.AUTO_ANSWER_RING_CLASS_40, defaults.getAlertInfoClass());
        intercom.setTimeout(1000);
        assertEquals(PolycomIntercomDefaults.RING_ANSWER_RING_CLASS_40, defaults.getAlertInfoClass());
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
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom, m_phone);
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
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom, m_phone);
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
        PolycomIntercomDefaults defaults = new PolycomIntercomDefaultsDummy(intercom, m_phone);
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

        public PolycomIntercomDefaultsDummy(Intercom intercom, Phone phone) {
            super(phone);
            m_itercom = intercom;
        }

        @Override
        protected Intercom getIntercom() {
            return m_itercom;
        }
    }
}
