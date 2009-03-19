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

import java.io.InputStreamReader;
import java.io.Reader;
import java.io.StringReader;
import java.io.StringWriter;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class DomainConfigurationTest extends TestCase {
    private String m_language;
    private DomainConfiguration m_out;

    @Override
    public void setUp() throws Exception {
        m_language = "en";

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
        m_out.setTemplate("commserver/domain-config.vm");
        m_out.setVelocityEngine(TestHelper.getVelocityEngine());
        m_out.setLocationsManager(locationsManager);

    }

    public void testWriteWithAliases() throws Exception {
        Domain m_domain = new Domain();
        m_domain.setName("domain.example.com");
        m_domain.setSipRealm("realm.example.com");
        m_domain.addAlias("alias.example.com");
        m_domain.setSharedSecret("mySecret");

        StringWriter actualConfigWriter = new StringWriter();
        m_out.generate(m_domain, "master.example.com", m_language);
        m_out.write(actualConfigWriter, null);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());

        String actualConfig = IOUtils.toString(actualConfigReader);

        Reader referenceConfigReader = new InputStreamReader(DomainConfigurationTest.class
                .getResourceAsStream("expected-domain-config"));
        String referenceConfig = IOUtils.toString(referenceConfigReader);
        assertEquals(referenceConfig, actualConfig);
    }

    public void testWriteNoAliases() throws Exception {
        Domain m_domain = new Domain();
        m_domain.setName("domain.example.com");
        m_domain.setSipRealm("realm.example.com");
        m_domain.setSharedSecret("mySecret");

        StringWriter actualConfigWriter = new StringWriter();
        m_out.generate(m_domain, "master.example.com", m_language);
        m_out.write(actualConfigWriter, null);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());

        String actualConfig = IOUtils.toString(actualConfigReader);

        Reader referenceConfigReader = new InputStreamReader(DomainConfigurationTest.class
                .getResourceAsStream("expected-no-aliases-domain-config"));
        String referenceConfig = IOUtils.toString(referenceConfigReader);
        assertEquals(referenceConfig, actualConfig);
    }

}
