/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.configdb;

import java.util.Arrays;
import java.util.Hashtable;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;

public class ConfigDbSettingAdaptorTest extends TestCase {

    private SettingSet m_group;
    private SettingImpl m_s1;
    private SettingImpl m_s2;

    public void setUp() {
        m_group = new SettingSet();
        m_group.setProfileName("dbname-config");

        m_s1 = new SettingImpl();
        m_s1.setName("1");
        m_s1.setProfileName("s1");
        m_s1.setValue("s1_value");
        m_s2 = new SettingImpl();
        m_s2.setName("2");
        m_s2.setProfileName("s2");
        m_s2.setValue("s2_value");
        m_group.addSetting(m_s1);
        m_group.addSetting(m_s2);
    }

    public void testSet() {
        IMocksControl dbCtrl = EasyMock.createControl();
        ConfigDbParameter db = dbCtrl.createMock(ConfigDbParameter.class);

        Hashtable params = new Hashtable();
        params.put("s1", "s1_value");
        params.put("s2", "s2_value");
        db.set("dbname-config", params);
        dbCtrl.andReturn(2);

        dbCtrl.replay();

        ConfigDbSettingAdaptor adaptor = new ConfigDbSettingAdaptor();
        adaptor.setConfigDbParameter(db);

        assertTrue(adaptor.set(m_group));

        dbCtrl.verify();
    }

    public void testSetCollection() {
        IMocksControl dbCtrl = EasyMock.createControl();
        ConfigDbParameter db = dbCtrl.createMock(ConfigDbParameter.class);

        Hashtable params = new Hashtable();
        params.put("s1", "s1_value");
        params.put("s2", "s2_value");
        db.set("dbname-config", params);
        dbCtrl.andReturn(2);

        dbCtrl.replay();

        ConfigDbSettingAdaptor adaptor = new ConfigDbSettingAdaptor();
        Setting[] setting = {
            m_s1, m_s2
        };
        adaptor.setConfigDbParameter(db);

        assertTrue(adaptor.set("dbname-config", Arrays.asList(setting)));

        dbCtrl.verify();
    }

    public void testGet() {
        IMocksControl dbCtrl = EasyMock.createControl();

        ConfigDbParameter db = dbCtrl.createMock(ConfigDbParameter.class);

        String[] dbnameParams = new String[] {
            "s1", "s2"
        };
        db.get(EasyMock.eq("dbname-config"), EasyMock.aryEq(dbnameParams));

        Hashtable params = new Hashtable();
        params.put("s1", "s1_value");
        params.put("s2", "s2_value");
        dbCtrl.andReturn(params);

        dbCtrl.replay();

        ConfigDbSettingAdaptor adaptor = new ConfigDbSettingAdaptor();
        adaptor.setConfigDbParameter(db);
        assertTrue(adaptor.get(m_group));

        assertEquals("s1_value", m_group.getSetting("1").getValue());
        assertEquals("s2_value", m_group.getSetting("2").getValue());

        dbCtrl.verify();
    }
}
