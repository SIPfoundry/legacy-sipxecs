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

import java.io.File;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.setting.type.BooleanSetting;
import org.sipfoundry.sipxconfig.setting.type.SettingType;

public class SettingTypeReferencesTest extends TestCase {

    private Setting group;

    protected void setUp() throws Exception {
        ModelBuilder builder = new XmlModelBuilder("etc");
        File in = TestHelper.getResourceAsFile(getClass(), "setting-type-references.xml");
        SettingSet root = builder.buildModel(in);
        group = root.getSetting("group");
    }

    public void testSettingRefidResolved() {
        SettingType type = group.getSetting("true-false-setting").getType();
        assertNotNull(type);
        assertTrue(type instanceof BooleanSetting);
    }
}
