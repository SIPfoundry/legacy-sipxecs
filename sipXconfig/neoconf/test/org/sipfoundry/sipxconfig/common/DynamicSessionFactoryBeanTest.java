/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.io.StringReader;

import junit.framework.TestCase;

import org.dom4j.Document;
import org.dom4j.io.SAXReader;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.hibernate.cfg.Configuration;
import org.hibernate.util.DTDEntityResolver;
import org.sipfoundry.sipxconfig.gateway.Gateway;
import org.sipfoundry.sipxconfig.gateway.acme.AcmeGateway;
import org.springframework.beans.factory.ListableBeanFactory;

public class DynamicSessionFactoryBeanTest extends TestCase {

    public void testXmlMapping() throws Exception {
        DynamicSessionFactoryBean factory = new DynamicSessionFactoryBean();
        String mapping = factory.xmlMapping(Gateway.class, AcmeGateway.class, "gwAcme");
        validateXml(mapping);
    }

    public void testBindSubclasses() throws Exception {
        IMocksControl controlFactory = EasyMock.createControl();
        ListableBeanFactory factory = controlFactory.createMock(ListableBeanFactory.class);
        factory.getBeanNamesForType(Gateway.class);
        controlFactory.andReturn(new String[] {"gwGeneric", "gwAcme"});
        factory.getType("gwGeneric");
        controlFactory.andReturn(Gateway.class);
        factory.getType("gwAcme");
        controlFactory.andReturn(AcmeGateway.class);
        controlFactory.replay();

        ConfigurationMock config = new ConfigurationMock();

        DynamicSessionFactoryBean bean = new DynamicSessionFactoryBean();
        bean.setBeanFactory(factory);
        bean.bindSubclasses(config, Gateway.class);

        controlFactory.verify();
        config.verify();
    }


    public void testBindSubclassesOfBean() throws Exception {
        IMocksControl controlFactory = EasyMock.createControl();
        ListableBeanFactory factory = controlFactory.createMock(ListableBeanFactory.class);
        factory.getBeanNamesForType(Gateway.class);
        controlFactory.andReturn(new String[] {"gwGeneric", "gwAcme"});
        factory.getType("gwGeneric");
        controlFactory.andReturn(Gateway.class);
        factory.getType("gwAcme");
        controlFactory.andReturn(AcmeGateway.class);
        controlFactory.replay();

        ConfigurationMock config = new ConfigurationMock();

        DynamicSessionFactoryBean bean = new DynamicSessionFactoryBean();
        bean.setBeanFactory(factory);
        bean.bindSubclasses(config, Gateway.class);

        controlFactory.verify();
        config.verify();
    }


    private static void validateXml(String xml) throws Exception {
        SAXReader xmlReader = new SAXReader();
        xmlReader.setEntityResolver(new DTDEntityResolver());
        xmlReader.setValidation(true);

        Document document = xmlReader.read(new StringReader(xml));

        assertEquals(Gateway.class.getName(), document.valueOf("/hibernate-mapping/subclass/@extends"));
        assertEquals(AcmeGateway.class.getName(),
                document.valueOf("/hibernate-mapping/subclass/@name"));
        assertEquals("gwAcme",
                document.valueOf("/hibernate-mapping/subclass/@discriminator-value"));
    }

    /**
     * Unfortunately class easy mock does not work for configuration
     */
    private static class ConfigurationMock extends Configuration {
        boolean m_valid;

        public void verify() throws Exception {
            assertTrue(m_valid);
        }

        public Configuration addXML(String xml) {
            try {
                validateXml(xml);
            } catch (RuntimeException e) {
                throw e;
            }
            catch (Exception e) {
                throw new RuntimeException(e);
            }
            m_valid = true;
            return this;
        }
    }
}
