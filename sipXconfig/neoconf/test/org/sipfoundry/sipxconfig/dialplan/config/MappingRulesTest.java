/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.dialplan.config;


import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.io.IOException;
import java.io.InputStream;
import java.io.StringWriter;
import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.ArrayUtils;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.VisitorSupport;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.commserver.Location;
import org.sipfoundry.sipxconfig.dialplan.AutoAttendant;
import org.sipfoundry.sipxconfig.dialplan.CallTag;
import org.sipfoundry.sipxconfig.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.dialplan.ExchangeMediaServer;
import org.sipfoundry.sipxconfig.dialplan.IDialingRule;
import org.sipfoundry.sipxconfig.dialplan.MappingRule;
import org.sipfoundry.sipxconfig.dialplan.MediaServer;
import org.sipfoundry.sipxconfig.dialplan.VoicemailRedirectRule;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchFeature;
import org.sipfoundry.sipxconfig.freeswitch.FreeswitchMediaServer;
import org.sipfoundry.sipxconfig.localization.LocalizationContext;
import org.sipfoundry.sipxconfig.moh.MohRule;
import org.sipfoundry.sipxconfig.paging.PagingContext;
import org.sipfoundry.sipxconfig.parkorbit.ParkOrbitContext;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.rls.Rls;
import org.sipfoundry.sipxconfig.rls.RlsRule;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.sipfoundry.sipxconfig.test.XmlUnitHelper;

/**
 * MappingRulesTest
 */
public class MappingRulesTest extends XMLTestCase {
    private static final String VOICEMAIL_SERVER = "https%3A%2F%2F192.168.1.1%3A443";
    private MappingRules m_out;
    private AddressManager m_addressManager;

    public MappingRulesTest() {
        XmlUnitHelper.setNamespaceAware(false);
        XMLUnit.setIgnoreWhitespace(true);
    }    
    
    public void setUp() {
        TestHelper.initDefaultDomain();
        m_out = new MappingRules();
        m_out.setDomainName("example.org");
        Location l = TestHelper.createDefaultLocation();
        m_out.setLocation(l);
        
        m_addressManager = createMock(AddressManager.class);
        m_addressManager.getSingleAddress(Rls.TCP_SIP, l);
        expectLastCall().andReturn(new Address(Rls.TCP_SIP, "192.168.1.5", 9906)).anyTimes();
        m_addressManager.getSingleAddress(ParkOrbitContext.SIP_TCP_PORT, l);
        expectLastCall().andReturn(new Address(ParkOrbitContext.SIP_TCP_PORT, "park.example.org", 100)).anyTimes();
        m_addressManager.getSingleAddress(PagingContext.SIP_TCP, l);
        expectLastCall().andReturn(new Address(PagingContext.SIP_TCP, "page.example.org", 101)).anyTimes();
        m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS, l);
        expectLastCall().andReturn(new Address(FreeswitchFeature.SIP_ADDRESS, "192.168.1.1", 102)).anyTimes();
        m_addressManager.getSingleAddress(FreeswitchFeature.SIP_ADDRESS);
        expectLastCall().andReturn(new Address(FreeswitchFeature.SIP_ADDRESS, "192.168.1.1", 102)).anyTimes();
        replay(m_addressManager);
        
        m_out.setAddressManager(m_addressManager);
    }

    public void testGetDocument() throws Exception {
        m_out.begin();
        m_out.end();
        Document document = m_out.getDocument();

        String xml = TestHelper.asString(document);
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

        String xml = TestHelper.asString(document);
        XmlUnitHelper.assertElementInNamespace(document.getRootElement(),
                "http://www.sipfoundry.org/sipX/schema/xml/urlmap-00-00");

        assertXpathExists("/mappings/hostMatch/hostPattern", xml);
    }

    public void testGetDocumentValidExternals() throws Exception {
        URL resource = getClass().getResource("external_mappingrules.test.xml");

        m_out.setExternalRulesFileName(resource.getFile());
        m_out.begin();
        m_out.end();
        Document document = m_out.getDocument();

        String xml = TestHelper.asString(document);

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
        m_out.setLocation(TestHelper.createDefaultLocation());
        Document document = m_out.getDocument();

        String xml = TestHelper.asString(document);

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
        Document document = m_out.getPreLocalizedDocument();
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

        String domDoc = TestHelper.asString(document);

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
        String domDoc = TestHelper.asString(document);

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
        mediaServer.setAddressManager(m_addressManager);
        //FreeswitchMediaServerTest.configureMediaServer(mediaServer);

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
        m_out.setLocation(TestHelper.createDefaultLocation());
        String generatedXml = toString(m_out);

        InputStream referenceXmlStream = getClass().getResourceAsStream("mappingrules-multiple-servers.test.xml");

        assertEquals(IOUtils.toString(referenceXmlStream, "UTF-8"), generatedXml);
        EasyMock.verify(lc);
    }
    
    private String toString(MappingRules rules) throws IOException {
        StringWriter wtr = new StringWriter();
        rules.write(wtr);        
        return wtr.toString();        
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
        String domDoc = TestHelper.asString(document);

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

        String domDoc = TestHelper.asString(document);

        assertXpathEvaluatesTo("my rule description", "/mappings/hostMatch/userMatch/description", domDoc);
        assertXpathEvaluatesTo("xxx", "/mappings/hostMatch/userMatch/userPattern", domDoc);
        assertXpathNotExists("/mappings/hostMatch/userMatch/permissionMatch/permission", domDoc);

        control.verify();
    }
}
