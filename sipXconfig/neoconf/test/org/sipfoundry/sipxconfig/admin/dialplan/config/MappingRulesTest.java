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

import java.io.InputStream;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.CallTag;


import org.apache.commons.io.IOUtils;
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
import org.sipfoundry.sipxconfig.admin.commserver.Location;
import org.sipfoundry.sipxconfig.admin.commserver.LocationsManager;
import org.sipfoundry.sipxconfig.admin.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.ExchangeMediaServer;
import org.sipfoundry.sipxconfig.admin.dialplan.FreeswitchMediaServer;
import org.sipfoundry.sipxconfig.admin.dialplan.FreeswitchMediaServerTest;
import org.sipfoundry.sipxconfig.admin.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.MappingRule;
import org.sipfoundry.sipxconfig.admin.dialplan.MediaServer;
import org.sipfoundry.sipxconfig.admin.dialplan.VoicemailRedirectRule;
import org.sipfoundry.sipxconfig.admin.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.moh.MohRule;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.service.SipxPageService;
import org.sipfoundry.sipxconfig.service.SipxParkService;
import org.sipfoundry.sipxconfig.service.SipxRlsService;
import org.sipfoundry.sipxconfig.service.SipxServiceManager;
import org.sipfoundry.sipxconfig.speeddial.RlsRule;
import org.sipfoundry.sipxconfig.test.TestUtil;

import static org.sipfoundry.sipxconfig.admin.AbstractConfigurationFile.getFileContent;

/**
 * MappingRulesTest
 */
public class MappingRulesTest extends XMLTestCase {
    private static final String VOICEMAIL_SERVER = "https%3A%2F%2F192.168.1.1%3A443";

    private final DomainManager m_domainManager;

    // Object Under Test
    private final MappingRules m_out;

