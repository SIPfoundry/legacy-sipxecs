/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.phone;

import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class TestPhone extends Phone {
    public static final String BEAN_ID = "testPhone";
    public static final String MODEL_ID = "testPhoneModel";

    public TestPhone() {
        setBeanId(BEAN_ID);
        setModelId(MODEL_ID);
    }

    @Override
    protected Setting loadSettings() {
        return loadSettings("phone.xml");
    }

    Setting loadSettings(String resource) {
        return TestHelper.loadSettings(TestHelper.getResourceAsFile(getClass(), resource));
    }

    @Override
    protected Setting loadLineSettings() {
        return loadSettings("line.xml");
    }

    @Override
    protected LineInfo getLineInfo(Line line) {
        return null;
    }

    @Override
    protected void setLineInfo(Line line, LineInfo lineInfo) {
    }

}
