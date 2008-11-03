/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.service;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class SipxIvrConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        DialPlanContext dialPlanContext = createMock(DialPlanContext.class);
        dialPlanContext.getVoiceMail();
        expectLastCall().andReturn("101");

        SipxIvrService ivrService = new SipxIvrService();
        ivrService.setModelDir("sipxivr");
        ivrService.setModelName("sipxivr.xml");
        ivrService.setDialPlanContext(dialPlanContext);
        initCommonAttributes(ivrService);

        ivrService.setMailstoreDir("/var/sipxdata/mediaserver/data/mailstore");
        ivrService.setScriptsDir("/usr/share/www/doc/aa_vxml");
        ivrService.setDocDir("/usr/share/www/doc");
        ivrService.setVxmlDir("/var/sipxdata/mediaserver/data");

        SipxServiceManager sipxServiceManager = createMock(SipxServiceManager.class);
        sipxServiceManager.getServiceByBeanId(SipxIvrService.BEAN_ID);
        expectLastCall().andReturn(ivrService).atLeastOnce();
        replay(sipxServiceManager, dialPlanContext);

        SipxIvrConfiguration out = new SipxIvrConfiguration();
        out.setSipxServiceManager(sipxServiceManager);
        out.setTemplate("sipxivr/sipxivr.properties.vm");

        assertCorrectFileGeneration(out, "expected-sipxivr.properties");

        verify(sipxServiceManager, dialPlanContext);
    }
}
