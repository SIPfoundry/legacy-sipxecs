/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel12x0;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.intercom.Intercom;
import org.sipfoundry.sipxconfig.setting.BeanValueStorage;

public class Nortel12x0IntercomDefaultsTest extends TestCase {
    private Nortel12x0IntercomDefaultsDummy m_noItercomDefaults;

    @Override
    protected void setUp() throws Exception {
        m_noItercomDefaults = new Nortel12x0IntercomDefaultsDummy(null);
    }

    @Override
    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testGetAlertInfoValue() {
        Intercom intercom = new Intercom();
        intercom.setEnabled(true);
        Nortel12x0IntercomDefaults defaults = new Nortel12x0IntercomDefaultsDummy(intercom);
        intercom.setCode("querty");
        assertEquals("querty", defaults.getAlertInfoValue());
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
        Nortel12x0IntercomDefaults defaults = new Nortel12x0IntercomDefaultsDummy(intercom);
        intercom.setCode("querty");
        try {
            defaults.getAlertInfoValue();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
    }

    public void testGetIntercomPrefixValue() {
        Intercom intercom = new Intercom();
        intercom.setEnabled(true);
        Nortel12x0IntercomDefaults defaults = new Nortel12x0IntercomDefaultsDummy(intercom);
        intercom.setPrefix("*76");
        assertEquals("*76", defaults.getIntercomPrefixValue());
    }

    public void testGetIntercomPrefixValueNoIntercom() {
        try {
            m_noItercomDefaults.getIntercomPrefixValue();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
    }

    public void testGetIntercomPrefixValueDisabled() {
        Intercom intercom = new Intercom();
        Nortel12x0IntercomDefaults defaults = new Nortel12x0IntercomDefaultsDummy(intercom);
        intercom.setPrefix("*76");
        try {
            defaults.getIntercomPrefixValue();
            fail("exception expected");
        } catch (BeanValueStorage.NoValueException e) {
            // ok
        }
    }

    public static class Nortel12x0IntercomDefaultsDummy extends Nortel12x0IntercomDefaults {
        private Intercom m_itercom;

        public Nortel12x0IntercomDefaultsDummy(Intercom intercom) {
            super(null);
            m_itercom = intercom;
        }

        @Override
        protected Intercom getIntercom() {
            return m_itercom;
        }
    }
}
