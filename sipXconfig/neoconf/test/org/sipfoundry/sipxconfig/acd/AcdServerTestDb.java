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
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.service.SipxPresenceService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;

import static org.easymock.EasyMock.anyObject;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createMock;
import static org.easymock.classextension.EasyMock.createNiceControl;
import static org.easymock.classextension.EasyMock.replay;

public class AcdServerTestDb extends TestCase {

    private AcdServer m_server;
    private CoreContext m_coreContext;

    @Override
    protected void setUp() throws Exception {
        m_server = (AcdServer) TestHelper.getApplicationContext().getBean("acdServer");

        m_coreContext = createMock(CoreContext.class);
        m_coreContext.getDomainName();
        expectLastCall().andReturn("presence.com").atLeastOnce();
        m_server.setCoreContext(m_coreContext);

        SipxPresenceService presenceService = createMock(SipxPresenceService.class);
        presenceService.getPresenceServerPort();
        expectLastCall().andReturn(5130).atLeastOnce();
        presenceService.getPresenceApiPort();
        expectLastCall().andReturn(8111).atLeastOnce();
        replay(m_coreContext, presenceService);

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxPresenceService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(presenceService).atLeastOnce();
        EasyMock.replay(sipxServiceManager);

        m_server.setSipxServiceManager(sipxServiceManager);
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
