/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import java.io.StringWriter;
import java.util.Collections;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.admin.dialplan.attendant.AutoAttendantsConfig;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;

import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.createStrictMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

/**
 * ConfigGeneratorTest
 */
public class ConfigGeneratorTest extends XMLTestCase {
    public ConfigGeneratorTest() {
        XMLUnit.setIgnoreWhitespace(true);
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
        authRules.setName("authrules.xml.in");
        generator.setAuthRules(authRules);

        MappingRules mappingRules = new MappingRules();
        mappingRules.setName("mappingrules.xml.in");
        generator.setMappingRules(mappingRules);

        FallbackRules fallbackRules = new FallbackRules();
        fallbackRules.setName("fallbackrules.xml.in");
        generator.setFallbackRules(fallbackRules);

        ForwardingRules forwardingRules = new ForwardingRules();
        forwardingRules.setName("fallbackrules.xml.in");
        forwardingRules.setVelocityEngine(TestHelper.getVelocityEngine());
        forwardingRules.setSbcManager(sbcManager);
        generator.setForwardingRules(forwardingRules);

        AutoAttendantsConfig autoAttendantsConfig = new AutoAttendantsConfig();
        generator.setAutoAttendantConfig(autoAttendantsConfig );

        generator.generate(dialPlanContext);

        checkConfigFileGeneration(generator, authRules, "authrules.xml.in");
        checkConfigFileGeneration(generator, mappingRules, "mappingrules.xml.in");
        checkConfigFileGeneration(generator, fallbackRules, "fallbackrules.xml.in");

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
        String actualXml = generator.getFileContent(name);
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
