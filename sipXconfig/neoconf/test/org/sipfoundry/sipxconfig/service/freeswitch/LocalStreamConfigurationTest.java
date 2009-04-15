/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service.freeswitch;

import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxServiceTestBase;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class LocalStreamConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        SipxFreeswitchService service = new SipxFreeswitchService();
        service.setModelDir("freeswitch");
        service.setModelName("freeswitch.xml");
        initCommonAttributes(service);
        
        service.setDocDir("/usr/share/www/doc");
        
        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxFreeswitchService.BEAN_ID);
        expectLastCall().andReturn(service);

        replay(sipxServiceManager);

        LocalStreamConfiguration configuration = new LocalStreamConfiguration();
        configuration.setSipxServiceManager(sipxServiceManager);
        configuration.setTemplate("freeswitch/local_stream.conf.xml.vm");

        assertCorrectFileGeneration(configuration, "local_stream.conf.test.xml");

        verify(sipxServiceManager);
    }
}
