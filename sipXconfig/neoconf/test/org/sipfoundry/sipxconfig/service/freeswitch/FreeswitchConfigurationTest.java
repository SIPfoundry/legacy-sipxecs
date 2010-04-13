/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service.freeswitch;

import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxServiceTestBase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class FreeswitchConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxFreeswitchService service = new SipxFreeswitchService();
        service.setModelDir("freeswitch");
        service.setModelName("freeswitch.xml");
        initCommonAttributes(service);

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxFreeswitchService.BEAN_ID);
        expectLastCall().andReturn(service);

        DomainManager domainManager = createMock(DomainManager.class);
        domainManager.getDomain();
        expectLastCall().andReturn(createDefaultDomain());

        replay(sipxServiceManager, domainManager);

        FreeswitchConfiguration configuration = new FreeswitchConfiguration();
        configuration.setSipxServiceManager(sipxServiceManager);
        configuration.setTemplate("freeswitch/freeswitch.xml.vm");
        configuration.setDomainManager(domainManager);

        assertCorrectFileGeneration(configuration, "freeswitch.test.xml");

        verify(sipxServiceManager, domainManager);
    }
}
