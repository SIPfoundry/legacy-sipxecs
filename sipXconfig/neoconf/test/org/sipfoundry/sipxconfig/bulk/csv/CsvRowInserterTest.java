/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.File;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.TestPhone;
import org.sipfoundry.sipxconfig.phone.TestPhoneModel;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public class CsvRowInserterTest extends TestCase {
    public void testUserFromRow() {
        User bongo = new User();
        bongo.setUserName("bongo");
        bongo.setFirstName("Ringo");

        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);
        coreContext.loadUserByUserName("bongo");
        coreContextCtrl.andReturn(bongo);
        coreContext.loadUserByUserName("kuku");
        coreContextCtrl.andReturn(null);
        coreContext.getAuthorizationRealm();
        coreContextCtrl.andReturn("sipfoundry.org").times(2);

        coreContextCtrl.replay();

        String[] userRow1 = new String[] {
            "bongo", "1234", "abcdef", "", "Star", ""
        };

        String[] userRow2 = new String[] {
            "kuku", "1234", "abcdef", "John", "Lennon", "jlennon, 121212"
        };

        CsvRowInserter impl = new CsvRowInserter();
        impl.setCoreContext(coreContext);

        User user1 = impl.userFromRow(userRow1);
        assertEquals("bongo", user1.getUserName());
        assertEquals("Ringo", user1.getFirstName());
        assertEquals("Star", user1.getLastName());
        assertEquals("abcdef", user1.getSipPassword());
        assertEquals(32, user1.getPintoken().length());

        User user2 = impl.userFromRow(userRow2);
        assertEquals("kuku", user2.getUserName());
        assertEquals("John", user2.getFirstName());
        assertEquals("Lennon", user2.getLastName());
        assertEquals("abcdef", user2.getSipPassword());
        assertEquals(32, user2.getPintoken().length());
        assertEquals("jlennon, 121212", user2.getAliasesString());

        coreContextCtrl.verify();
    }

    public void testCheckRowData() {
        CsvRowInserter impl = new CsvRowInserter();
        String[] row = {
            "kuku", "", "", "", "", "", "", "", "001122334466", "polycom300", "yellow phone", ""
        };
        assertTrue(impl.checkRowData(row));
        String[] rowShort = {
            "kuku", "", "", "", "", "", "", "", "001122334466", "polycom300", "yellow phone"
        };
        assertTrue(impl.checkRowData(rowShort));
        row[Index.USERNAME.getValue()] = "";
        assertFalse(impl.checkRowData(row));
        row[Index.SERIAL_NUMBER.getValue()] = "";
        assertFalse(impl.checkRowData(row));
        row[Index.USERNAME.getValue()] = "kuku";
        assertFalse(impl.checkRowData(row));
    }

    public void testPhoneFromRowUpdate() {
        final String[] phoneRow = new String[] {
            "", "", "", "", "", "", "", "", "001122334466", "polycom300", "yellow phone", ""
        };

        Integer phoneId = new Integer(5);
        Phone phone = new TestPhone();
        PhoneModel model = new TestPhoneModel();

        phone.setSerialNumber("001122334466");
        phone.setDescription("old description");

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);

        phoneContext.getPhoneIdBySerialNumber("001122334466");
        phoneContextCtrl.andReturn(phoneId);
        phoneContext.loadPhone(phoneId);
        phoneContextCtrl.andReturn(phone);

        phoneContextCtrl.replay();

        IMocksControl phoneModelSourceControl = EasyMock.createControl();
        ModelSource<PhoneModel> phoneModelSource = phoneModelSourceControl
                .createMock(ModelSource.class);
        phoneModelSource.getModel("polycom300");
        phoneModelSourceControl.andReturn(model);

        phoneModelSourceControl.replay();

        CsvRowInserter impl = new CsvRowInserter();
        impl.setPhoneContext(phoneContext);
        impl.setPhoneModelSource(phoneModelSource);

        // update existing phone
        Phone phone1 = impl.phoneFromRow(phoneRow);
        assertEquals("old description", phone1.getDescription());
        assertEquals("001122334466", phone1.getSerialNumber());

        phoneContextCtrl.verify();
        phoneModelSourceControl.verify();
    }

    public void testPhoneFromRowNew() {
        final String[] phoneRow1 = new String[] {
            "", "", "", "", "", "", "", "", "001122334455", "testPhoneModel", "yellow phone",
            "phone in John room"
        };

        Phone phone = new TestPhone();
        PhoneModel model = new TestPhoneModel();
        phone.setDescription("old description");

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);

        phoneContext.getPhoneIdBySerialNumber("001122334455");
        phoneContextCtrl.andReturn(null);
        phoneContext.newPhone(model);
        phoneContextCtrl.andReturn(phone);

        phoneContextCtrl.replay();

        IMocksControl phoneModelSourceControl = EasyMock.createControl();
        ModelSource<PhoneModel> phoneModelSource = phoneModelSourceControl
                .createMock(ModelSource.class);
        phoneModelSource.getModel(model.getModelId());
        phoneModelSourceControl.andReturn(model);

        phoneModelSourceControl.replay();

        CsvRowInserter impl = new CsvRowInserter();
        impl.setPhoneContext(phoneContext);
        impl.setPhoneModelSource(phoneModelSource);

        // new phone
        Phone phone1 = impl.phoneFromRow(phoneRow1);
        assertEquals("phone in John room", phone1.getDescription());
        assertEquals("001122334455", phone1.getSerialNumber());

        phoneModelSourceControl.verify();
        phoneContextCtrl.verify();
    }

    public void testPhoneFromRowSpaces() {
        final String[] phoneRow1 = new String[] {
            "", "", "", "", "", "", "", "", "001122334455", "testPhoneModel", "yellow phone",
            "phone in John room"
        };

        Phone phone = new TestPhone();
        PhoneModel model = new TestPhoneModel();
        phone.setDescription("old description");

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);

        phoneContext.getPhoneIdBySerialNumber("001122334455");
        phoneContextCtrl.andReturn(null);
        phoneContext.newPhone(model);
        phoneContextCtrl.andReturn(phone);

        phoneContextCtrl.replay();

        IMocksControl phoneModelSourceControl = EasyMock.createControl();
        ModelSource<PhoneModel> phoneModelSource = phoneModelSourceControl
                .createMock(ModelSource.class);
        phoneModelSource.getModel(model.getModelId());
        phoneModelSourceControl.andReturn(model);

        phoneModelSourceControl.replay();

        CsvRowInserter impl = new CsvRowInserter();
        impl.setPhoneModelSource(phoneModelSource);
        impl.setPhoneContext(phoneContext);

        // new phone
        Phone phone1 = impl.phoneFromRow(phoneRow1);
        assertEquals("phone in John room", phone1.getDescription());
        assertEquals("001122334455", phone1.getSerialNumber());

        phoneModelSourceControl.verify();
        phoneContextCtrl.verify();
    }

    public void testUpdateMailboxPreferences() {
        User user = new User();
        user.setUserName("kuku");
        Mailbox mailbox = new Mailbox(new File("."), "kuku");
        MailboxPreferences expected = new MailboxPreferences();

        IMocksControl mailboxManagerControl = EasyMock.createStrictControl();
        MailboxManager mailboxManager = mailboxManagerControl.createMock(MailboxManager.class);
        
        mailboxManager.isEnabled();
        mailboxManagerControl.andReturn(true);
        mailboxManager.deleteMailbox("kuku");
        mailboxManager.getMailbox("kuku");
        mailboxManagerControl.andReturn(mailbox);
        mailboxManager.loadMailboxPreferences(mailbox);
        mailboxManagerControl.andReturn(expected);
        mailboxManager.saveMailboxPreferences(mailbox, expected);

        mailboxManager.isEnabled();
        mailboxManagerControl.andReturn(true);
        mailboxManager.getMailbox("kuku");
        mailboxManagerControl.andReturn(mailbox);
        mailboxManager.loadMailboxPreferences(mailbox);
        mailboxManagerControl.andReturn(expected);
        mailboxManager.saveMailboxPreferences(mailbox, expected);
        
        mailboxManager.isEnabled();
        mailboxManagerControl.andReturn(false);
        mailboxManagerControl.replay();

        CsvRowInserter impl = new CsvRowInserter();
        impl.setMailboxManager(mailboxManager);
        
        impl.updateMailboxPreferences(user, "jlennon@example.com", true);

        impl.updateMailboxPreferences(user, "jlennon@example.com", false);

        impl.updateMailboxPreferences(user, "jlennon@example.com", true);        
        mailboxManagerControl.verify();
    }

    public void testAddLine() {
        CsvRowInserter impl = new CsvRowInserter();
        Phone phone = new TestPhone();
        phone.setModel(new PhoneModel("test"));
        User user = new User();
        Line expected = impl.addLine(phone, user);
        Line actual = impl.addLine(phone, user);
        assertSame(expected, actual);

        User newuser = new TestUser(1);
        Line newline = impl.addLine(phone, newuser);
        assertNotSame(expected, newline);
    }

    private class TestUser extends User {
        private Integer m_id;

        TestUser(Integer id) {
            m_id = id;
        }

        public Integer getId() {
            return m_id;
        }
    }
}
