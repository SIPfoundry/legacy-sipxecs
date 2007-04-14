package org.sipfoundry.sipxconfig.phone;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.admin.intercom.IntercomManager;

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
}
