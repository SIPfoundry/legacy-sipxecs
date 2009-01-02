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

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import static org.easymock.EasyMock.*;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;

public class SipxServiceTestBase extends TestCase {
    protected void initCommonAttributes(SipxService service) {
        Domain domain = createDefaultDomain();
        DomainManager domainManager = createMock(DomainManager.class);
        domainManager.getAuthorizationRealm();
        expectLastCall().andReturn("realm.example.org").anyTimes();
        domainManager.getDomain();
        expectLastCall().andReturn(domain).anyTimes();
        replay(domainManager);

        service.setDomainManager(domainManager);
        service.setSipPort("5060");
        service.setLogDir("/var/log/sipxpbx");
        service.setConfDir("/etc/sipxpbx");
        service.setModelFilesContext(TestHelper.getModelFilesContext());
    }

    protected Location createDefaultLocation() {
        Location location = new Location();
        location.setName("localLocation");
        location.setFqdn("sipx.example.org");
        location.setAddress("192.168.1.1");
        return location;
    }

    protected Domain createDefaultDomain() {
        Domain domain = new Domain();
        domain.setName("example.org");
        return domain;
    }

    public void assertCorrectFileGeneration(SipxServiceConfiguration configuration,
            String expectedFileName) throws Exception {
        configuration.setVelocityEngine(TestHelper.getVelocityEngine());

        StringWriter actualConfigWriter = new StringWriter();
        configuration.write(actualConfigWriter, createDefaultLocation());

        InputStream resourceAsStream = configuration.getClass().getResourceAsStream(expectedFileName);
        assertNotNull(resourceAsStream);

        Reader referenceConfigReader = new InputStreamReader(resourceAsStream);
        String referenceConfig = IOUtils.toString(referenceConfigReader);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(referenceConfig, actualConfig);
    }
}
