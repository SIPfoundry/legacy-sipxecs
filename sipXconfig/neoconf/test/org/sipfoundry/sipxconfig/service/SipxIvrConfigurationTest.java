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

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;

import static org.easymock.EasyMock.*;

public class SipxIvrConfigurationTest extends SipxServiceTestBase {

    public void testWrite() throws Exception {
        DialPlanContext dialPlanContext = createMock(DialPlanContext.class);
        dialPlanContext.getVoiceMail();
        expectLastCall().andReturn("101");

        replay(dialPlanContext);

        SipxIvrConfiguration out = new SipxIvrConfiguration();
        out.setVelocityEngine(TestHelper.getVelocityEngine());
        out.setTemplate("sipxivr/sipxivr.properties.vm");

        SipxIvrService ivrService = new SipxIvrService();
        ivrService.setModelDir("sipxivr");
        ivrService.setModelName("sipxivr.xml");
        ivrService.setDialPlanContext(dialPlanContext);
        initCommonAttributes(ivrService);

        ivrService.setMailstoreDir("/var/sipxdata/mediaserver/data/mailstore");
        ivrService.setScriptsDir("/usr/share/www/doc/aa_vxml");
        ivrService.setVxmlDir("/var/sipxdata/mediaserver/data");

        out.generate(ivrService);

        StringWriter actualConfigWriter = new StringWriter();
        out.write(actualConfigWriter, null);

        Reader referenceConfigReader = new InputStreamReader(getClass().getResourceAsStream(
                "expected-sipxivr.properties"));
        String referenceConfig = IOUtils.toString(referenceConfigReader);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);

        verify(dialPlanContext);
    }
}
