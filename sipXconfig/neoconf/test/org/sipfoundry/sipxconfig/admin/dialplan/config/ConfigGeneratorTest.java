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

import static org.easymock.EasyMock.expectLastCall;

import java.io.StringWriter;
import java.util.Collections;

import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.DialPlanContext;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRuleProvider;
import org.sipfoundry.sipxconfig.admin.dialplan.EmergencyRouting;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcDeviceManager;
import org.sipfoundry.sipxconfig.admin.dialplan.sbc.SbcManager;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversal;
import org.sipfoundry.sipxconfig.nattraversal.NatTraversalManager;

/**
 * ConfigGeneratorTest
 */
public class ConfigGeneratorTest extends XMLTestCase {
    public ConfigGeneratorTest() {
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGetFileContent() throws Exception {
        IMocksControl planCtrl = EasyMock.createStrictControl();
        DialPlanContext dialPlanContext = planCtrl.createMock(DialPlanContext.class);
        DialingRuleProvider dialingRuleProvider = planCtrl.createMock(DialingRuleProvider.class);
        dialingRuleProvider.getDialingRules();
        planCtrl.andReturn(Collections.EMPTY_LIST);
        dialPlanContext.getGenerationRules();
        planCtrl.andReturn(Collections.EMPTY_LIST);
        dialPlanContext.getAttendantRules();
        planCtrl.andReturn(Collections.EMPTY_LIST);
        planCtrl.replay();

        EmergencyRouting er = new EmergencyRouting();

        ConfigGenerator generator = createConfigGenerator();
        generator.setDialingRuleProvider(dialingRuleProvider);
        generator.generate(dialPlanContext, er);

        AuthRules authRules = new AuthRules();
        checkConfigFileGeneration(generator, authRules, ConfigFileType.AUTH_RULES);
        MappingRules mappingRules = new MappingRules();
        checkConfigFileGeneration(generator, mappingRules, ConfigFileType.MAPPING_RULES);
        FallbackRules fallbackRules = new FallbackRules();
        checkConfigFileGeneration(generator, fallbackRules, ConfigFileType.FALLBACK_RULES);
        planCtrl.verify();
    }

    /**
     * Execute test for a single configuration type. Tries to generate it directly and generate
     * pretty formatted text through generator.
     */
    private void checkConfigFileGeneration(ConfigGenerator generator, RulesXmlFile configFile,
            ConfigFileType type) throws Exception {
        configFile.begin();
        configFile.end();
        Document document = configFile.getDocument();
        StringWriter writer = new StringWriter();
        document.write(writer);
        String actualXml = generator.getFileContent(type);
        String expectedXml = writer.getBuffer().toString();

        // The XML diff is getting confused by whitespace, so remove it
        actualXml = removeTroublesomeWhitespace(actualXml);
        expectedXml = removeTroublesomeWhitespace(expectedXml);

        assertXMLEqual("Comparing: " + type, expectedXml, actualXml);
    }

    private String removeTroublesomeWhitespace(String text) {
        return text.replaceAll("\\n\\s*", "");
    }

    public static ConfigGenerator createConfigGenerator() {
        SbcManager sbcManager = EasyMock.createNiceMock(SbcManager.class);

        SbcDeviceManager sbcDeviceManager = EasyMock.createNiceMock(SbcDeviceManager.class);

        NatTraversalManager natTraversalManager = EasyMock.createNiceMock(NatTraversalManager.class);
        NatTraversal natTraversal = new NatTraversal();
        natTraversalManager.getNatTraversal();
        expectLastCall().andReturn(natTraversal);

        EasyMock.replay(sbcManager, sbcDeviceManager, natTraversalManager);

        ConfigGenerator generator = new ConfigGenerator();
        generator.getForwardingRules().setVelocityEngine(TestHelper.getVelocityEngine());
        generator.getForwardingRules().setSbcManager(sbcManager);

        generator.getNatTraversalRules().setSbcManager(sbcManager);
        generator.getNatTraversalRules().setNatTraversalManager(natTraversalManager);
        generator.getNatTraversalRules().setSbcDeviceManager(sbcDeviceManager);
        generator.getNatTraversalRules().setVelocityEngine(TestHelper.getVelocityEngine());

        return generator;
    }
}
