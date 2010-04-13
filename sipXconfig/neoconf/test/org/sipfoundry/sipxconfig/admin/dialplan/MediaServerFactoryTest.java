/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan;

import java.util.Arrays;
import java.util.Collection;

import junit.framework.TestCase;
import org.springframework.beans.factory.BeanFactory;

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

public class MediaServerFactoryTest extends TestCase {

    private static final String SIPX_MEDIA_SERVER = "freeswitchMediaServer";
    private static final String EXCHANGE_MEDIA_SERVER = "exchangeUmMediaServer";

    private MediaServerFactory m_out;
    private BeanFactory m_mockBeanFactory;

    /**
     * Creates the object under test and configures it with a mock BeanFactory object. The mock
     * BeanFactory needs to be configured in each individual test method
     */
    @Override
    public void setUp() {
        m_out = new MediaServerFactory();
        m_mockBeanFactory = createNiceMock(BeanFactory.class);
        m_out.setBeanFactory(m_mockBeanFactory);
    }

    public void testCreate() {
        m_mockBeanFactory.getBean(SIPX_MEDIA_SERVER, MediaServer.class);
        expectLastCall().andReturn(new FreeswitchMediaServer());
        m_mockBeanFactory.getBean(EXCHANGE_MEDIA_SERVER, MediaServer.class);
        expectLastCall().andReturn(new ExchangeMediaServer());
        replay(m_mockBeanFactory);

        MediaServer sipXMediaServer = m_out.create(SIPX_MEDIA_SERVER);
        assertNotNull("Null returned for " + SIPX_MEDIA_SERVER, sipXMediaServer);
        assertTrue("Wrong type for " + SIPX_MEDIA_SERVER, sipXMediaServer instanceof FreeswitchMediaServer);

        MediaServer exchangeMediaServer = m_out.create(EXCHANGE_MEDIA_SERVER);
        assertNotNull("Null returned for " + EXCHANGE_MEDIA_SERVER, exchangeMediaServer);
        assertTrue("Wrong type for " + EXCHANGE_MEDIA_SERVER, exchangeMediaServer instanceof ExchangeMediaServer);
    }

    public void testGetBeanIds() {
        Collection<String> idList = Arrays.asList(new String[] {
            SIPX_MEDIA_SERVER, EXCHANGE_MEDIA_SERVER
        });
        m_out.setBeanIds(idList);

        Collection<String> beanListFromFactory = m_out.getBeanIds();
        assertTrue(SIPX_MEDIA_SERVER + " not in list.", beanListFromFactory.contains(SIPX_MEDIA_SERVER));
        assertTrue(EXCHANGE_MEDIA_SERVER + " not in list.", beanListFromFactory.contains(EXCHANGE_MEDIA_SERVER));
    }
}
