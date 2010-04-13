/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.site.common;

import java.io.File;
import java.net.URL;
import java.util.Locale;
import java.util.Properties;

import junit.framework.TestCase;

import org.apache.hivemind.Messages;
import org.apache.hivemind.util.FileResource;
import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.services.ComponentMessagesSource;
import org.apache.tapestry.services.impl.ComponentMessages;
import org.apache.tapestry.spec.ComponentSpecification;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.site.admin.configdiag.ConfigurationDiagnosticPage;

public class JarMessagesSourceServiceAdapterTest extends TestCase {


    private JarMessagesSourceServiceAdapter m_out;
    private JarMessagesSource m_jarMessagesSource;
    private DefaultJarMessagesSourceContext m_context;
    private IComponent m_component;
    private IPage m_mockPage;

    protected void setUp() throws Exception {
        m_out = new JarMessagesSourceServiceAdapter();

        m_context = new DefaultJarMessagesSourceContext();
        URL resource = getClass().getClassLoader().getResource("org/sipfoundry/sipxconfig/site/common/JarMessagesSourceTest.class");
        String path = new File(resource.getFile()).getParent();
        m_context.setLocalizationPackageRoot(path);

        m_jarMessagesSource = new JarMessagesSource();
        m_jarMessagesSource.setContext(m_context);

        m_out.setJarMessagesSource(m_jarMessagesSource);

        m_out.setSystemMessagesSource(new ComponentMessagesSource() {
            public Messages getMessages(IComponent component) {
                Properties messageProps = new Properties();
                messageProps.put("unsupported.message.id", "Unsupported message");
                return new ComponentMessages(Locale.ENGLISH, messageProps);
            }
        });

        Creator creator = new Creator();
        m_component = (ConfigurationDiagnosticPage)creator.newInstance(ConfigurationDiagnosticPage.class);
        m_mockPage = EasyMock.createNiceMock(IPage.class);
        m_component.setPage(m_mockPage);

        ComponentSpecification specification = new ComponentSpecification();
        specification.setSpecificationLocation(new FileResource("context:/WEB-INF/admin/configdiag/ConfigurationDiagnosticPage.page"));
        PropertyUtils.write(m_component, "specification", specification);
    }

    public void testGetMessagesForSupportedLanguageWithoutRegion() {
        m_mockPage.getLocale();
        EasyMock.expectLastCall().andReturn(Locale.FRENCH).anyTimes();
        EasyMock.replay(m_mockPage);

        Messages messages = m_out.getMessages(m_component);
        assertEquals("My French Title", messages.getMessage("title"));
    }

    public void testGetMessagesForSupportedLanguageWithRegion() {
        m_mockPage.getLocale();
        EasyMock.expectLastCall().andReturn(Locale.CANADA_FRENCH).anyTimes();
        EasyMock.replay(m_mockPage);

        Messages messages = m_out.getMessages(m_component);
        assertEquals("My Canadian French Title", messages.getMessage("title"));
    }

    public void testGetMessageForUnsupportedMessageId() {
        m_mockPage.getLocale();
        EasyMock.expectLastCall().andReturn(Locale.FRENCH).anyTimes();
        EasyMock.replay(m_mockPage);

        Messages messages = m_out.getMessages(m_component);
        assertEquals("Unsupported message", messages.getMessage("unsupported.message.id"));
    }

    public void testGetMessagesForUnsupportedLanguage() {
        m_out.setSystemMessagesSource(new ComponentMessagesSource() {
            public Messages getMessages(IComponent component) {
                Properties messageProps = new Properties();
                messageProps.put("title", "System Title");
                return new ComponentMessages(Locale.PRC, messageProps);
            }
        });

        EasyMock.reset(m_mockPage);
        m_mockPage.getLocale();
        EasyMock.expectLastCall().andReturn(Locale.PRC).anyTimes();
        EasyMock.replay(m_mockPage);

        Messages messages = m_out.getMessages(m_component);
        assertEquals("System Title", messages.getMessage("title"));
    }
}

