/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */

package org.sipfoundry.sipxconfig.phone.snom;

import static org.easymock.EasyMock.createNiceControl;
import static org.easymock.EasyMock.createNiceMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phonebook.AddressBookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;
import org.sipfoundry.sipxconfig.speeddial.SpeedDialManager;
import org.sipfoundry.sipxconfig.test.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class SnomTest extends TestCase {
    private final Collection<PhonebookEntry> m_emptyPhonebook = Collections.<PhonebookEntry> emptyList();

    public void testGenerateProfiles360() throws Exception {
        PhoneModel model = new PhoneModel("snom");
        model.setModelId("snom360");
        model.setLabel("Snom 360");
        model.setModelDir("snom");
        model.setSupportedFeatures(Collections.singleton("blf"));
        model.setProfileTemplate("snom/snom.vm");

        SnomPhone phone = preparePhone(model);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone, TestHelper.getEtcDir());

        phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-snom-360.xml"));

        assertEquals(expected, location.toString());
    }

    public void testGenerateProfiles320() throws Exception {
        PhoneModel model = new PhoneModel("snom");
        model.setModelId("snom320");
        model.setLabel("Snom 320");
        model.setModelDir("snom");
        model.setProfileTemplate("snom/snom.vm");

        SnomPhone phone = preparePhone(model);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone, TestHelper.getEtcDir());

        phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-snom-320.xml"));

        assertEquals(expected, location.toString());
    }

    public void testGenerateProfiles_m3() throws Exception {
        PhoneModel model = new PhoneModel("snom-m3");
        model.setMaxLineCount(8);
        model.setModelId("snomM3");
        model.setLabel("Snom m3");
        model.setModelDir("snom-m3");
        model.setProfileTemplate("snom-m3/snom_m3.vm");

        SnomM3Phone phone = prepareM3Phone(model);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone, TestHelper.getEtcDir());

        phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass().getResourceAsStream("expected-snom-m3.cfg"));

        assertEquals(expected, location.toString());

    }

    private SnomPhone preparePhone(PhoneModel model) {
        User user = new User();
        user.setUserName("juser");
        user.setFirstName("Joe");
        user.setLastName("User");
        user.setSipPassword("1234");
        user.setIsShared(false);

        SpeedDial sp = new SpeedDial();
        sp.setUser(user);
        Button button = new Button("Yogi", "yogi@example.com");
        button.setBlf(true);
        sp.setButtons(Collections.singletonList(button));

        SpeedDialManager speedDialManager = createNiceMock(SpeedDialManager.class);
        speedDialManager.getSpeedDialForUserId(user.getId(), false);
        expectLastCall().andReturn(sp);
        replay(speedDialManager);

        SnomPhone phone = new SnomPhone();
        phone.setSerialNumber("abc123");

        phone.setModel(model);
        phone.setSpeedDialManager(speedDialManager);
        PhoneTestDriver.supplyTestData(phone, Arrays.asList(user));
        return phone;
    }

    private SnomM3Phone prepareM3Phone(PhoneModel model) {
        User user = new User();
        user.setUserName("juser");
        user.setFirstName("Joe");
        user.setLastName("User");
        user.setSipPassword("1234");
        user.setIsShared(false);

        SnomM3Phone phone = new SnomM3Phone();
        phone.setSerialNumber("abc123");

        phone.setModel(model);
        PhoneTestDriver.supplyTestData(phone, Arrays.asList(user));
        return phone;
    }

    public void testGenerateProfilesWithSpeedDial() throws Exception {
        SnomPhone phone = new SnomPhone();
        phone.setSerialNumber("abc123");
        PhoneModel model = new PhoneModel("snom");
        model.setLabel("Snom 360");
        model.setModelDir("snom");
        model.setProfileTemplate("snom/snom.vm");

        SpeedDial sp = new SpeedDial();
        sp.setButtons(Arrays.asList(new Button("Yogi", "yogi@example.com"), new Button("Daffy Duck", "213")));

        phone.setModel(model);

        IMocksControl phoneContextControl = createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);
        phoneContext.getSpeedDial(phone);
        phoneContextControl.andReturn(sp).anyTimes();
        phoneContext.getPhonebookEntries(phone);
        phoneContextControl.andReturn(m_emptyPhonebook).anyTimes();
        phoneContextControl.replay();

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone, TestHelper.getEtcDir());

        phone.setPhoneContext(phoneContext);
        phone.generateProfiles(location);

        String profile = location.toString();

        assertTrue(profile.contains("<speed idx=\"0\" perm=\"R\">yogi@example.com</speed>"));
        assertTrue(profile.contains("<speed idx=\"1\" perm=\"R\">213</speed>"));

        phoneContextControl.verify();
    }

    public void testGenerateProfilesWithPhoneBook() throws Exception {
        SnomPhone phone = new SnomPhone();
        phone.setSerialNumber("abc123");
        PhoneModel model = new PhoneModel("snom");
        model.setLabel("Snom 360");
        model.setModelDir("snom");
        model.setProfileTemplate("snom/snom.vm");

        PhonebookEntry entryWithSpecialChars = new PhonebookEntry();
        entryWithSpecialChars.setFirstName("&first");
        entryWithSpecialChars.setLastName("<last>");
        List< ? extends PhonebookEntry> phonebook = Arrays.asList(new DummyEntry("1"), new DummyEntry("3"),
                new DummyEntry("5"), entryWithSpecialChars);

        phone.setModel(model);

        IMocksControl phoneContextControl = createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);
        phoneContext.getPhonebookEntries(phone);
        phoneContextControl.andReturn(phonebook).anyTimes();
        phoneContext.getSpeedDial(phone);
        phoneContextControl.andReturn(null).anyTimes();
        phoneContextControl.replay();

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone, TestHelper.getEtcDir());

        phone.setPhoneContext(phoneContext);
        phone.generateProfiles(location);

        String profile = location.toString();

        assertTrue(profile
                .contains("<item context=\"active\" type=\"none\" index=\"0\">\n			<name>first1 last1</name>\n			<number>number1</number>\n			<search></search>\n		</item>"));

        assertTrue(profile
                .contains("<item context=\"active\" type=\"none\" index=\"1\">\n			<name>first3 last3</name>\n			<number>number3</number>\n			<search></search>\n		</item>"));
        assertTrue(profile
                .contains("<item context=\"active\" type=\"none\" index=\"2\">\n			<name>first5 last5</name>\n			<number>number5</number>\n			<search></search>\n		</item>"));
        assertTrue(profile.contains("<name>&amp;first &lt;last&gt;</name>"));

        phoneContextControl.verify();
    }

    public void testGetProfileName() {
        Phone phone = new SnomPhone();
        phone.setSerialNumber("abc123");
        assertEquals("ABC123.xml", phone.getProfileFilename());
    }

    public void testSnomContextEmpty() {
        SnomPhone phone = new SnomPhone();

        SnomProfileContext sc = new SnomProfileContext(phone, null, m_emptyPhonebook, null);
        String[] numbers = (String[]) sc.getContext().get("speedDial");

        assertEquals(0, numbers.length);
    }

    public void testSnomContext() {
        SpeedDial smallSd = createSpeedDial(5);
        SpeedDial largeSd = createSpeedDial(40);

        SnomPhone phone = new SnomPhone();

        SnomProfileContext sc = new SnomProfileContext(phone, smallSd, m_emptyPhonebook, null);
        String[] numbers = (String[]) sc.getContext().get("speedDial");
        assertEquals(5, numbers.length);
        for (int i = 0; i < numbers.length; i++) {
            assertEquals(Integer.toString(i), numbers[i]);
        }

        sc = new SnomProfileContext(phone, largeSd, m_emptyPhonebook, null);
        numbers = (String[]) sc.getContext().get("speedDial");
        assertEquals(33, numbers.length);
        for (int i = 0; i < numbers.length; i++) {
            assertEquals(Integer.toString(i), numbers[i]);
        }
    }

    public void testSnomContextLargePhonebook() {
        Collection<PhonebookEntry> phonebook = new ArrayList<PhonebookEntry>();
        for (int i = 0; i < 120; i++) {
            phonebook.add(new DummyEntry(Integer.toString(i)));
        }
        SnomPhone phone = new SnomPhone();

        SnomProfileContext sc = new SnomProfileContext(phone, null, phonebook, null);
        Collection< ? > trimmed = (Collection< ? >) sc.getContext().get("phoneBook");
        assertEquals(100, trimmed.size());
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
        private final String m_id;

        public DummyEntry(String id) {
            m_id = id;
        }

        public String getFirstName() {
            return "first" + m_id;
        }

        public String getLastName() {
            return "last" + m_id;
        }

        public String getNumber() {
            return "number" + m_id;
        }

        public AddressBookEntry getAddressBookEntry() {
            return null;
        }
    }
}
