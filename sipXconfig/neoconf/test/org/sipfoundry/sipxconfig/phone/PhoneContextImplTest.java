/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.Arrays;
import java.util.Collection;

import junit.framework.TestCase;

import org.apache.commons.collections.CollectionUtils;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.alarm.AlarmDefinition;
import org.sipfoundry.sipxconfig.alarm.AlarmServerManagerImpl;
import org.sipfoundry.sipxconfig.intercom.IntercomManager;

public class PhoneContextImplTest extends TestCase {
    private PhoneContextImpl m_impl;

    protected void setUp() {
        m_impl = new PhoneContextImpl();
    }

    // The method getIntercomForPhone simply forwards to the intercom manager
    public void testGetIntercomForPhone() {
        IntercomManager intercomManager = EasyMock.createMock(IntercomManager.class);
        Phone phone = new TestPhone();
        intercomManager.getIntercomForPhone(phone);
        EasyMock.expectLastCall().andReturn(null).anyTimes();
        EasyMock.replay(intercomManager);
        m_impl.setIntercomManager(intercomManager);

        m_impl.getIntercomForPhone(phone);
        EasyMock.verify(intercomManager);
    }

    public void testGetAlarms() {
        Collection<AlarmDefinition> alarms = m_impl.getAvailableAlarms(new AlarmServerManagerImpl());
        assertTrue(CollectionUtils.isEqualCollection(alarms, Arrays.asList(new AlarmDefinition[] {
            PhoneContext.ALARM_PHONE_ADDED, PhoneContext.ALARM_PHONE_DELETED, PhoneContext.ALARM_PHONE_CHANGED
        })));
    }
}
