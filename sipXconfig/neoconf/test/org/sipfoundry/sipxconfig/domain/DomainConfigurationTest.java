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

public class DomainConfigurationTest extends TestCase {

    private Domain m_domain;
    private String m_language;
    private String m_realm;
    private String m_alarmServerUrl;
    private DomainConfiguration m_out;
    private String m_referenceConfig;

    public void setUp() throws Exception {
        m_domain = new Domain();
        m_domain.setName("domain.example.com");
        m_domain.setSharedSecret("mySecret");
        m_language = "en";
        m_realm = "realm.example.com";
        m_alarmServerUrl = "https://domain.example.com:8092";

        m_out = new DomainConfiguration();
        m_out.setTemplate("commserver/domain-config.vm");
        m_out.setVelocityEngine(TestHelper.getVelocityEngine());

        Reader referenceConfigReader = new InputStreamReader(DomainConfigurationTest.class
                .getResourceAsStream("expected-domain-config"));
        m_referenceConfig = IOUtils.toString(referenceConfigReader);
    }

    public void testWrite() throws Exception {
        StringWriter actualConfigWriter = new StringWriter();
        m_out.generate(m_domain, m_realm, m_language, m_alarmServerUrl);
        m_out.write(actualConfigWriter, null);

        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());

        String actualConfig = IOUtils.toString(actualConfigReader);

        assertEquals(m_referenceConfig, actualConfig);
    }
}
