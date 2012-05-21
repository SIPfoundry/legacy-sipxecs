/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig;

import java.util.Map;

import org.sipfoundry.sipxconfig.address.AddressProvider;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;

/**
 * Explicitly exercises the spring configuration
 */
public class SpringConfigurationTestIntegration extends IntegrationTestCase {
    public void testConfiguration() {
        assertTrue(true);
    }
    
    public void testBeansOfType() {
        Map<String, AddressProvider> beans = getApplicationContext().getBeansOfType(AddressProvider.class);
        for (AddressProvider a : beans.values()) {
            System.out.println(a.toString());            
        }
    }
}
