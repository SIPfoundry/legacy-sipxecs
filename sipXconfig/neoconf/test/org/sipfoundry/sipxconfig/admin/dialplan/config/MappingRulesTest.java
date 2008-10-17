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

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.VisitorSupport;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.XmlUnitHelper;
import org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.ExchangeMediaServer;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.MappingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.MediaServer;
import org.sipfoundry.sipxconfig.admin.dialplan.SipXMediaServer;
import org.sipfoundry.sipxconfig.admin.dialplan.SipXMediaServerTest;
import org.sipfoundry.sipxconfig.admin.dialplan.VoicemailRedirectRule;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.admin.parkorbit.MohRule;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.speeddial.RlsRule;

/**
 * MappingRulesTest
 */
public class MappingRulesTest extends XMLTestCase {
    private static final String VOICEMAIL_SERVER = "https%3A%2F%2Flocalhost%3A443";

    public MappingRulesTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGetDocument() throws Exception {
        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        mappingRules.end();
        Document document = mappingRules.getDocument();

        String xml = XmlUnitHelper.asString(document);
        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathExists("/mappings/hostMatch/hostPattern", xml);
        assertXpathEvaluatesTo("${SIPXCHANGE_DOMAIN_NAME}", "/mappings/hostMatch/hostPattern", xml);
        assertXpathEvaluatesTo("${MY_FULL_HOSTNAME}", "/mappings/hostMatch/hostPattern[2]", xml);
        assertXpathEvaluatesTo("${MY_HOSTNAME}", "/mappings/hostMatch/hostPattern[3]", xml);
        assertXpathEvaluatesTo("${MY_IP_ADDR}", "/mappings/hostMatch/hostPattern[4]", xml);
    }

    public void testGetDocumentInvalidExternals() throws Exception {
        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        mappingRules.end();
        mappingRules.setExternalRulesFileName("/invalid/file/name");
        Document document = mappingRules.getDocument();

        String xml = XmlUnitHelper.asString(document);
        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathExists("/mappings/hostMatch/hostPattern", xml);
    }

    public void testGetDocumentValidExternals() throws Exception {
        URL resource = getClass().getResource("external_mappingrules.test.xml");

        MappingRules mappingRules = new MappingRules();
        mappingRules.setExternalRulesFileName(resource.getFile());
        mappingRules.begin();
        mappingRules.end();
        Document document = mappingRules.getDocument();

        String xml = XmlUnitHelper.asString(document);

        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathEvaluatesTo("some_other.domain.com", "/mappings/hostMatch/hostPattern", xml);
        assertXpathEvaluatesTo("${SIPXCHANGE_DOMAIN_NAME}",
                "/mappings/hostMatch/userMatch/permissionMatch/transform/host", xml);

        assertXpathExists("/mappings/hostMatch[2]/hostPattern", xml);
    }

    public void testGetDocumentValidExternalsExtraSpaceInFilename() throws Exception {
        URL resource = getClass().getResource("external_mappingrules.test.xml");

        MappingRules mappingRules = new MappingRules();
        mappingRules.setExternalRulesFileName(resource.getFile() + " ");
        mappingRules.begin();
        mappingRules.end();
        Document document = mappingRules.getDocument();

        String xml = XmlUnitHelper.asString(document);

        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathEvaluatesTo("some_other.domain.com", "/mappings/hostMatch/hostPattern", xml);
        assertXpathEvaluatesTo("${SIPXCHANGE_DOMAIN_NAME}",
                "/mappings/hostMatch/userMatch/permissionMatch/transform/host", xml);

        assertXpathExists("/mappings/hostMatch[2]/hostPattern", xml);
    }

    /**
     * This is mostly to demonstrate how complicated the XPatch expression becomes for a document
     * with a namespace
     * 
     * @param document
     */
    static void dumpXPaths(Document document) {
        VisitorSupport support = new VisitorSupport() {
            public void visit(Element node) {
                System.err.println(node.getPath());
            }
        };
        document.accept(support);
    }

    public void testGetHostMatch() throws Exception {
        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        mappingRules.end();
        Element hostMatch = mappingRules.getFirstHostMatch();
        Document document = mappingRules.getDocument();
        assertSame(document, hostMatch.getDocument());
        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");
        assertEquals("/*[name()='mappings']/*[name()='hostMatch']", hostMatch.getPath());
    }

