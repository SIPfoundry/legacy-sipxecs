/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.site.common;

import java.io.File;
import java.util.Locale;

import junit.framework.TestCase;

import org.apache.hivemind.Messages;
import org.apache.hivemind.util.FileResource;
import org.apache.hivemind.util.PropertyUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.spec.ComponentSpecification;
import org.apache.tapestry.test.Creator;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.components.AdminNavigation;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class JarMessagesSourceTest extends TestCase {

    private JarMessagesSource m_out;
    private BaseComponent m_component;
    private IPage m_mockPage;
    private DefaultJarMessagesSourceContext m_context;

    protected void setUp() throws Exception {
        m_out = new JarMessagesSource();

        m_context = new DefaultJarMessagesSourceContext();
        File srcFile = TestHelper.getResourceAsFile(getClass(), "sipxconfig_fr.jar");
        m_context.setLocalizationPackageRoot(srcFile.getParent());
        m_out.setContext(m_context);

        Creator creator = new Creator();
        m_component = (AdminNavigation)creator.newInstance(AdminNavigation.class);
        m_mockPage = EasyMock.createNiceMock(IPage.class);
        m_component.setPage(m_mockPage);

        ComponentSpecification specification = new ComponentSpecification();
        specification.setSpecificationLocation(new FileResource("context:/WEB-INF/admin/configdiag/ConfigurationDiagnosticPage.page"));
        PropertyUtils.write(m_component, "specification", specification);
    }

    public void testGetMessagesForSupportedLanguageWithoutRegion() {
        // configure mock
        m_mockPage.getLocale();
        EasyMock.expectLastCall().andReturn(Locale.FRENCH).anyTimes();
        EasyMock.replay(m_mockPage);

        Messages messages = m_out.getMessages(m_component);
        assertEquals("My French Title", messages.getMessage("title"));
    }

    public void testGetMessagesForSupportedLanguageWithRegion() {
        // configure mock
        m_mockPage.getLocale();
        EasyMock.expectLastCall().andReturn(Locale.CANADA_FRENCH).anyTimes();
        EasyMock.replay(m_mockPage);

        Messages messages = m_out.getMessages(m_component);
        assertEquals("My Canadian French Title", messages.getMessage("title"));
    }

    public void testResolveLocaleNameWithLocale() {
        // test default case for locales which have no label defined
        Locale frenchLocale = new Locale("pl");
        assertEqualsNoCase("Polski", m_out.resolveLocaleName(frenchLocale));

        // test for locales that have a label defined in their message source
        Locale canadianFrenchLocale = new Locale("fr", "CA");
        assertEqualsNoCase("french (canada)", m_out.resolveLocaleName(canadianFrenchLocale));
    }

    private void assertEqualsNoCase(String expected, String actual) {
	assertEquals(expected.toLowerCase(), actual.toLowerCase());
    }

    public void testResolveLocaleNameWithStrings() {
        // test default case for locales which have no label defined
        assertEqualsNoCase("polski", m_out.resolveLocaleName("pl"));

        // test for locales that have a label defined in their message source
        assertEqualsNoCase("french (canada)", m_out.resolveLocaleName("fr-CA"));

        // test for locales that have two labels  defined in their message source
        assertEqualsNoCase("abitibi (france,ca)", m_out.resolveLocaleName("abitibi-fr-ca"));
    }
}
