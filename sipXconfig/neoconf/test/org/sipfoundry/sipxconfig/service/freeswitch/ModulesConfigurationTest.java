/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.service.freeswitch;

import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.service.SipxFreeswitchService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxServiceTestBase;

public class ModulesConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        write(false);
    }

    public void testWriteG729() throws Exception {
        write(true);
    }

    public void write(boolean codecG729) throws Exception {
        SipxFreeswitchService service = new SipxFreeswitchService();
        service.setModelDir("freeswitch");
        service.setModelName("freeswitch.xml");
        initCommonAttributes(service);

        //set/unset G729 codec
        SipxFreeswitchService.setCodecG729(codecG729);
        service.initialize();

        SipxServiceManager manager = EasyMock.createMock(SipxServiceManager.class);
        manager.getServiceByBeanId(SipxFreeswitchService.BEAN_ID);
        EasyMock.expectLastCall().andReturn(service).anyTimes();
        EasyMock.replay(manager);

        ModulesConfiguration configuration = new ModulesConfiguration();
        configuration.setSipxServiceManager(manager);
        configuration.setTemplate("freeswitch/modules.conf.xml.vm");

        if (codecG729) {
            assertCorrectFileGeneration(configuration, "modules_g729.conf.test.xml");
        } else {
            assertCorrectFileGeneration(configuration, "modules.conf.test.xml");
        }
    }

}
