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

import junit.framework.TestCase;
import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.TestPhone;
import org.sipfoundry.sipxconfig.phone.TestPhoneModel;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class CsvRowInserterTest extends TestCase {
    public void testUserFromRow() {
        User bongo = new User();
        bongo.setUserName("bongo");
        bongo.setFirstName("Ringo");

        DomainManager domainManager = createMock(DomainManager.class);
        domainManager.getAuthorizationRealm();
        expectLastCall().andReturn("sipfoundry.org").times(2);

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUserByUserName("bongo");
        expectLastCall().andReturn(bongo);
        coreContext.loadUserByUserName("kuku");
        expectLastCall().andReturn(null);

        replay(domainManager, coreContext);

        String[] userRow1 = new String[] {
            "bongo", "1234", "abcdef", "", "Star", "","","","","","",""," im_id"
        };

        String[] userRow2 = new String[] {
            "kuku", "1234", "abcdef", " John", " Lennon  ", "jlennon, 121212"
        };

        CsvRowInserter impl = new CsvRowInserter();
        impl.setCoreContext(coreContext);
        impl.setDomainManager(domainManager);

        User user1 = impl.userFromRow(userRow1);
        assertEquals("bongo", user1.getUserName());
        assertEquals("Ringo", user1.getFirstName());
        assertEquals("Star", user1.getLastName());
        assertEquals("abcdef", user1.getSipPassword());
        assertEquals("im_id", user1.getImId());
        assertEquals(32, user1.getPintoken().length());

        User user2 = impl.userFromRow(userRow2);
        assertEquals("kuku", user2.getUserName());
        assertEquals("John", user2.getFirstName());
        assertEquals("Lennon", user2.getLastName());
        assertEquals("abcdef", user2.getSipPassword());
        assertEquals(32, user2.getPintoken().length());
        assertEquals("121212 jlennon,", user2.getAliasesString());

        verify(coreContext, domainManager);
    }

    public void testCheckRowData() {
        DomainManager domainManager = createMock(DomainManager.class);
        domainManager.getAuthorizationRealm();
        expectLastCall().andReturn("sipfoundry.org").times(4);

        User superadmin = new User();
        superadmin.setUserName("superadmin");
        superadmin.setPintoken("12345678901234567890123456789012");

        replay(domainManager);

        CsvRowInserter impl = new CsvRowInserter();
        impl.setDomainManager(domainManager);

        String[] row = {
            "kuku", "", "", "", "", "", "", "", "001122334466", "polycom300", "yellow phone", ""
        };
        assertEquals(RowInserter.RowStatus.SUCCESS, impl.checkRowData(row));
        String[] rowShort = {
            "kuku", "", "", "", "", "", "", "", "001122334466", "polycom300", "yellow phone"
        };
        assertEquals(RowInserter.RowStatus.SUCCESS, impl.checkRowData(rowShort));

        String[] rowAuthRealmMatch = {
            "authMatch", "sipfoundry.org#12345678901234567890123456789012", "", "", "", "", "", "", "001122334466",
            "polycom300", "yellow phone", ""
        };
        assertEquals(RowInserter.RowStatus.SUCCESS, impl.checkRowData(rowAuthRealmMatch));

        String[] rowAuthRealmNotMatched = {
            "authNotMatch", "shipfoundry.org#12345678901234567890123456789012", "", "", "", "", "", "", "001122334466",
            "polycom300", "yellow phone", ""
        };
        assertEquals(RowInserter.RowStatus.WARNING_PIN_RESET, impl.checkRowData(rowAuthRealmNotMatched));

        String[] rowHashTooShort = {
            "hashTooShort", "sipfoundry.org#12345678", "", "", "", "", "", "", "001122334466", "polycom300",
            "yellow phone", ""
        };
        assertEquals(RowInserter.RowStatus.WARNING_PIN_RESET, impl.checkRowData(rowHashTooShort));

        String[] rowSuperadminhashpinsuccess = {
            "superadmin", "sipfoundry.org#12345678901234567890123456789012", "", "", "", "", "", "", "001122334466",
            "polycom300", "yellow phone", ""
        };
        assertEquals(RowInserter.RowStatus.SUCCESS, impl.checkRowData(rowSuperadminhashpinsuccess));

        superadmin.setPintoken("49b45dc98f67624e117a86ea4c9dc0da");
        String[] rowSuperadminclearpinsuccess = {
            "superadmin", "1234", "", "", "", "", "", "", "001122334466", "polycom300", "yellow phone", ""
        };
        assertEquals(RowInserter.RowStatus.SUCCESS, impl.checkRowData(rowSuperadminclearpinsuccess));

        //
        // Changes made allow either username or serialnumber to be blank, not both.
        //
        row[Index.USERNAME.getValue()] = "";
        assertEquals(RowInserter.RowStatus.SUCCESS, impl.checkRowData(row));
        row[Index.SERIAL_NUMBER.getValue()] = "";
        assertEquals(RowInserter.RowStatus.FAILURE, impl.checkRowData(row));
        row[Index.USERNAME.getValue()] = "kuku";
        assertEquals(RowInserter.RowStatus.SUCCESS, impl.checkRowData(row));

        verify(domainManager);
    }

    public void testDataToString() {
        CsvRowInserter impl = new CsvRowInserter();

        assertEquals("", impl.dataToString(new String[0]));

        String[] row = Index.newRow();
        assertEquals("", impl.dataToString(row));

        Index.USERNAME.set(row, "user");
        assertEquals("user", impl.dataToString(row));

        Index.SERIAL_NUMBER.set(row, "phone");
        assertEquals("user phone", impl.dataToString(row));

        Index.USERNAME.set(row, "");
        assertEquals("phone", impl.dataToString(row));
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

        PhoneContext phoneContext = createMock(PhoneContext.class);

        phoneContext.getPhoneIdBySerialNumber("001122334466");
        expectLastCall().andReturn(phoneId);
        phoneContext.loadPhone(phoneId);
        expectLastCall().andReturn(phone);

        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);
        phoneModelSource.getModel("polycom300");
        expectLastCall().andReturn(model);

        replay(phoneContext, phoneModelSource);

        CsvRowInserter impl = new CsvRowInserter();
        impl.setPhoneContext(phoneContext);
        impl.setPhoneModelSource(phoneModelSource);

        // update existing phone
        Phone phone1 = impl.phoneFromRow(phoneRow);
        assertEquals("old description", phone1.getDescription());
        assertEquals("001122334466", phone1.getSerialNumber());

        verify(phoneContext, phoneModelSource);
    }

    public void testPhoneFromRowNew() {
        final String[] phoneRow1 = new String[] {
            "", "", "", "", "", "", "", "", "001122334455", "testPhoneModel", "yellow phone", "phone in John room"
        };

        Phone phone = new TestPhone();
        PhoneModel model = new TestPhoneModel();
        phone.setDescription("old description");

        PhoneContext phoneContext = createMock(PhoneContext.class);

        phoneContext.getPhoneIdBySerialNumber("001122334455");
        expectLastCall().andReturn(null);
        phoneContext.newPhone(model);
        expectLastCall().andReturn(phone);

        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);
        phoneModelSource.getModel(model.getModelId());
        expectLastCall().andReturn(model);

        replay(phoneContext, phoneModelSource);

        CsvRowInserter impl = new CsvRowInserter();
        impl.setPhoneContext(phoneContext);
        impl.setPhoneModelSource(phoneModelSource);

        // new phone
        Phone phone1 = impl.phoneFromRow(phoneRow1);
        assertEquals("phone in John room", phone1.getDescription());
        assertEquals("001122334455", phone1.getSerialNumber());

        verify(phoneContext, phoneModelSource);
    }

    public void testPhoneFromRowSpaces() {
        final String[] phoneRow1 = new String[] {
            "", "", "", "", "", "", "", "", "001122334455", "testPhoneModel", "yellow phone", "phone in John room"
        };

        Phone phone = new TestPhone();
        PhoneModel model = new TestPhoneModel();
        phone.setDescription("old description");

        PhoneContext phoneContext = createMock(PhoneContext.class);

        phoneContext.getPhoneIdBySerialNumber("001122334455");
        expectLastCall().andReturn(null);
        phoneContext.newPhone(model);
        expectLastCall().andReturn(phone);

        ModelSource<PhoneModel> phoneModelSource = createMock(ModelSource.class);
        phoneModelSource.getModel(model.getModelId());
        expectLastCall().andReturn(model);

        replay(phoneContext, phoneModelSource);

        CsvRowInserter impl = new CsvRowInserter();
        impl.setPhoneModelSource(phoneModelSource);
        impl.setPhoneContext(phoneContext);

        // new phone
        Phone phone1 = impl.phoneFromRow(phoneRow1);
        assertEquals("phone in John room", phone1.getDescription());
        assertEquals("001122334455", phone1.getSerialNumber());

        verify(phoneContext, phoneModelSource);
    }

    public void testUpdateMailboxPreferences() {
        User user = new User();
        user.setUserName("kuku");

        MailboxManager mailboxManager = createMock(MailboxManager.class);

        mailboxManager.isEnabled();
        expectLastCall().andReturn(true);
        mailboxManager.deleteMailbox("kuku");

        mailboxManager.isEnabled();
        expectLastCall().andReturn(true);

        mailboxManager.isEnabled();
        expectLastCall().andReturn(false);
        replay(mailboxManager);

        CsvRowInserter impl = new CsvRowInserter();
        impl.setMailboxManager(mailboxManager);

        impl.updateMailbox(user, true);

        impl.updateMailbox(user, false);

        impl.updateMailbox(user, true);
        verify(mailboxManager);
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

    public void testAddLineNoUser() {
        CsvRowInserter impl = new CsvRowInserter();
        Phone phone = new TestPhone();
        phone.setModel(new PhoneModel("test"));

        Line nullUserLine = impl.addLine(phone, null);
        assertNull(nullUserLine);
        assertEquals(0, phone.getLines().size());
    }

    private class TestUser extends User {
        private final Integer m_id;

        TestUser(Integer id) {
            m_id = id;
        }

        @Override
        public Integer getId() {
            return m_id;
        }
    }
}