    public void testGenerate() throws Exception {
        UrlTransform voicemail = new UrlTransform();
        voicemail.setUrl("<sip:{digits}@localhost;transport=tcp;"
                + "play=" + VOICEMAIL_SERVER + "/sipx-cgi/voicemail/mediaserver.cgi?action=deposit&mailbox={digits}>;q=0.1");

        UrlTransform voicemail2 = new UrlTransform();
        voicemail2.setUrl("<sip:{digits}@testserver;"
                + "play=" + VOICEMAIL_SERVER + "/sipx-cgi/voicemail/mediaserver.cgi?action=deposit&mailbox={digits}>;q=0.001");

        IMocksControl control = EasyMock.createNiceControl();
        IDialingRule rule = control.createMock(IDialingRule.class);
        rule.isInternal();
        control.andReturn(true);
        rule.getHostPatterns();
        control.andReturn(ArrayUtils.EMPTY_STRING_ARRAY);
        rule.getName();
        control.andReturn(null);
        rule.getDescription();
        control.andReturn("my rule description");
        rule.getPatterns();
        control.andReturn(new String[] {
            "x."
        }).anyTimes();
        rule.isInternal();
        control.andReturn(true);
        rule.getPermissionNames();
        control.andReturn(Arrays.asList(new String[] {
            PermissionName.VOICEMAIL.getName()
        }));
        rule.getTransforms();
        control.andReturn(new Transform[] {
            voicemail, voicemail2
        });
        control.replay();

        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        mappingRules.generate(rule);
        mappingRules.end();

        Document document = mappingRules.getDocument();

        String domDoc = XmlUnitHelper.asString(document);

        assertXpathEvaluatesTo("my rule description", "/mappings/hostMatch/userMatch/description", domDoc);
        assertXpathEvaluatesTo("x.", "/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathEvaluatesTo("Voicemail", "/mappings/hostMatch/userMatch/permissionMatch/permission", domDoc);
        assertXpathEvaluatesTo(voicemail.getUrl(), "/mappings/hostMatch/userMatch/permissionMatch/transform/url",
                domDoc);
        assertXpathEvaluatesTo(voicemail2.getUrl(), "/mappings/hostMatch/userMatch/permissionMatch/transform[2]/url",
                domDoc);

        control.verify();
    }

    public void testGenerateRuleWithGateways() throws Exception {
        IMocksControl control = EasyMock.createControl();
        IDialingRule rule = control.createMock(IDialingRule.class);
        rule.isInternal();
        control.andReturn(false);
        control.replay();

        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        mappingRules.generate(rule);
        mappingRules.end();

        Document document = mappingRules.getDocument();
        String domDoc = XmlUnitHelper.asString(document);

        assertXpathNotExists("/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathNotExists("/mappings/hostMatch/userMatch/permissionMatch", domDoc);
        assertXpathExists("/mappings/hostMatch/hostPattern", domDoc);

        control.verify();
    }

    public void testVoicemailRules() throws Exception {
        int extension = 3;
        List<DialingRule> rules = new ArrayList<DialingRule>();
        AutoAttendant aa = new AutoAttendant();
        aa.setSystemId(AutoAttendant.OPERATOR_ID);
        aa.setName("Operator");
        aa.resetToFactoryDefault();
        rules.add(new MohRule());
        rules.add(new RlsRule());

        LocalizationContext lc = EasyMock.createNiceMock(LocalizationContext.class);

        SipXMediaServer mediaServer = new SipXMediaServer();
        SipXMediaServerTest.configureMediaServer(mediaServer);

        mediaServer.setLocalizationContext(lc);
        MediaServer exchangeMediaServer = new ExchangeMediaServer("exchange.example.com", "102");
        exchangeMediaServer.setLocalizationContext(lc);

        EasyMock.replay(lc);

        rules.add(new MappingRule.Operator(aa, "100", new String[] {
            "operator", "0"
        }, mediaServer));
        rules.add(new MappingRule.Voicemail("101", mediaServer));
        rules.add(new MappingRule.Voicemail("102", exchangeMediaServer));
        rules.add(new MappingRule.VoicemailTransfer("2", extension, mediaServer));
        rules.add(new MappingRule.VoicemailTransfer("2", extension, exchangeMediaServer));
        rules.add(new MappingRule.VoicemailFallback(mediaServer));
        rules.add(new MappingRule.VoicemailFallback(exchangeMediaServer));
        rules.add(new VoicemailRedirectRule());


        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        for (DialingRule rule : rules) {
            mappingRules.generate(rule);
        }
        mappingRules.end();

        String generatedXml = mappingRules.getFileContent();

        InputStream referenceXmlStream = getClass().getResourceAsStream("mappingrules-multiple-servers.test.xml");

        assertXMLEqual(new InputStreamReader(referenceXmlStream), new StringReader(generatedXml));
        EasyMock.verify(lc);
    }

    public void testEquals() throws Exception {
        AbstractConfigurationFile f1 = new MappingRules();
        f1.setName("mappingrules.xml.in");
        AbstractConfigurationFile f2 = new MappingRules();
        f2.setName("mappingrules.xml.in");
        AbstractConfigurationFile f3 = new FallbackRules();
        f3.setName("fallbackrules.xml.in");
        assertEquals(f1, f2);
        assertNotSame(f1, f2);
        assertEquals(f1.hashCode(), f2.hashCode());
        assertFalse(f1.equals(null));
        assertFalse(f1.equals(f3));
    }

    public void testHostPatternProvider() throws Exception {
        IMocksControl control = EasyMock.createNiceControl();
        IDialingRule rule = control.createMock(IDialingRule.class);
        rule.isInternal();
        control.andReturn(true).times(2);
        rule.getHostPatterns();
        control.andReturn(new String[] {
            "gander"
        });
        rule.getPatterns();
        control.andReturn(new String[] {
            "dot"
        });
        rule.getPermissionNames();
        control.andReturn(Collections.EMPTY_LIST);
        rule.getTransforms();
        control.andReturn(new Transform[0]);

        control.replay();

        MappingRules mappingRules = new MappingRules();
        mappingRules.begin();
        mappingRules.generate(rule);
        mappingRules.end();

        Document document = mappingRules.getDocument();
        String domDoc = XmlUnitHelper.asString(document);

        assertXpathExists("/mappings/hostMatch[1]/hostPattern", domDoc);
        assertXpathExists("/mappings/hostMatch[1]/userMatch/userPattern", domDoc);

        control.verify();
    }
}
