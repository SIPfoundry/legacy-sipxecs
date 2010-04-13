/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.FileNotFoundException;
import java.io.IOException;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;

public class BeanValueStorageTest extends TestCase {

    BeanValueStorage m_vs;
    Setting m_birds;

    protected void setUp() throws IOException {
        BeanValueStorageTestBean bean = new BeanValueStorageTestBean();
        m_vs = new BeanValueStorage(bean);
        m_birds = TestHelper.loadSettings(getClass(), "birds.xml");
    }

    public void testGetSettingValue() {
        Setting s = m_birds.getSetting("towhee/canyon");
        assertEquals("14 inches", m_vs.getSettingValue(s).getValue());
    }

    public void testObstainFromGetSettingValue() {
        Setting s = m_birds.getSetting("woodpecker/ivory-billed");
        assertNull(m_vs.getSettingValue(s));
    }

    public void testGetSettingValuePathArray() {
        Setting s1 = m_birds.getSetting("flycatcher/peewee");
        assertEquals(".25 inches", m_vs.getSettingValue(s1).getValue());
        Setting s2 = m_birds.getSetting("flycatcher/willow");
        assertEquals(".25 inches", m_vs.getSettingValue(s2).getValue());
    }

    public void testNoValue() {
        Setting s1 = m_birds.getSetting("pigeon/passenger");
        assertNull(m_vs.getSettingValue(s1));
    }

    public void testRuntimeException() {
        try {
            Setting s = m_birds.getSetting("pigeon/rock-dove");
            m_vs.getSettingValue(s);
            fail("Expected exception");
        } catch (NullPointerException e) {
            // OK
        }
    }

    public void testException() {
        try {
            Setting s = m_birds.getSetting("woodpecker/red-bellied");
            m_vs.getSettingValue(s);
            fail("Expected exception");
        } catch (RuntimeException e) {
            assertTrue(e.getCause() instanceof FileNotFoundException);
        }
    }

    static class BeanValueStorageTestBean {

        @SettingEntry(path = "towhee/canyon")
        public String getFunkyLittleBird() {
            return "14 inches";
        }

        @SettingEntry(paths = {
            "flycatcher/peewee", "flycatcher/willow"
        })
        public String getFlycathers() {
            return ".25 inches";
        }

        @SettingEntry(path = "pigeon/passenger")
        public String getSomeValue() {
            throw new BeanValueStorage.NoValueException();
        }

        @SettingEntry(path = "pigeon/rock-dove")
        public String getRuntimeException() {
            throw new NullPointerException();
        }

        @SettingEntry(path = "woodpecker/red-bellied")
        public String getException() throws IOException {
            throw new FileNotFoundException();
        }

        @SettingEntry(path = "woodpecker/ivory-billed")
        public String getObstainFromAnsweringByReturningNull() {
            return null;
        }
    }
}
