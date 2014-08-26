/**
 *
 *
 * Copyright (c) 2014 Karel, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.systemaudit;

import java.util.List;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class ConfigChangeContextTestIntegration extends IntegrationTestCase {

    private ConfigChangeContext m_configChangeContext;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testStoreConfigChange() throws SystemAuditException {
        ConfigChange configChange = new ConfigChange();
        configChange.setConfigChangeAction(ConfigChangeAction.ADDED);
        configChange.setConfigChangeType(ConfigChangeType.PHONE.getName());

        configChange.setUserName("superadmin");
        configChange.setIpAddress("localhost");

        ConfigChangeValue configChangeValue = new ConfigChangeValue(configChange);
        configChangeValue.setPropertyName("description");
        configChangeValue.setValueBefore("oldDescription");
        configChangeValue.setValueAfter("newDescription");
        configChange.addValue(configChangeValue);

        m_configChangeContext.storeConfigChange(configChange);
        assertEquals(1, m_configChangeContext.getConfigChanges().size());
        assertNotNull(configChange.getDateTime());
        assertNotSame(-1, configChange.getId());
        assertNotSame(-1, configChangeValue.getId());
    }

    public void testUserIpAddressMissingUserName() {
        ConfigChange configChange = new ConfigChange();
        configChange.setConfigChangeAction(ConfigChangeAction.ADDED);
        configChange.setConfigChangeType(ConfigChangeType.PHONE.getName());
        configChange.setUserName("superadmin");
        configChange.setIpAddress("localhost");
        try {
            m_configChangeContext.storeConfigChange(configChange);
            assertEquals(1, m_configChangeContext.getConfigChanges().size());
        } catch (Exception e) {
            assertTrue(e.getMessage().contains("ConstraintViolationException"));
        }
    }

    public void testUserIpAddressMissingIpAddress() {
        ConfigChange configChange = new ConfigChange();
        configChange.setConfigChangeType(ConfigChangeType.PHONE.getName());
        configChange.setUserName("superadmin");
        configChange.setIpAddress("localhost");
        try {
            m_configChangeContext.storeConfigChange(configChange);
        } catch (Exception e) {
            assertTrue(e.getMessage().contains("ConstraintViolationException"));
        }
    }

    public void testGetConfigChanges() throws Exception {
        TestHelper.cleanInsertFlat("systemaudit/config_change.db.xml");

        List<ConfigChange> configChanges = m_configChangeContext
                .getConfigChanges();
        assertEquals(3, configChanges.size());

        ConfigChange configChange = configChanges.get(2);
        assertEquals(40, configChange.getId().intValue());
        assertEquals("0004f2842306", configChange.getDetails());
        assertEquals(ConfigChangeAction.MODIFIED,
                configChange.getConfigChangeAction());
        assertEquals(ConfigChangeType.PHONE.getName(), configChange.getConfigChangeType());
        List<ConfigChangeValue> configChangeValues = configChange.getValues();
        assertEquals(1, configChangeValues.size());
        ConfigChangeValue configChangeValue = configChangeValues.get(0);
        assertEquals(3, configChangeValue.getId().intValue());
        assertEquals("description", configChangeValue.getPropertyName());
        assertEquals("description1", configChangeValue.getValueBefore());
        assertEquals("description2", configChangeValue.getValueAfter());
    }

    public void setConfigChangeContext(ConfigChangeContext configChangeContext) {
        m_configChangeContext = configChangeContext;
    }
}
