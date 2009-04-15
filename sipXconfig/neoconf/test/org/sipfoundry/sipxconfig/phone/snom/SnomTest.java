package org.sipfoundry.sipxconfig.phone.snom;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.snom.SnomPhone.SnomContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.speeddial.Button;
import org.sipfoundry.sipxconfig.speeddial.SpeedDial;

public class SnomTest extends TestCase {
    private final Collection<PhonebookEntry> m_emptyPhonebook = Collections
            .<PhonebookEntry> emptyList();

    public void testGenerateProfiles() throws Exception {
        SnomPhone phone = new SnomPhone();
        PhoneModel model = new PhoneModel("snom");
        model.setLabel("Snom 360");
        model.setModelDir("snom");
        model.setProfileTemplate("snom/snom.vm");

        phone.setModel(model);
        PhoneTestDriver.supplyTestData(phone);

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        phone.generateProfiles(location);
        String expected = IOUtils.toString(this.getClass()
                .getResourceAsStream("expected-360.cfg"));

        assertEquals(expected, location.toString());
    }

    public void testGenerateProfilesWithSpeedDial() throws Exception {
        SnomPhone phone = new SnomPhone();
        PhoneModel model = new PhoneModel("snom");
        model.setLabel("Snom 360");
        model.setModelDir("snom");
        model.setProfileTemplate("snom/snom.vm");

        SpeedDial sp = new SpeedDial();
        sp.setButtons(Arrays.asList(new Button("Yogi", "yogi@example.com"), new Button(
                "Daffy Duck", "213")));

        phone.setModel(model);

        IMocksControl phoneContextControl = EasyMock.createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);
        phoneContext.getSpeedDial(phone);
        phoneContextControl.andReturn(sp).anyTimes();
        phoneContext.getPhonebookEntries(phone);
        phoneContextControl.andReturn(m_emptyPhonebook).anyTimes();
        phoneContextControl.replay();

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        phone.setPhoneContext(phoneContext);
        phone.generateProfiles(location);

        String profile = location.toString();

        assertTrue(profile.contains("speed0!: yogi@example.com"));
        assertTrue(profile.contains("speed1!: 213"));
        assertFalse(profile.contains("speed2"));

        phoneContextControl.verify();
    }

    public void testGenerateProfilesWithPhoneBook() throws Exception {
        SnomPhone phone = new SnomPhone();
        PhoneModel model = new PhoneModel("snom");
        model.setLabel("Snom 360");
        model.setModelDir("snom");
        model.setProfileTemplate("snom/snom.vm");

        List< ? extends PhonebookEntry> phonebook = Arrays.asList(new DummyEntry("1"),
                new DummyEntry("3"), new DummyEntry("5"));

        phone.setModel(model);

        IMocksControl phoneContextControl = EasyMock.createNiceControl();
        PhoneContext phoneContext = phoneContextControl.createMock(PhoneContext.class);
        PhoneTestDriver.supplyVitalTestData(phoneContextControl, phoneContext, phone);
        phoneContext.getPhonebookEntries(phone);
        phoneContextControl.andReturn(phonebook).anyTimes();
        phoneContext.getSpeedDial(phone);
        phoneContextControl.andReturn(null).anyTimes();
        phoneContextControl.replay();

        MemoryProfileLocation location = TestHelper.setVelocityProfileGenerator(phone);

        phone.setPhoneContext(phoneContext);
        phone.generateProfiles(location);

        String profile = location.toString();

        assertTrue(profile.contains("tn_0!: first1 last1"));
        assertTrue(profile.contains("tn_1!: first3 last3"));
        assertTrue(profile.contains("tn_2!: first5 last5"));
        assertTrue(profile.contains("tu_0!: number1"));
        assertTrue(profile.contains("tu_1!: number3"));
        assertTrue(profile.contains("tu_2!: number5"));
        assertFalse(profile.contains("tn_3"));
        assertFalse(profile.contains("tu_3"));

        phoneContextControl.verify();
    }

    public void testGetProfileName() {
        Phone phone = new SnomPhone();
        // it can be called without serial number
        assertEquals("snom.htm", phone.getProfileFilename());
        phone.setSerialNumber("abc123");
        assertEquals("ABC123.htm", phone.getProfileFilename());
    }

    public void testSnomContextEmpty() {
        SnomPhone phone = new SnomPhone();

        SnomContext sc = new SnomPhone.SnomContext(phone, null, m_emptyPhonebook, null);
        String[] numbers = (String[]) sc.getContext().get("speedDial");

        assertEquals(0, numbers.length);
    }

    public void testSnomContext() {
        SpeedDial smallSd = createSpeedDial(5);
        SpeedDial largeSd = createSpeedDial(40);

        SnomPhone phone = new SnomPhone();

        SnomContext sc = new SnomPhone.SnomContext(phone, smallSd, m_emptyPhonebook, null);
        String[] numbers = (String[]) sc.getContext().get("speedDial");
        assertEquals(5, numbers.length);
        for (int i = 0; i < numbers.length; i++) {
            assertEquals(Integer.toString(i), numbers[i]);
        }

        sc = new SnomPhone.SnomContext(phone, largeSd, m_emptyPhonebook, null);
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

        SnomContext sc = new SnomPhone.SnomContext(phone, null, phonebook, null);
        Collection<?> trimmed = (Collection<?>) sc.getContext().get("phoneBook");
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

    static class DummyEntry implements PhonebookEntry {
        private String m_id;

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

    }
}
