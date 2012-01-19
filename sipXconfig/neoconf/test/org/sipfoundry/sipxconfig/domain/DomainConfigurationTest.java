/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.commserver.LocationsManager;

public class DomainConfigurationTest extends TestCase {
    private DomainConfiguration m_out;

    @Override
    public void setUp() throws Exception {
        Location location = new Location();
        location.setAddress("10.10.10.1");
        location.setFqdn("host1.example.com");

        LocationsManager locationsManager = createNiceMock(LocationsManager.class);
        locationsManager.getLocations();
        expectLastCall().andReturn(new Location[] {
            location
        }).anyTimes();
        replay(locationsManager);

        m_out = new DomainConfiguration();
        m_out.setLocationsManager(locationsManager);
    }

    public void testWriteWithAliases() throws Exception {
        Domain domain = new Domain();
        domain.setName("domain.example.com");
        domain.setSipRealm("realm.example.com");
        domain.addAlias("alias.example.com");
        domain.setSharedSecret("mySecret");

        StringWriter actual = new StringWriter();
        m_out.writeDomainConfigPart(actual, domain, "master.example.com");

        Reader actualConfigReader = new StringReader(actual.toString());

        String actualConfig = IOUtils.toString(actualConfigReader);

        Reader referenceConfigReader = new InputStreamReader(DomainConfigurationTest.class
                .getResourceAsStream("expected-domain-config"));
        String referenceConfig = IOUtils.toString(referenceConfigReader);
        assertEquals(referenceConfig, actualConfig);
    }

    public void testWriteNoAliases() throws Exception {
        Domain domain = new Domain();
        domain.setName("domain.example.com");
        domain.setSipRealm("realm.example.com");
        domain.setSharedSecret("mySecret");

        StringWriter actual = new StringWriter();
        m_out.writeDomainConfigPart(actual, domain, "master.example.com");

        Reader actualConfigReader = new StringReader(actual.toString());

        String actualConfig = IOUtils.toString(actualConfigReader);

        Reader referenceConfigReader = new InputStreamReader(DomainConfigurationTest.class
                .getResourceAsStream("expected-no-aliases-domain-config"));
        String referenceConfig = IOUtils.toString(referenceConfigReader);
        assertEquals(referenceConfig, actualConfig);
    }

}
