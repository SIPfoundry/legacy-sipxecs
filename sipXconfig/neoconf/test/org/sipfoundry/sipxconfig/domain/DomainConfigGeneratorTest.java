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

public class DomainConfigGeneratorTest extends TestCase {
    
    private Domain m_domain;
    private DomainConfigGenerator m_out;
    
    public void setUp() {
        m_domain = new Domain();
        m_domain.setName("domain.example.com");
        m_domain.setSharedSecret("mySecret");
        
        m_out = new DomainConfigGenerator();
        m_out.setVelocityEngine(TestHelper.getVelocityEngine());
    }
    
    public void testGenerateDomainConfigWithWriter() throws Exception {
        Reader referenceConfigReader = new InputStreamReader(
                DomainConfigGeneratorTest.class.getResourceAsStream("expected-domain-config"));
        
        
        StringWriter actualConfigWriter = new StringWriter();
        m_out.generate(m_domain, actualConfigWriter);
        
        Reader actualConfigReader = new StringReader(actualConfigWriter.toString());
        
        String referenceConfig = IOUtils.toString(referenceConfigReader);
        String actualConfig = IOUtils.toString(actualConfigReader);
        
        assertEquals(referenceConfig, actualConfig);
    }
}
