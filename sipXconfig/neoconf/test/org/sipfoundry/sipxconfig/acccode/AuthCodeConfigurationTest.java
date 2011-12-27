/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.acccode;


import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.junit.Assert.assertEquals;

import java.io.IOException;
import java.io.StringWriter;
import java.util.Collections;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.common.InternalUser;
import org.sipfoundry.sipxconfig.dialplan.config.XmlFile;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class AuthCodeConfigurationTest {
    AuthCodesConfig m_config;
    AuthCodeSettings m_settings;
    Domain m_domain;
    
    @Before
    public void setUp() {
        m_config = new AuthCodesConfig();
        m_settings = new AuthCodeSettings();
        m_settings.setModelFilesContext(TestHelper.getModelFilesContext());
        m_domain = new Domain("example.org");
        m_domain.setSipRealm("grapefruit");        
    }    

    @Test
    public void testProperties() throws Exception {
        StringWriter actual = new StringWriter();
        m_config.writeConfig(actual, m_settings, m_domain, 1000);
        assertEquals(expected("expected-sipxacccode.properties"), actual.toString());
    }
    
    @Test
    public void testXml() throws Exception {
        AuthCode code = new AuthCode();
        code.setCode("444");
        InternalUser user = new InternalUser();
        user.setSipPassword("xxx");
        user.setName("~~ac~1");
        code.setInternalUser(user);
        AuthCodeManager authCodesManager = createMock(AuthCodeManager.class);
        authCodesManager.getAuthCodes();
        expectLastCall().andReturn(Collections.singletonList(code)).once();
        replay(authCodesManager);
        m_config.setAuthCodeManager(authCodesManager);
        
        StringWriter actual = new StringWriter();
        XmlFile x = new XmlFile(actual);
        x.write(m_config.getDocument());
        assertEquals(expected("expected-authcodes.xml"), actual.toString());
    }
    
    private String expected(String name) throws IOException {
        return IOUtils.toString(getClass().getResourceAsStream(name));
    }
}
