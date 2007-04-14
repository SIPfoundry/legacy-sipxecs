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
        
        @SettingEntry(path = "woodpecker/ivory-billed")
        public String getObstainFromAnsweringByReturningNull() {
            return null;
        }
    }
}
