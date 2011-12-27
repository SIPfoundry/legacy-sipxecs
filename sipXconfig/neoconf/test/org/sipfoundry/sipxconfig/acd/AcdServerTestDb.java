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

import static org.easymock.EasyMock.anyObject;
import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.createNiceControl;

import java.util.Hashtable;
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.presence.PresenceServer;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class AcdServerTestDb extends TestCase {

    private AcdServer m_server;
    private CoreContext m_coreContext;

    @Override
    protected void setUp() throws Exception {
        m_server = (AcdServer) TestHelper.getApplicationContext().getBean("acdServer");

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.getDomainName();
        expectLastCall().andReturn("presence.com").atLeastOnce();
        replay(m_coreContext);
        
        m_server.setCoreContext(m_coreContext);

        AddressManager addressManager = createMock(AddressManager.class);
        addressManager.getSingleAddress(PresenceServer.HTTP_ADDRESS);
        expectLastCall().andReturn(new Address("presence-api.example.org", 100)).anyTimes();
        addressManager.getSingleAddress(PresenceServer.SIP_TCP_ADDRESS);
        expectLastCall().andReturn(new Address("presence-sip.example.org", 101)).anyTimes();
        replay(addressManager);
        m_server.setAddressManager(addressManager);    
    }

    public void testDeploy() {
        IMocksControl sipxServerCtrl = createNiceControl();
        sipxServerCtrl.replay();

        Map results = new Hashtable();
        results.put("resultCode", Provisioning.SUCCESS);

        IMocksControl mc = createNiceControl();
        Provisioning provisioning = mc.createMock(Provisioning.class);

        anyObject();
        provisioning.delete(null);
        mc.andReturn(results).anyTimes();

        anyObject();
        provisioning.create(null);
        mc.andReturn(results).anyTimes();

        anyObject();
        provisioning.set(null);
        mc.andReturn(results).anyTimes();

        anyObject();
        provisioning.get(null);
        mc.andReturn(results).anyTimes();

        mc.replay();

        XmlRpcSettings xmlRpc = new XmlRpcSettings(provisioning);

        m_server.deploy(xmlRpc);

        mc.verify();
        sipxServerCtrl.verify();
    }

}
