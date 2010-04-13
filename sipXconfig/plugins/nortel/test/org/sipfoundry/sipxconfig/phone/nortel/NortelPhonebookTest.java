/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.phone.nortel;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.junit.Before;
import org.junit.Test;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.device.MemoryProfileLocation;
import org.sipfoundry.sipxconfig.device.ProfileGenerator;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.PhoneTestDriver;
import org.sipfoundry.sipxconfig.phone.nortel.NortelPhonebook.NortelPhonebookEntry;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;


/**
 * Tests the Contents of the Phonebook profile
 * Line count for each user
 */
public class NortelPhonebookTest extends TestCase{
    private CoreContext m_coreContext;

    private ProfileGenerator m_pg;
    private MemoryProfileLocation m_location;

    @Before
    public void setUp() throws Exception {
        m_location = new MemoryProfileLocation();
        VelocityProfileGenerator pg = new VelocityProfileGenerator();
        pg.setVelocityEngine(TestHelper.getVelocityEngine());
        m_pg = pg;
    }
    /**
     * sets the values for phonebook entries
     * verfies the contents of the phonebook entries generated inside phonebook
     * Be careful because NortelPhonebook doesnt have the exact
     * phonebook entreis , it has a different format and NortelPhonebookEntry instead of PhonebookEntry
     * @throws Exception
     * for correct format have a look at the *.ab profile generated at
     * /home/sipxchange/sipx/BUILD/sipXconfig/web/test-results/profile/tftproot
     * If the file doesnt exist go to the UI and generate the profile first
     * [contact]
     * address sip:juser@sipfoundry.org
     * buddy 0
     * group
     * edited 0
     * nickname 1111
     * [contact]
     * address sip:juser1@sipfoundry.org
     * buddy 0
     * group
     * edited 0
     * nickname 2222
     * [group]
     * name
     * [version]
     * id 1
     * merge overwrite
     */

    @Test
    public void testNortelPhonebook() throws Exception {
        Integer maxUsers = 300;
        String userName = "juser";
        PhoneModel nortelModel = new PhoneModel("nortel");
        NortelPhone phone = new NortelPhone();
        phone.setModel(nortelModel);
        // sets test data for the phone and creates profiles
        PhoneTestDriver.supplyTestData(phone, true);
        Collection<NortelPhonebookEntry> nortelPhonebook = new ArrayList<NortelPhonebookEntry>();

        for(Integer i = 0;i < maxUsers; i++)
        {

            NortelPhonebookEntry nortelPhonebookEntry=new NortelPhonebookEntry();
            nortelPhonebookEntry.setNumber(i.toString());
            nortelPhonebookEntry.setSipUri(userName+i+"@sipfoundry.org");
            nortelPhonebookEntry.setNickName(userName+i);
            nortelPhonebook.add(nortelPhonebookEntry);
        }

        IMocksControl coreContextControl = EasyMock.createControl();
        m_coreContext = coreContextControl.createMock(CoreContext.class);
        NortelPhonebook book = new NortelPhonebook(null, m_coreContext);
        book.setPhonebook(nortelPhonebook);
        assertNotNull(book);
        m_pg.generate(m_location,book, null, "phonebook.ab");

        List<String> list = IOUtils.readLines(m_location.getReader());


        // 6 lines per user so numOfUsers * 6
        // list.size()-5 because all the tags after [group] are not generated for each user
        // they are same for all the users
        Integer numOfUsers=nortelPhonebook.size();
        Integer phonebooklen=list.size();
        assertEquals(numOfUsers * 6, phonebooklen-4);

        Integer k=0;
        for(Integer j=0; j<numOfUsers*6;j=j+6,k++)
        {
            assertEquals("[contact]", list.get(j));
            assertEquals("address "+(userName+k)+"@sipfoundry.org", list.get(j+1));
            assertEquals("buddy "+0, list.get(j+2));
            assertEquals("group", list.get(j+3));
            assertEquals("edited "+0, list.get(j+4));
            assertEquals("nickname "+(userName+k), list.get(j+5));
        }
        assertEquals("[group]", list.get(phonebooklen-4));
        assertEquals("name", list.get(phonebooklen-3));
        assertEquals("[version]", list.get(phonebooklen-2));
        assertEquals("id 1", list.get(phonebooklen-1));
     }
}
