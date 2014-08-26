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

import junit.framework.TestCase;

public class ConfigChangeTest extends TestCase {

    public void testConfigChangeFields() {
        ConfigChange configChange = new ConfigChange();

        configChange.setConfigChangeAction(ConfigChangeAction.ADDED);
        assertEquals(ConfigChangeAction.ADDED,
                configChange.getConfigChangeAction());
        configChange.setConfigChangeType(ConfigChangeType.PHONE.getName());
        assertEquals(ConfigChangeType.PHONE.getName(), configChange.getConfigChangeType());

        configChange.setIpAddress("localhost");
        configChange.setUserName("superadmin");

        ConfigChangeValue configChangeValue = new ConfigChangeValue(configChange);
        configChangeValue.setPropertyName("description");
        configChangeValue.setValueBefore("oldDescription");
        configChangeValue.setValueAfter("newDescription");
        configChange.addValue(configChangeValue);
        assertEquals(configChangeValue, configChange.getValues().get(0));
        assertEquals(1, configChange.getValues().size());
    }

    public void testConfigChangeValueFields() {
        ConfigChangeValue configChangeValue = new ConfigChangeValue(new ConfigChange());
        configChangeValue.setPropertyName("description");
        assertEquals("description", configChangeValue.getPropertyName());
        configChangeValue.setValueBefore("oldDescription");
        assertEquals("oldDescription", configChangeValue.getValueBefore());
        configChangeValue.setValueAfter("newDescription");
        assertEquals("newDescription", configChangeValue.getValueAfter());
    }
}
