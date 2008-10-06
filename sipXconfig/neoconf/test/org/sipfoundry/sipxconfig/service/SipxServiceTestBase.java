/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.service;

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;

public class SipxServiceTestBase extends TestCase {
    protected void initCommonAttributes(SipxService service) {
        service.setIpAddress("192.168.1.1");
        service.setHostname("sipx");
        service.setFullHostname("sipx.example.org");
        service.setDomainName("example.org");
        service.setRealm("realm.example.org");
        service.setSipPort("5060");
        service.setLogDir("/var/log/sipxpbx");
        service.setConfDir("/etc/sipxpbx");
        service.setVoicemailHttpsPort("443");
        service.setModelFilesContext(TestHelper.getModelFilesContext());
    }

    public void assertCorrectFileGeneration(SipxServiceConfiguration configuration,
            String expectedFileName) throws Exception {
        StringWriter actualConfigWriter = new StringWriter();
        configuration.write(actualConfigWriter, null);

        Reader referenceConfigReader = new InputStreamReader(configuration.getClass()
                .getResourceAsStream(expectedFileName));
        String referenceConfig = IOUtils.toString(referenceConfigReader);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);
    }
}
