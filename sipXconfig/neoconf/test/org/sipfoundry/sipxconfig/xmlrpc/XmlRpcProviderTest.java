/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.xmlrpc;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.commserver.ProcessManagerApi;

public class XmlRpcProviderTest extends TestCase {

    public void testGetApi() throws Exception {
        XmlRpcApiProvider m_provider = new XmlRpcApiProvider();
        m_provider.setServiceInterface(ProcessManagerApi.class);
        m_provider.setSecure(true);
        m_provider.afterPropertiesSet();
        assertNotNull(m_provider.getApi("https://host.example.org:4321"));
    }

    public void testInvalidConfig() throws Exception {
        XmlRpcApiProvider m_provider = new XmlRpcApiProvider();
        m_provider.setServiceInterface(ProcessManagerApi.class);
        m_provider.setSecure(true);
        m_provider.setMarshaller(new XmlRpcClientInterceptor.DefaultMarshaller(null));
        m_provider.setMethodNamePrefix("prefix");

        try {
            m_provider.afterPropertiesSet();
            fail("Should detect invalid configuration");
        } catch (IllegalArgumentException e) {
            // ok
        }
    }
}
