/*
 *
 *
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 *
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel12x0;

import java.io.InputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;

import junit.framework.TestCase;
import org.apache.commons.io.IOUtils;
import org.dom4j.Document;
import org.dom4j.dom.DOMDocumentFactory;
import org.dom4j.dom.DOMElement;
import org.dom4j.io.SAXReader;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import static org.easymock.EasyMock.expectLastCall;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.SpecialUser.SpecialUserType;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.nortel12x0.Nortel12x0Phone.Nortel12x0Context;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import static org.sipfoundry.sipxconfig.test.TestUtil.getModelDirectory;

public class Nortel12x0PhoneTest extends TestCase {

    private final Collection<PhonebookEntry> m_emptyPhonebook = Collections.<PhonebookEntry> emptyList();

    public void _testFactoryRegistered() {
        PhoneContext pc = (PhoneContext) TestHelper.getApplicationContext().getBean(PhoneContext.CONTEXT_BEAN_NAME);
        assertNotNull(pc.newPhone(new PhoneModel("nortel12x0")));
    }

    public void testGetFileName() throws Exception {

        Nortel12x0Phone phone = new Nortel12x0Phone();
        phone.setSerialNumber("00041c001e6c");
        assertEquals("Nortel/config/SIP00041C001E6C.xml", phone.getProfileFilename());
    }

    public void testGetSettings() {

        Nortel12x0Phone phone = new Nortel12x0Phone();
        PhoneModel model = new PhoneModel("nortel12x0");
        model.setLabel("Nortel IP Phone 1230");
        model.setModelDir("nortel12x0");
        model.setProfileTemplate("nortel12x0/nortel12x0.vm");
        phone.setModel(model);
        PhoneTestDriver.supplyTestData(phone);
        assertNotNull(phone.getSettings());
    }

    public void testExternalLine() throws Exception {

        Nortel12x0Phone phone = new Nortel12x0Phone();
        PhoneModel model = new PhoneModel("nortel12x0");
        model.setLabel("Nortel IP Phone 1230");
        model.setModelDir("nortel12x0");
        model.setProfileTemplate("nortel12x0/nortel12x0.vm");
        phone.setModel(model);

        PhoneTestDriver.supplyTestData(phone, new ArrayList<User>());
        LineInfo li = new LineInfo();
        li.setDisplayName("abc xyz");
        li.setUserId("def");
        li.setRegistrationServer("example.org");
        li.setPassword("1234");

        Line line = phone.createLine();
        phone.addLine(line);
        line.setLineInfo(li);

        assertEquals("\"abc xyz\"<sip:def@example.org>", line.getUri());
    }

    public void testRestart() throws Exception {

        Nortel12x0Phone phone = new Nortel12x0Phone();
        PhoneModel model = new PhoneModel("nortel12x0");
        model.setLabel("Nortel IP Phone 1230");
        model.setModelDir("nortel12x0");
        model.setProfileTemplate("nortel12x0/nortel12x0.vm");
        phone.setModel(model);

        PhoneTestDriver testDriver = PhoneTestDriver.supplyTestData(phone,true,false,false,true);
        phone.restart();

        testDriver.sipControl.verify();
    }

    public void testRestartNoLine() throws Exception {

        Nortel12x0Phone phone = new Nortel12x0Phone();
        PhoneModel model = new PhoneModel("nortel12x0");
        model.setLabel("Nortel IP Phone 1230");
        model.setModelDir("nortel12x0");
        model.setProfileTemplate("nortel12x0/nortel12x0.vm");
        phone.setModel(model);

        PhoneTestDriver.supplyTestData(phone, new ArrayList<User>(), true);
        phone.restart();
    }

    public void testGenerateTypicalProfile() throws Exception {

        Nortel12x0Phone phone = new Nortel12x0Phone();
        PhoneModel model = new PhoneModel("nortel12x0");
        model.setLabel("Nortel IP Phone 1230");
        model.setModelDir("nortel12x0");
        model.setProfileTemplate("nortel12x0/nortel12x0.vm");
        phone.setModel(model);
        PhoneTestDriver.supplyTestData(phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);
        phone.generateProfiles(location);
        InputStream expectedProfile = getClass().getResourceAsStream("expected-config");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        assertEquals(expected, location.toString());
    }

    private static final String expected_label = "ID: YBU";

    private static final User special_user;

    static {

        String expectedMohUri = "~~mh@example.org";
        IMocksControl mohManagerControl = EasyMock.createNiceControl();
        MusicOnHoldManager mohManager = mohManagerControl.createMock(MusicOnHoldManager.class);
        mohManager.getDefaultMohUri();
        expectLastCall().andReturn("sip:" + expectedMohUri).anyTimes();
        mohManagerControl.replay();

        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext(getModelDirectory("neoconf")));

        special_user = new User();
        special_user.setPermissionManager(pm);
        special_user.setMusicOnHoldManager(mohManager);
        special_user.setSipPassword("the ~~id~sipXprovision password");
        special_user.setUserName(SpecialUserType.PHONE_PROVISION.getUserName());
        special_user.setFirstName(expected_label.split(" ")[0]);
        special_user.setLastName(expected_label.split(" ")[1]);
    }

    protected static IMocksControl setMockPhoneContextForSpecialUser(Nortel12x0Phone phone) {

        IMocksControl phoneContextControl = EasyMock.createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);

        phoneContext.createSpecialPhoneProvisionUser(phone.getSerialNumber());
        phoneContextControl.andReturn(special_user).once();
        phoneContextControl.replay();

        phone.setPhoneContext(phoneContext);

        return phoneContextControl;
    }

    protected static Nortel12x0Phone getPhoneWithMockPhoneContextForSpecialUser() {

        Nortel12x0Phone phone = new Nortel12x0Phone();
        phone.setModel(new PhoneModel("nortel12x0"));
        setMockPhoneContextForSpecialUser(phone);

        return phone;
    }

    /**
     * XX-6976: Polycom/Nortel 12x0: Give User-less profiles the sipXprovision special user credentials
     * and MAC hash ID label
     *
     * @throws Exception
     */
    public void testGenerateSpecialUserRegistrationWhenNoConfiguredLines() throws Exception {

        Nortel12x0Phone phone = new Nortel12x0Phone();
        PhoneModel model = new PhoneModel("nortel12x0");
        model.setLabel("Nortel IP Phone 1230");
        model.setModelDir("nortel12x0");
        model.setProfileTemplate("nortel12x0/nortel12x0.vm");
        phone.setModel(model);

        PhoneTestDriver.supplyTestData(phone, new ArrayList<User>());
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        IMocksControl phoneContextControl = setMockPhoneContextForSpecialUser(phone);

        phone.generateProfiles(location);

        phoneContextControl.verify();

        // Test only content from the sipAccounts node.
        SAXReader reader = new SAXReader(new DOMDocumentFactory());
        Document profile = reader.read(location.getReader());
        reader.getDocumentFactory().createDocument();
        DOMElement accounts_element = (DOMElement) profile.selectSingleNode("/configuration/sipAccounts");

        assertNotNull("The <sipAccounts> node is missing.", accounts_element);

        assertEquals("There should be exactly one account enabled.", 1, accounts_element.elements().size());

        DOMElement l1 =  (DOMElement) accounts_element.selectSingleNode("account_L1");
        assertEquals(special_user.getUserName(), l1.selectSingleNode("authname").getStringValue());
        assertEquals(special_user.getSipPassword(), l1.selectSingleNode("authPassword").getStringValue());
        assertEquals(special_user.getUserName() + "/" + phone.getSerialNumber(),
                l1.selectSingleNode("authId").getStringValue());
        assertEquals(expected_label, l1.selectSingleNode("displayname").getStringValue());
        assertEquals("FALSE", l1.selectSingleNode("mwiSubscribe").getStringValue());
    }

    public void testGenerateProfilesForWithBlfSpeedDial() throws Exception {

        Nortel12x0Phone phone = new Nortel12x0Phone();
        PhoneModel model = new PhoneModel("nortel12x0");
        model.setLabel("Nortel IP Phone 1230");
        model.setModelDir("nortel12x0");
        model.setProfileTemplate("nortel12x0/nortel12x0.vm");
        phone.setModel(model);

        User user1 = new User();

        user1.setUserName("juser");
        user1.setFirstName("Joe");
        user1.setLastName("User");
        user1.setSipPassword("1234");

        PhoneTestDriver.supplyTestData(phone, Collections.singletonList(user1));
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        Button[] buttons = new Button[] {
            new Button("a,b,c", "123@sipfoundry.org"), new Button("def_def", "456"), new Button("xyz abc", "789"),
            new Button(null, "147"), new Button("Joe User", "258@sipfoundry.com"), new Button("", "369")
        };
        buttons[1].setBlf(true);

        SpeedDial sp = new SpeedDial();
        sp.setButtons(Arrays.asList(buttons));
        sp.setUser(user1);

        User user2 = new User();
        user2.setUserName("def_def");
        Set user2Aliases = new LinkedHashSet(); // use LinkedHashSet for stable ordering
        user2Aliases.add("456");
        user2.setAliases(user2Aliases);

        User user3 = new User();
        user3.setUserName("xyz");
        Set user3Aliases = new LinkedHashSet(); // use LinkedHashSet for stable ordering
        user3Aliases.add("789");
        user3.setAliases(user3Aliases);

        IMocksControl coreContextControl = EasyMock.createNiceControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);

        coreContext.loadUserByAlias("456");
        EasyMock.expectLastCall().andReturn(user2).atLeastOnce();
        coreContextControl.replay();

        phone.setCoreContext(coreContext);

        IMocksControl phoneContextControl = EasyMock.createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);

        phoneContext.getSpeedDial(phone);
        phoneContextControl.andReturn(sp).anyTimes();
        phoneContextControl.replay();

        phone.setPhoneContext(phoneContext);
        phone.generateProfiles(location);

        InputStream expectedProfile = getClass().getResourceAsStream("speed-dials");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        assertEquals(expected, location.toString());
        phoneContextControl.verify();
    }

    public void testGenerateProfilesForPhoneBook() throws Exception {

        Nortel12x0Phone phone = new Nortel12x0Phone();
        PhoneModel model = new PhoneModel("nortel12x0");
        model.setLabel("Nortel IP Phone 1230");
        model.setModelDir("nortel12x0");
        model.setProfileTemplate("nortel12x0/nortel12x0.vm");
        phone.setModel(model);

        PhoneTestDriver.supplyTestData(phone);
        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        List< ? extends PhonebookEntry> phonebook = Arrays.asList(new DummyEntry("001"), new DummyEntry("003"),
                new DummyEntry("005"));

        IMocksControl phoneContextControl = EasyMock.createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);

        phoneContext.getPhonebookEntries(phone);
        phoneContextControl.andReturn(phonebook).anyTimes();
        phoneContextControl.replay();

        phone.setPhoneContext(phoneContext);
        phone.generateProfiles(location);

        InputStream expectedProfile = getClass().getResourceAsStream("phonebook");
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();
        assertEquals(expected, location.toString());

        phoneContextControl.verify();
    }

    public void testNortel12x0ContextEmpty() {

        Nortel12x0Phone phone = getPhoneWithMockPhoneContextForSpecialUser();

        Nortel12x0Context sc = new Nortel12x0Phone.Nortel12x0Context(phone, null, null, m_emptyPhonebook, null);
        String[] numbers = (String[]) sc.getContext().get("speedDialInfo");

        assertEquals(0, numbers.length);
    }

    public void testNortel12x0MaxNumOfSpeedDials() {

        SpeedDial maxSd = createSpeedDial(220);

        Nortel12x0Phone phone = getPhoneWithMockPhoneContextForSpecialUser();

        Nortel12x0Context sc = new Nortel12x0Phone.Nortel12x0Context(phone, maxSd, null, m_emptyPhonebook, null);
        String[] numbers = (String[]) sc.getContext().get("speedDialInfo");
        assertEquals(200 * 3, numbers.length);
    }

    public void testNortel12x0ActualNumOfSpeedDialsGenerated() {

        SpeedDial smallSd = createSpeedDial(5);
        SpeedDial largeSd = createSpeedDial(100);

        Nortel12x0Phone phone = getPhoneWithMockPhoneContextForSpecialUser();

        Nortel12x0Context sc = new Nortel12x0Phone.Nortel12x0Context(phone, smallSd, null, m_emptyPhonebook, null);
        String[] numbers = (String[]) sc.getContext().get("speedDialInfo");
        assertEquals(5 * 3, numbers.length);

        sc = new Nortel12x0Phone.Nortel12x0Context(phone, largeSd, null, m_emptyPhonebook, null);
        numbers = (String[]) sc.getContext().get("speedDialInfo");
        assertEquals(100 * 3, numbers.length);
    }

    public void testNortel12x0MaxNumOfPhonebookEntries() {

        Collection<PhonebookEntry> phonebook = new ArrayList<PhonebookEntry>();

        for (int i = 0; i < 220; i++) {
            phonebook.add(new DummyEntry(Integer.toString(i)));
        }

        Nortel12x0Phone phone = getPhoneWithMockPhoneContextForSpecialUser();

        Nortel12x0Context sc = new Nortel12x0Phone.Nortel12x0Context(phone, null, null, phonebook, null);
        Collection< ? > maxEntries = (Collection< ? >) sc.getContext().get("phoneBook");
        assertEquals(200, maxEntries.size());
    }

    public void testNortel12x0ActualNumOfPhoneBookEntriesConfigured() {

        Collection<PhonebookEntry> phonebook = new ArrayList<PhonebookEntry>();

        for (int i = 0; i < 120; i++) {
            phonebook.add(new DummyEntry(Integer.toString(i)));
        }

        Nortel12x0Phone phone = getPhoneWithMockPhoneContextForSpecialUser();

        Nortel12x0Context sc = new Nortel12x0Phone.Nortel12x0Context(phone, null, null, phonebook, null);
        Collection< ? > maxEntries = (Collection< ? >) sc.getContext().get("phoneBook");
        assertEquals(120, maxEntries.size());
    }

    private SpeedDial createSpeedDial(int size) {

        List<Button> buttons = new ArrayList<Button>();
        for (int i = 0; i < size; i++) {
            buttons.add(new Button("test", Integer.toString(i)));
        }
        SpeedDial sd = new SpeedDial();
        sd.setButtons(buttons);
        return sd;
    }

    static class DummyEntry extends PhonebookEntry {

        private final String entry;

        public DummyEntry(String param) {
            entry = param;
        }

        @Override
        public String getFirstName() {
            return "bookFirstName" + entry;
        }

        @Override
        public String getLastName() {
            return "bookLastName" + entry;
        }

        @Override
        public String getNumber() {
            return "bookNumber" + entry;
        }

        @Override
        public AddressBookEntry getAddressBookEntry() {
            return null;
        }
    }

}
