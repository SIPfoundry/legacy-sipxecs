/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.acd;

import java.util.Hashtable;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.type.IntegerSetting;

public class XmlRpcSettingsTest extends TestCase {

    private SettingSet m_setting;
    private Hashtable m_result;

    protected void setUp() throws Exception {
        IntegerSetting type = new IntegerSetting();
        type.setRequired(true);

        SettingImpl s1 = new SettingImpl();
        s1.setType(type);
        s1.setName("integerSetting");
        s1.setValue("4");

        SettingImpl s2 = new SettingImpl();
        s2.setName("stringSetting");
        s2.setValue("bongo");

        m_setting = new SettingSet();
        m_setting.setName("testGroup");
        m_setting.addSetting(s1);
        m_setting.addSetting(s2);

        m_result = new Hashtable();
        m_result.put("result-code", new Integer(1));
    }

    public void testCreate() {
        Hashtable params = new Hashtable();
        params.put("object-class", "testGroup");
        params.put("integerSetting", new Integer(4));
        params.put("stringSetting", "bongo");

        IMocksControl control = EasyMock.createControl();
        Provisioning prov = control.createMock(Provisioning.class);
        prov.create(params);
        control.andReturn(m_result);
        control.replay();

        XmlRpcSettings xmlRpc = new XmlRpcSettings(prov);
        assertTrue(xmlRpc.create(m_setting));

        control.verify();
    }

    public void testDelete() {
        Hashtable params = new Hashtable();
        params.put("object-class", "testGroup");
        params.put("integerSetting", new Integer(4));

        IMocksControl control = EasyMock.createControl();
        Provisioning prov = control.createMock(Provisioning.class);
        prov.delete(params);
        control.andReturn(m_result);
        control.replay();

        XmlRpcSettings xmlRpc = new XmlRpcSettings(prov);
        assertTrue(xmlRpc.delete(m_setting));

        control.verify();
    }

    public void testSet() {
        Hashtable params = new Hashtable();
        params.put("object-class", "testGroup");
        params.put("integerSetting", new Integer(4));
        params.put("stringSetting", "bongo");

        IMocksControl control = EasyMock.createControl();
        Provisioning prov = control.createMock(Provisioning.class);
        prov.set(params);
        control.andReturn(m_result);
        control.replay();

        XmlRpcSettings xmlRpc = new XmlRpcSettings(prov);
        assertTrue(xmlRpc.set(m_setting));

        control.verify();
    }

    public void testGet() {
        Hashtable result = new Hashtable();
        result.put("result-code", new Integer(1));
        result.put("integerSetting", new Integer(17));
        result.put("stringSetting", "kuku");

        Hashtable params = new Hashtable();
        params.put("object-class", "testGroup");
        params.put("integerSetting", new Integer(4));
        params.put("stringSetting", "bongo");

        IMocksControl control = EasyMock.createControl();
        Provisioning prov = control.createMock(Provisioning.class);
        prov.get(params);
        control.andReturn(result);
        control.replay();

        XmlRpcSettings xmlRpc = new XmlRpcSettings(prov);
        assertTrue(xmlRpc.get(m_setting));
        assertEquals("kuku", m_setting.getSetting("stringSetting").getValue());
        assertEquals("17", m_setting.getSetting("integerSetting").getValue());

        control.verify();
    }
}
