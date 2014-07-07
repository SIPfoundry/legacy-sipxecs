/*
*
*
* Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
* Contributors retain copyright to elements licensed under a Contributor Agreement.
* Licensed to the User under the LGPL license.
*
* $
*/
package org.sipfoundry.sipxconfig.phone.polycom;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.replay;

import java.io.File;
import java.io.FileReader;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

import org.apache.commons.io.FileUtils;
import org.dom4j.Attribute;
import org.dom4j.Document;
import org.dom4j.dom.DOMDocumentFactory;
import org.dom4j.dom.DOMElement;
import org.dom4j.io.SAXReader;
import org.easymock.classextension.EasyMock;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.FileSystemProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.feature.FeatureManager;
import org.sipfoundry.sipxconfig.moh.MohAddressFactory;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.mwi.Mwi;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.rls.Rls;
import org.sipfoundry.sipxconfig.test.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.test.TestHelper;

/**
* Tests file phone.cfg generation
*/
public class PhoneConfigurationTest extends PolycomXmlTestCase {

    private PolycomPhone phone;
    private ProfileGenerator m_pg;
    private MemoryProfileLocation m_location;
    private PhoneTestDriver m_testDriver;

    @Override
    protected void setUp() throws Exception {
        FeatureManager featureManagerMock = createMock(FeatureManager.class);
        featureManagerMock.isFeatureEnabled(Mwi.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();

        featureManagerMock.isFeatureEnabled(Rls.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();

        featureManagerMock.isFeatureEnabled(MusicOnHoldManager.FEATURE);
        EasyMock.expectLastCall().andReturn(true).anyTimes();
        phone = new PolycomPhone();
        phone.setFeatureManager(featureManagerMock);
        EasyMock.replay(featureManagerMock);

        m_location = TestHelper.setVelocityProfileGenerator(phone, TestHelper.getEtcDir());
        m_pg = phone.getProfileGenerator();
    }

    /**
* Test 2.x profile generation.
*/
    public void testGenerateProfileVersion20() throws Exception {


        FileSystemProfileLocation location = TestHelper.setFsVelocityProfileGenerator(phone, TestHelper.getEtcDir());;
        location.setParentDir(TestHelper.getTestOutputDirectory());

        PolycomModel model = new PolycomModel();
        model.setDefaultVersion(PolycomModel.VER_3_2_X);
        model.setMaxLineCount(6);
        phone.setModel(model);

        m_testDriver = PhoneTestDriver.supplyTestDataWithSpecialChars(phone, true, false, true, true);
        m_testDriver.getPrimaryLine().setSettingValue("reg/label", "Joe & Joe");

        // XCF-3581: No longer automatically generating phone emergency dial routing. These
        // settings
        // are as if they'd been manually configured under Line 1 - Dial Plan - Emergency Routing.
        Line line = phone.getLines().get(0);
        line.setSettingValue("line-dialplan/digitmap/routing.1/address", "emergency-gateway.example.org");
        line.setSettingValue("line-dialplan/digitmap/routing.1/port", "5440");
        line.setSettingValue("line-dialplan/digitmap/routing.1/emergency.1.value", "911,9911");

        phone.beforeProfileGeneration();
        PhoneConfiguration cfg = new PhoneConfiguration(phone);

        m_pg.generate(location, cfg, null, "profile.XXX");

        System.out.println("*** BEGIN actual profile content. ***");
        dumpXml(new FileReader(TestHelper.getTestOutputDirectory()+"/profile.XXX"), System.out);
        System.out.println("*** END actual profile content. ***");
        System.out.println("*** BEGIN expected profile content. ***");
        dumpXml(getClass().getResourceAsStream("expected-phone.cfg.xml"), System.out);
        System.out.println("*** END expected profile content. ***");

        //Use string comparison here since we want to capture special chars like German umlaut, etc;
        //using xml comparison via assertXmlEquals will reconvert the characters encoded
        assertEquals(FileUtils.readFileToString(new File(getClass().getResource("expected-phone.cfg.xml").getFile())), FileUtils.readFileToString(new File(TestHelper.getTestOutputDirectory()+"/profile.XXX")));
    }

    /**
* XX-6976: Polycom/Nortel 12x0: Give User-less profiles the sipXprovision special user
* credentials and MAC hash ID label
*
* @throws Exception
*/
    public void testGenerateSpecialUserRegistrationWhenNoConfiguredLines() throws Exception {
        PolycomModel model = new PolycomModel();
        model.setMaxLineCount(2);
        model.setModelId("polycom330");
        model.setDefaultVersion(PolycomModel.VER_3_2_X);
        Set<String> features = new HashSet<String>();
        features.add("intercom");
        features.add("voiceQualityMonitoring");
        features.add("OTHERS_CodecPref");
        model.setSupportedFeatures(features);
        phone.setModel(model);

        // The phone has no lines configured.
        m_testDriver = PhoneTestDriver.supplyTestData(phone, new ArrayList<User>());

        //String expectedMohUri = "~~mh@example.org";
        String expectedMohUri = "";// actually empty, since there are no lines
        MohAddressFactory moh = EasyMock.createNiceMock(MohAddressFactory.class);
        moh.getPersonalMohFilesUri(SpecialUserType.PHONE_PROVISION.getUserName());
        expectLastCall().andReturn("sip:" + expectedMohUri).anyTimes();
        replay(moh);

        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext(TestHelper.getSystemEtcDir()));

        User special_user = new User();
        special_user.setPermissionManager(pm);
        special_user.setMohAddresses(moh);
        special_user.setSipPassword("abcd");
        special_user.setUserName(SpecialUserType.PHONE_PROVISION.getUserName());
        String expected_label = "ID: HL2";
        special_user.setFirstName(expected_label.split(" ")[0]);
        special_user.setLastName(expected_label.split(" ")[1]);


        phone.beforeProfileGeneration();
        PhoneConfiguration cfg = new PhoneConfiguration(phone);
        m_pg.generate(m_location, cfg, null, "profile");

        // The profile.
        SAXReader reader = new SAXReader(new DOMDocumentFactory());
        Document profile = reader.read(m_location.getReader());
        reader.getDocumentFactory().createDocument();

        // Test content from the registration node.
        String xml_path = "/phone1/reg";
        DOMElement reg_element = (DOMElement) profile.selectSingleNode(xml_path);
        System.out.println("*** BEGIN actual profile " + xml_path + " content. ***");
        dumpXml(reg_element, System.out, "");
        System.out.println("*** END actual profile " + xml_path + " content. ***");

        // The sipXprovisoin special user line.
        Attribute attribute = (Attribute) reg_element.selectSingleNode("@reg.1.auth.password");
        assertEquals(special_user.getSipPassword(), attribute.getStringValue());

        attribute = (Attribute) reg_element.selectSingleNode("@reg.1.address");
        assertEquals(special_user.getUserName(), attribute.getStringValue());

        attribute = (Attribute) reg_element.selectSingleNode("@reg.1.auth.userId");
        assertEquals(special_user.getUserName() + "/" + phone.getSerialNumber(), attribute.getStringValue());

        attribute = (Attribute) reg_element.selectSingleNode("@reg.1.displayName");
        assertEquals(expected_label, attribute.getStringValue());

        attribute = (Attribute) reg_element.selectSingleNode("@reg.1.label");
        assertEquals(expected_label, attribute.getStringValue());

        attribute = (Attribute) reg_element.selectSingleNode("@reg.1.server.1.address");
        assertEquals("sipfoundry.org", attribute.getStringValue());

        attribute = (Attribute) reg_element.selectSingleNode("@reg.1.server.1.address");
        assertEquals("sipfoundry.org", attribute.getStringValue());

        attribute = (Attribute) reg_element.selectSingleNode("@reg.1.musicOnHold.uri");
        assertEquals(expectedMohUri, attribute.getStringValue());

        // All others registrations are unused.
        for (int x = 2; x <= PhoneConfiguration.TEMPLATE_DEFAULT_LINE_COUNT; x++) {
            attribute = (Attribute) reg_element.selectSingleNode("@reg." + x + ".address");
            assertEquals("", attribute.getStringValue());
        }

        // Test content from the mwi node.
        xml_path = "/phone1/msg/mwi";
        DOMElement mwi_element = (DOMElement) profile.selectSingleNode(xml_path);
        System.out.println("*** BEGIN actual profile " + xml_path + " content. ***");
        dumpXml(reg_element, System.out, "");
        System.out.println("*** END actual profile " + xml_path + " content. ***");

        // All mwis are disabled.
        for (int x = 1; x <= PhoneConfiguration.TEMPLATE_DEFAULT_LINE_COUNT; x++) {
            attribute = (Attribute) mwi_element.selectSingleNode("@msg.mwi." + x + ".subscribe");
            assertEquals("", attribute.getStringValue());

            attribute = (Attribute) mwi_element.selectSingleNode("@msg.mwi." + x + ".callBackMode");
            assertEquals("disabled", attribute.getStringValue());
        }
    }

    public void testGenerateProfileVersion20WithoutVoicemailPermission() throws Exception {
        PolycomModel model = new PolycomModel();
        model.setDefaultVersion(PolycomModel.VER_3_1_X);
        model.setMaxLineCount(6);
        phone.setModel(model);

        List<User> users = new ArrayList<User>();

        PermissionManagerImpl pManager = new PermissionManagerImpl();
        pManager.setModelFilesContext(TestHelper.getModelFilesContext(TestHelper.getSystemEtcDir()));

        User user1 = new User();
        user1.setUserName("juser");
        user1.setFirstName("Joe");
        user1.setLastName("User");
        user1.setSipPassword("1234");
        user1.setPermissionManager(pManager);
        user1.setPermission(PermissionName.VOICEMAIL, false);

        User user2 = new User();
        user2.setUserName("kuser");
        user2.setFirstName("Kate");
        user2.setLastName("User");
        user2.setSipPassword("1234");
        user2.setPermissionManager(pManager);
        user2.setPermission(PermissionName.VOICEMAIL, true);

        users.add(user1);
        users.add(user2);

        m_testDriver = PhoneTestDriver.supplyTestData(phone, users);

        // XCF-3581: No longer automatically generating phone emergency dial routing. These
        // settings
        // are as if they'd been manually configured under Line 1 - Dial Plan - Emergency Routing.
        Line line = phone.getLines().get(0);
        line.setSettingValue("line-dialplan/digitmap/routing.1/address", "emergency-gateway.example.org");
        line.setSettingValue("line-dialplan/digitmap/routing.1/port", "5440");
        line.setSettingValue("line-dialplan/digitmap/routing.1/emergency.1.value", "911,9911");
        phone.beforeProfileGeneration();
        PhoneConfiguration cfg = new PhoneConfiguration(phone);

        m_pg.generate(m_location, cfg, null, "profile");

        assertPolycomXmlEquals(getClass().getResourceAsStream("expected-without-voicemail-permission-phone.cfg.xml"), m_location.getReader());
    }

    public void testGenerateSipxPhoneWithExternalLine() throws Exception {
        PolycomModel model = PolycomXmlTestCase.phoneModelBuilder("polycom335", getClass());
        phone.setModel(model);
        phone.setDeviceVersion(PolycomModel.VER_3_2_X);

        List<User> users = new ArrayList<User>();

        PermissionManagerImpl pManager = new PermissionManagerImpl();
        pManager.setModelFilesContext(TestHelper.getModelFilesContext(TestHelper.getSystemEtcDir()));

        m_testDriver = PhoneTestDriver.supplyTestData(phone, new ArrayList<User>());

        LineInfo li = new LineInfo();
        li.setDisplayName("dddd");
        li.setUserId("jon");
        li.setRegistrationServer("sipfoundry.org");
        li.setPassword("1234");
        li.setVoiceMail("");

        Line externalLine = phone.createLine();
        phone.addLine(externalLine);
        externalLine.setLineInfo(li);
        phone.beforeProfileGeneration();
        PhoneConfiguration cfg = new PhoneConfiguration(phone);

        m_pg.generate(m_location, cfg, null, "profile");

        assertPolycomXmlEquals(getClass().getResourceAsStream("expected-external-line-sipx-phone.cfg.xml"), m_location.getReader());
    }

    @Override
    protected void tearDown() throws Exception {
        new File(TestHelper.getTestOutputDirectory()+"/profile.XXX").delete();
        super.tearDown();
    }
}