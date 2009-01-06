/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.util.Set;

import junit.framework.TestCase;

public class ConferenceBridgeBuilderTest extends TestCase {
    private ConferenceBridgeBuilder m_builder;
    private org.sipfoundry.sipxconfig.conference.Bridge m_myBridge;
    private ConferenceBridge m_apiBridge;
    
    protected void setUp() {
        m_builder = new ConferenceBridgeBuilder();
        m_myBridge = new org.sipfoundry.sipxconfig.conference.Bridge();
        m_apiBridge = new ConferenceBridge();
    }

    public void testFromApi() {
        Conference apiConf = new Conference();
        apiConf.setDescription("conference in botswana");
        apiConf.setEnabled(new Boolean(true));
        apiConf.setExtension("1234");
        apiConf.setName("conf0");
        Conference[] apiConfs = new Conference[1];
        apiConfs[0] = apiConf;
        m_apiBridge.setConferences(apiConfs);
        m_apiBridge.setDescription("bridge to nowhere");
        m_apiBridge.setEnabled(new Boolean(true));
        m_apiBridge.setHost("bridge host");
        m_apiBridge.setName("bridge name");
        m_apiBridge.setPort(new Integer(4321));
        ApiBeanUtil.toMyObject(m_builder, m_myBridge, m_apiBridge);
        assertEquals("bridge to nowhere", m_myBridge.getDescription());
        assertEquals(true, m_myBridge.isEnabled());
        assertEquals("bridge host", m_myBridge.getHost());
        assertEquals("bridge name", m_myBridge.getName());
        assertEquals(4321, m_myBridge.getPort());
        Set conferences = m_myBridge.getConferences();
        assertEquals(1, conferences.size());
        org.sipfoundry.sipxconfig.conference.Conference myConf =
            (org.sipfoundry.sipxconfig.conference.Conference) conferences.iterator().next();
        assertEquals("conference in botswana", myConf.getDescription());
        assertEquals(true, myConf.isEnabled());
        assertEquals("1234", myConf.getExtension());
        assertEquals("conf0", myConf.getName());
    }

    public void testToApi() {
        org.sipfoundry.sipxconfig.conference.Conference myConf =
            new org.sipfoundry.sipxconfig.conference.Conference();
        myConf.setDescription("conference in botswana");
        myConf.setEnabled(true);
        myConf.setExtension("1234");
        myConf.setName("conf0");
        m_myBridge.addConference(myConf);
//        m_myBridge.setDescription("bridge to nowhere");
        m_myBridge.setEnabled(true);
//        m_myBridge.setHost("bridge host");
//        m_myBridge.setName("bridge name");
//        m_myBridge.setPort(4321);
        ApiBeanUtil.toApiObject(m_builder, m_apiBridge, m_myBridge);
        assertEquals("bridge to nowhere", m_apiBridge.getDescription());
        assertEquals(true, m_apiBridge.getEnabled().booleanValue());
        assertEquals("bridge host", m_apiBridge.getHost());
        assertEquals("bridge name", m_apiBridge.getName());
        assertEquals(4321, m_apiBridge.getPort().intValue());
        Conference[] conferences = m_apiBridge.getConferences();
        assertEquals(1, conferences.length);
        Conference apiConf = conferences[0];
        assertEquals("conference in botswana", apiConf.getDescription());
        assertEquals(true, apiConf.getEnabled().booleanValue());
        assertEquals("1234", apiConf.getExtension());
        assertEquals("conf0", apiConf.getName());
    }
}
