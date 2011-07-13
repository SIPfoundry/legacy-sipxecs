package org.sipfoundry.sipxconfig.phone.karel_ip11x;

import java.io.IOException;
import java.io.InputStream;
import java.util.Collections;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class KarelIP11xPhonebookTest extends TestCase {
    private static final String EXPECTED_PHONEBOOK_NO_RECORDS = "expected-phonebook.0";
    private static final String EXPECTED_PHONEBOOK_1_RECORD = "expected-phonebook.1";
    private ProfileGenerator m_pg;
    private MemoryProfileLocation m_location;

    protected void setUp() {
        m_location = new MemoryProfileLocation();
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }

    public void testPhonebookNoRecords() throws IOException {
        KarelIP11xPhonebook book = new KarelIP11xPhonebook(Collections.EMPTY_LIST);

        m_pg.generate(m_location, book, null, "phonebook");

        String actual = IOUtils.toString(m_location.getReader());

        InputStream expectedProfile = getClass().getResourceAsStream(EXPECTED_PHONEBOOK_NO_RECORDS);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, actual);
    }

    public void testPhonebookOneRecord() throws IOException {
        PhonebookEntry phonebookEntry = new PhonebookEntry();
        phonebookEntry.setFirstName("Eda");
        phonebookEntry.setLastName("Ercan");
        phonebookEntry.setNumber("303");

        KarelIP11xPhonebook book = new KarelIP11xPhonebook(Collections.singleton(phonebookEntry));

        m_pg.generate(m_location, book, null, "phonebook");

        String actual = IOUtils.toString(m_location.getReader());

        InputStream expectedProfile = getClass().getResourceAsStream(EXPECTED_PHONEBOOK_1_RECORD);
        String expected = IOUtils.toString(expectedProfile);
        expectedProfile.close();

        assertEquals(expected, actual);
    }

}
