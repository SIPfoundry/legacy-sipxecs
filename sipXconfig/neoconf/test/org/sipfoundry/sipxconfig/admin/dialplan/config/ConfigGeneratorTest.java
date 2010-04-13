/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.StringWriter;
import java.util.Collections;

import org.sipfoundry.sipxconfig.service.SipxService;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.service.SipxProxyService;
import org.sipfoundry.sipxconfig.service.SipxRegistrarService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.service.SipxStatusService;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.createStrictMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

/**
 * ConfigGeneratorTest
 */
public class ConfigGeneratorTest extends XMLTestCase {

    private DomainManager m_domainManager;
    private SipxServiceManager m_sipxServiceManager;

    public ConfigGeneratorTest() {
        XMLUnit.setIgnoreWhitespace(true);
    }

    @Override
    public void setUp() {
        m_domainManager = EasyMock.createMock(DomainManager.class);
        m_domainManager.getDomain();
        EasyMock.expectLastCall().andReturn(new Domain("example.org")).anyTimes();
        EasyMock.replay(m_domainManager);

        SipxProxyService proxyService = new SipxProxyService();
        proxyService.setModelDir("sipxproxy");
        proxyService.setModelName("sipxproxy.xml");
        proxyService.setModelFilesContext(TestHelper.getModelFilesContext());
        proxyService.setBeanName(SipxProxyService.BEAN_ID);
        proxyService.setSipPort("9901");
        proxyService.setDomainManager(m_domainManager);

        SipxService statusService = new SipxStatusService();
        statusService.setBeanName(SipxStatusService.BEAN_ID);
        statusService.setModelFilesContext(TestHelper.getModelFilesContext());
        statusService.setSettings(TestHelper.loadSettings("sipxstatus/sipxstatus.xml"));
        Setting statusConfigSettings = statusService.getSettings().getSetting("status-config");
        statusConfigSettings.getSetting("SIP_STATUS_SIP_PORT").setValue("9905");

        SipxRegistrarService registrarService = new SipxRegistrarService();
        registrarService.setBeanName(SipxRegistrarService.BEAN_ID);
        registrarService.setRegistrarEventSipPort("9906");
        registrarService.setSipPort("9907");

        m_sipxServiceManager = TestUtil.getMockSipxServiceManager(true, proxyService, registrarService, statusService);
    }

    public void testGetFileContent() throws Exception {
        DialPlanContext dialPlanContext = createStrictMock(DialPlanContext.class);
        DialingRuleProvider dialingRuleProvider = createStrictMock(DialingRuleProvider.class);
        dialingRuleProvider.getDialingRules();
        expectLastCall().andReturn(Collections.EMPTY_LIST);
        dialPlanContext.getGenerationRules();
        expectLastCall().andReturn(Collections.EMPTY_LIST);

        SbcManager sbcManager = createNiceMock(SbcManager.class);

        SbcDeviceManager sbcDeviceManager = createNiceMock(SbcDeviceManager.class);

        replay(dialingRuleProvider, dialPlanContext, sbcDeviceManager, sbcManager);

        ConfigGenerator generator = new ConfigGenerator();
        generator.setDialingRuleProvider(dialingRuleProvider);

        AuthRules authRules = new AuthRules();
        authRules.setName("authrules.xml");
        authRules.setDomainManager(m_domainManager);
        generator.setAuthRules(authRules);

        RulesXmlFile mappingRules = new MappingRules();
        mappingRules.setName("mappingrules.xml");
        mappingRules.setDomainManager(m_domainManager);
        generator.setMappingRules(mappingRules);

        FallbackRules fallbackRules = new FallbackRules();
        fallbackRules.setName("fallbackrules.xml");
        fallbackRules.setDomainManager(m_domainManager);
        generator.setFallbackRules(fallbackRules);

        ForwardingRules forwardingRules = new ForwardingRules();
        forwardingRules.setName("fallbackrules.xml");
        forwardingRules.setVelocityEngine(TestHelper.getVelocityEngine());
        forwardingRules.setSbcManager(sbcManager);
        forwardingRules.setSipxServiceManager(m_sipxServiceManager);
        generator.setForwardingRules(forwardingRules);

        generator.generate(dialPlanContext);

        checkConfigFileGeneration(generator, authRules, "authrules.xml");
        checkConfigFileGeneration(generator, mappingRules, "mappingrules.xml");
        checkConfigFileGeneration(generator, fallbackRules, "fallbackrules.xml");

        verify(dialingRuleProvider, dialPlanContext);
    }

    /**
     * Execute test for a single configuration type. Tries to generate it directly and generate
     * pretty formatted text through generator.
     */
    private void checkConfigFileGeneration(ConfigGenerator generator, RulesXmlFile configFile, String name)
            throws Exception {
        configFile.begin();
        configFile.end();
        Document document = configFile.getDocument();
        StringWriter writer = new StringWriter();
        document.write(writer);
        String actualXml =  AbstractConfigurationFile.getFileContent(generator.getConfigurationFileByName(name), null);
        String expectedXml = writer.getBuffer().toString();

        // The XML diff is getting confused by whitespace, so remove it
        actualXml = removeTroublesomeWhitespace(actualXml);
        expectedXml = removeTroublesomeWhitespace(expectedXml);

        assertXMLEqual("Comparing: " + name, expectedXml, actualXml);
    }

    private String removeTroublesomeWhitespace(String text) {
        return text.replaceAll("\\n\\s*", "");
    }
}