    public MappingRulesTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);

        m_domainManager = TestUtil.getMockDomainManager();
        EasyMock.replay(m_domainManager);

        m_out = new MappingRules();
        m_out.setDomainManager(m_domainManager);

        Location serviceLocation = new Location();
        serviceLocation.setAddress("192.168.1.5");
        List<Location> locations = new ArrayList<Location>();
        locations.add(serviceLocation);
        LocationsManager locationsManager = EasyMock.createNiceMock(LocationsManager.class);

        SipxParkService parkService = new SipxParkService();
        parkService.setBeanName(SipxParkService.BEAN_ID);
        parkService.setParkServerSipPort("9905");
        locationsManager.getLocationsForService(parkService);
        EasyMock.expectLastCall().andReturn(locations).anyTimes();
        parkService.setLocationsManager(locationsManager);

        SipxRlsService rlsService = new SipxRlsService();
        rlsService.setBeanName(SipxRlsService.BEAN_ID);
        rlsService.setRlsPort("9906");
        locationsManager.getLocationsForService(rlsService);
        EasyMock.expectLastCall().andReturn(locations).anyTimes();
        rlsService.setLocationsManager(locationsManager);

        SipxPageService pageService = new SipxPageService() {
            @Override
            public String getSipPort() {
                return "9910";
            }
        };
        pageService.setBeanName(SipxPageService.BEAN_ID);
        locationsManager.getLocationsForService(pageService);
        EasyMock.expectLastCall().andReturn(locations).anyTimes();
        pageService.setLocationsManager(locationsManager);

        EasyMock.replay(locationsManager);

        SipxServiceManager sipxServiceManager = TestUtil.getMockSipxServiceManager(true, parkService, rlsService,
                pageService);
        m_out.setSipxServiceManager(sipxServiceManager);
    }

    public void testGetDocument() throws Exception {
        m_out.begin();
        m_out.end();
        m_out.localizeDocument(TestUtil.createDefaultLocation());
        Document document = m_out.getDocument();

        String xml = TestUtil.asString(document);
        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathExists("/mappings/hostMatch/hostPattern", xml);
        assertXpathEvaluatesTo("example.org", "/mappings/hostMatch/hostPattern", xml);
        assertXpathEvaluatesTo("sipx.example.org", "/mappings/hostMatch/hostPattern[2]", xml);
        assertXpathEvaluatesTo("sipx", "/mappings/hostMatch/hostPattern[3]", xml);
        assertXpathEvaluatesTo("192.168.1.1", "/mappings/hostMatch/hostPattern[4]", xml);
    }

    public void testGetDocumentInvalidExternals() throws Exception {
        m_out.begin();
        m_out.end();
        m_out.setExternalRulesFileName("/invalid/file/name");
        Document document = m_out.getDocument();

        String xml = TestUtil.asString(document);
        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathExists("/mappings/hostMatch/hostPattern", xml);
    }

    public void testGetDocumentValidExternals() throws Exception {
        URL resource = getClass().getResource("external_mappingrules.test.xml");

        m_out.setExternalRulesFileName(resource.getFile());
        m_out.begin();
        m_out.end();
        m_out.localizeDocument(TestUtil.createDefaultLocation());
        Document document = m_out.getDocument();

        String xml = TestUtil.asString(document);

        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathEvaluatesTo("some_other.domain.com", "/mappings/hostMatch/hostPattern", xml);
        assertXpathEvaluatesTo("example.org", "/mappings/hostMatch/userMatch/permissionMatch/transform/host", xml);

        assertXpathExists("/mappings/hostMatch[2]/hostPattern", xml);
    }

    public void testGetDocumentValidExternalsExtraSpaceInFilename() throws Exception {
        URL resource = getClass().getResource("external_mappingrules.test.xml");

        m_out.setExternalRulesFileName(resource.getFile() + " ");
        m_out.begin();
        m_out.end();
        m_out.localizeDocument(TestUtil.createDefaultLocation());
        Document document = m_out.getDocument();

        String xml = TestUtil.asString(document);

        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathEvaluatesTo("some_other.domain.com", "/mappings/hostMatch/hostPattern", xml);
        assertXpathEvaluatesTo("example.org", "/mappings/hostMatch/userMatch/permissionMatch/transform/host", xml);

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
            @Override
            public void visit(Element node) {
                System.err.println(node.getPath());
            }
        };
        document.accept(support);
    }

    public void testGetHostMatch() throws Exception {
        m_out.begin();
        m_out.end();
        Element hostMatch = m_out.getFirstHostMatch();
        Document document = m_out.getDocument();
        assertSame(document, hostMatch.getDocument());
        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");
        assertEquals("/*[name()='mappings']/*[name()='hostMatch']", hostMatch.getPath());
    }

    public void testGenerate() throws Exception {
        UrlTransform voicemail = new UrlTransform();
        voicemail.setUrl("<sip:{digits}@localhost;transport=tcp;" + "play=" + VOICEMAIL_SERVER
                + "/sipx-cgi/voicemail/mediaserver.cgi?action=deposit&mailbox={digits}>;q=0.1");

        UrlTransform voicemail2 = new UrlTransform();
        voicemail2.setUrl("<sip:{digits}@testserver;" + "play=" + VOICEMAIL_SERVER
                + "/sipx-cgi/voicemail/mediaserver.cgi?action=deposit&mailbox={digits}>;q=0.001");

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
        rule.isTargetPermission();
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

        m_out.begin();
        m_out.generate(rule);
        m_out.end();

        Document document = m_out.getDocument();

        String domDoc = TestUtil.asString(document);

        assertXpathEvaluatesTo("my rule description", "/mappings/hostMatch/userMatch/description", domDoc);
        assertXpathEvaluatesTo("x.", "/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathEvaluatesTo("Voicemail", "/mappings/hostMatch/userMatch/permissionMatch/permission", domDoc);
        assertXpathEvaluatesTo(voicemail.getUrl(), "/mappings/hostMatch/userMatch/permissionMatch/transform/url",
                domDoc);
        assertXpathEvaluatesTo(voicemail2.getUrl(),
                "/mappings/hostMatch/userMatch/permissionMatch/transform[2]/url", domDoc);

        control.verify();
    }

    public void testGenerateRuleWithGateways() throws Exception {
        IMocksControl control = EasyMock.createControl();
        IDialingRule rule = control.createMock(IDialingRule.class);
        rule.isInternal();
        control.andReturn(false);
        rule.getCallTag();
        control.andReturn(CallTag.UNK).anyTimes();
        control.replay();

        m_out.begin();
        m_out.generate(rule);
        m_out.end();

        Document document = m_out.getDocument();
        String domDoc = TestUtil.asString(document);

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
        rules.add(new MohRule("192.168.1.5:9905", "~~mh~u"));
        rules.add(new RlsRule());

        LocalizationContext lc = EasyMock.createNiceMock(LocalizationContext.class);

        FreeswitchMediaServer mediaServer = new FreeswitchMediaServer();
        FreeswitchMediaServerTest.configureMediaServer(mediaServer);

        mediaServer.setLocalizationContext(lc);
        MediaServer exchangeMediaServer = new ExchangeMediaServer("exchange.example.com", "102");
        exchangeMediaServer.setLocalizationContext(lc);

        EasyMock.replay(lc);

        rules.add(new MappingRule.Operator(aa, "100", new String[] {
            "operator", "0"
        }, mediaServer));
        rules.add(new MappingRule.Voicemail("101", "+123456789", mediaServer));
        rules.add(new MappingRule.Voicemail("102",null, exchangeMediaServer));
        rules.add(new MappingRule.VoicemailTransfer("2", extension, mediaServer));
        rules.add(new MappingRule.VoicemailTransfer("2", extension, exchangeMediaServer));
        rules.add(new MappingRule.VoicemailFallback(mediaServer));
        rules.add(new MappingRule.VoicemailFallback(exchangeMediaServer));
        rules.add(new VoicemailRedirectRule());

        m_out.begin();
        for (DialingRule rule : rules) {
            m_out.generate(rule);
        }
        m_out.end();
        m_out.localizeDocument(TestUtil.createDefaultLocation());

        String generatedXml = getFileContent(m_out, null);

        InputStream referenceXmlStream = getClass().getResourceAsStream("mappingrules-multiple-servers.test.xml");

        assertEquals(IOUtils.toString(referenceXmlStream, "UTF-8"), generatedXml);
        EasyMock.verify(lc);
    }

    public void testEquals() throws Exception {
        AbstractConfigurationFile f1 = new MappingRules();
        f1.setName("mappingrules.xml");
        AbstractConfigurationFile f2 = new MappingRules();
        f2.setName("mappingrules.xml");
        AbstractConfigurationFile f3 = new FallbackRules();
        f3.setName("fallbackrules.xml");
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
        control.andReturn(true);
        rule.isTargetPermission();
        control.andReturn(true);
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

        m_out.begin();
        m_out.generate(rule);
        m_out.end();

        Document document = m_out.getDocument();
        String domDoc = TestUtil.asString(document);

        assertXpathExists("/mappings/hostMatch[1]/hostPattern", domDoc);
        assertXpathExists("/mappings/hostMatch[1]/userMatch/userPattern", domDoc);

        assertXpathEvaluatesTo("gander", "/mappings/hostMatch[1]/hostPattern", domDoc);
        assertXpathNotExists("/mappings/hostMatch[1]/hostPattern[2]", domDoc);

        control.verify();
    }

    public void testGenerateInternalRuleWithSourcePermission() throws Exception {
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
            "xxx"
        }).anyTimes();
        rule.isTargetPermission();
        control.andReturn(false);
        rule.getTransforms();
        control.andReturn(new Transform[0]);
        control.replay();

        m_out.begin();
        m_out.generate(rule);
        m_out.end();

        Document document = m_out.getDocument();

        String domDoc = TestUtil.asString(document);

        assertXpathEvaluatesTo("my rule description", "/mappings/hostMatch/userMatch/description", domDoc);
        assertXpathEvaluatesTo("xxx", "/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathNotExists("/mappings/hostMatch/userMatch/permissionMatch/permission", domDoc);

        control.verify();
    }
}
