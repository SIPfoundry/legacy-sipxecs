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
import java.util.Map;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Server;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

public class AcdServerTestDb extends TestCase {

    private AcdServer m_server;

    protected void setUp() throws Exception {
        m_server = (AcdServer) TestHelper.getApplicationContext().getBean("acdServer");
        
        SipxPresenceService presenceService = org.easymock.classextension.EasyMock.createMock(SipxPresenceService.class);
        presenceService.getPresenceServerUri();
        org.easymock.classextension.EasyMock.expectLastCall().andReturn("sip:presence.com:5130").atLeastOnce();
        presenceService.getPresenceServiceUri();
        org.easymock.classextension.EasyMock.expectLastCall().andReturn("sip:presence.com:8111/RPC2").atLeastOnce();
        org.easymock.classextension.EasyMock.replay(presenceService);
        
        SipxServiceManager sipxServiceManager = EasyMock.createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxPresenceService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(presenceService).atLeastOnce();
        EasyMock.replay(sipxServiceManager);
        
        m_server.setSipxServiceManager(sipxServiceManager);
    }

    public void testDeploy() {
        IMocksControl sipxServerCtrl = EasyMock.createNiceControl();
        Server sipxServer = sipxServerCtrl.createMock(Server.class);
        sipxServerCtrl.replay();

        Map results = new Hashtable();
        results.put("resultCode", Provisioning.SUCCESS);

        IMocksControl mc = EasyMock.createNiceControl();
        Provisioning provisioning = mc.createMock(Provisioning.class);
        
        EasyMock.anyObject();
        provisioning.delete(null);
        mc.andReturn(results).anyTimes();
        
        EasyMock.anyObject();
        provisioning.create(null);
        mc.andReturn(results).anyTimes();
        
        EasyMock.anyObject();
        provisioning.set(null);
        mc.andReturn(results).anyTimes();
        
        EasyMock.anyObject();
        provisioning.get(null);
        mc.andReturn(results).anyTimes();
        
        mc.replay();

        XmlRpcSettings xmlRpc = new XmlRpcSettings(provisioning);

        m_server.deploy(xmlRpc);

        mc.verify();
        sipxServerCtrl.verify();
    }

}
