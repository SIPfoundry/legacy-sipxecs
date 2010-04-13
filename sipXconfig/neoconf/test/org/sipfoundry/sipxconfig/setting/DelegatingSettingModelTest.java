/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class DelegatingSettingModelTest extends TestCase {

    private SettingModel m_sm;
    private DelegatingSettingModel m_delegatingModel;

    protected void setUp() throws Exception {
        m_sm = createMock(SettingModel.class);

        m_delegatingModel = new DelegatingSettingModel(m_sm, new PrefixFilter());
    }

    protected void tearDown() throws Exception {
        verify(m_sm);
    }

    public void testGetDefaultSettingValue() {
        m_sm.getDefaultSettingValue(null);
        expectLastCall().andReturn(new SettingValueImpl("default"));
        replay(m_sm);

        assertEquals("prefix-default", m_delegatingModel.getDefaultSettingValue(null).getValue());
    }

    public void testGetProfileName() {
        m_sm.getProfileName(null);
        expectLastCall().andReturn(new SettingValueImpl("profileName"));
        replay(m_sm);

        assertEquals("profileName", m_delegatingModel.getProfileName(null).getValue());
    }

    public void testGetSettingValue() {
        m_sm.getDefaultSettingValue(null);
        expectLastCall().andReturn(new SettingValueImpl("value"));
        replay(m_sm);

        assertEquals("prefix-value", m_delegatingModel.getDefaultSettingValue(null).getValue());
    }

    public void testSetSettingValue() {
        m_sm.setSettingValue(null, "value");
        replay(m_sm);

        m_delegatingModel.setSettingValue(null, "value");
    }

    private static class PrefixFilter implements SettingValueFilter {

        public SettingValue filter(SettingValue sv) {
            String value = sv.getValue();
            return new SettingValueImpl("prefix-" + value);
        }

    }
}
