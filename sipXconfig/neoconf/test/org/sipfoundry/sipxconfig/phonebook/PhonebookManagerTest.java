/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.io.File;
import java.util.Collection;
import java.util.Collections;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.CsvFileFormatError;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.PhoneEntryComparator;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManagerImpl.StringArrayPhonebookEntry;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.test.TestUtil;

public class PhonebookManagerTest extends TestCase {
    
    public void testMkdirs() {
        String dir = TestHelper.getTestDirectory() + File.separator + System.currentTimeMillis(); 
        assertFalse(new File(dir).exists());
        PhonebookManagerImpl.mkdirs(dir);
        assertTrue(new File(dir).exists());
    }
    
    public void testGetEmptyPhonebookRows() {
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        assertEquals(0, context.getRows(new Phonebook()).size());
    }
    
    public void testGetRows() {       
        Phonebook phonebook = new Phonebook();
        Group group = new Group();
        User user = new User();
        user.setFirstName("Tweety");
        user.setLastName("Bird");
        user.setUserName("tbird");
        phonebook.setMembers(Collections.singleton(group));
        Collection<User> users = Collections.singleton(user);
        IMocksControl coreContextControl = EasyMock.createControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);
        coreContext.getGroupMembers(group);
        coreContextControl.andReturn(users);
        coreContextControl.replay();
        
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        context.setCoreContext(coreContext);              
        Collection<PhonebookEntry> entries = context.getRows(phonebook);
        assertEquals(1, entries.size());
        PhonebookEntry entry = entries.iterator().next(); 
        assertEquals("Tweety", entry.getFirstName());
        
        coreContextControl.verify();
    }
    
    public void testPhoneEntryComparator() {
        PhoneEntryComparator c = new PhoneEntryComparator();
        IMocksControl phonebookEntryControl = EasyMock.createControl();
        PhonebookEntry a = phonebookEntryControl.createMock(PhonebookEntry.class);
        PhonebookEntry b = phonebookEntryControl.createMock(PhonebookEntry.class);

        a.getLastName();
        phonebookEntryControl.andReturn("Avocet");
        b.getLastName();
        phonebookEntryControl.andReturn("Vireo");
        phonebookEntryControl.replay();

        assertTrue(c.compare(a, b) < 0);
        
        phonebookEntryControl.reset();
        a.getLastName();
        phonebookEntryControl.andReturn("Avocet");
        a.getFirstName();
        phonebookEntryControl.andReturn("Southern");
        b.getLastName();
        phonebookEntryControl.andReturn("Avocet");
        b.getFirstName();
        phonebookEntryControl.andReturn("Northern");
        phonebookEntryControl.replay();
        
        assertTrue(c.compare(a, b) > 0);

        phonebookEntryControl.reset();
        a.getLastName();
        phonebookEntryControl.andReturn("Avocet");
        a.getFirstName();
        phonebookEntryControl.andReturn("Northern");
        a.getNumber();
        phonebookEntryControl.andReturn("1234");
        
        b.getLastName();
        phonebookEntryControl.andReturn("Avocet");
        b.getFirstName();
        phonebookEntryControl.andReturn("Northern");
        b.getNumber();
        phonebookEntryControl.andReturn("abc");
        phonebookEntryControl.replay();
        
        assertTrue(c.compare(a, b) < 0);

        phonebookEntryControl.verify();        
    }
    
    public void testStringArrayPhonebookEntry() {
        try {
            new StringArrayPhonebookEntry(new String[2]);
            fail();
        } catch (CsvFileFormatError e) {
            assertTrue(true);
        }
    }

    public void testStringArrayPhonebookEntryOk() {
        new StringArrayPhonebookEntry(new String[3]);
    }
    
    
    public void testGetCsvFile() {
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        String dir = TestUtil.getTestSourceDirectory(getClass());
        context.setExternalUsersDirectory(dir);
        
        Phonebook phonebook = new Phonebook();
        phonebook.setMembersCsvFilename("bogus.csv");
        try {
            context.getRows(phonebook);
            fail();
        } catch (RuntimeException expected) {
            assertTrue(true);
        }
        
        phonebook.setMembersCsvFilename("phonebook.csv");
        Collection<PhonebookEntry> entries = context.getRows(phonebook);
        assertEquals(1, entries.size());
    }
}
