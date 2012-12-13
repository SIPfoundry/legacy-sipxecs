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

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;
import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.sipfoundry.sipxconfig.bulk.RowInserter.RowStatus;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.sipfoundry.sipxconfig.phone.TestPhone;
import org.sipfoundry.sipxconfig.phone.TestPhoneModel;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

public class CsvRowInserterTest extends TestCase {
    public void testUserFromRow() {
        User bongo = new User();
        bongo.setUserName("bongo");
        bongo.setFirstName("Ringo");

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUserByUserName("bongo");
        expectLastCall().andReturn(bongo);
        coreContext.loadUserByUserName("kuku");
        expectLastCall().andReturn(null);
        coreContext.newUser();
        expectLastCall().andReturn(new User());

        replay(coreContext);

        String[] userRow1 = new String[] {
            "bongo", "1234", "123", "abcdef", "", "Star", "","","","","","",""," im_id"
        };

        String[] userRow2 = new String[] {
            "kuku", "1234", "125c43a02fc91d8229ee307e6d2e2d20", "abcdef", " John", " Lennon  ", "jlennon, 121212"
        };

        CsvRowInserter impl = new CsvRowInserter();
        impl.setCoreContext(coreContext);

        User user1 = impl.userFromRow(userRow1);
        assertEquals("bongo", user1.getUserName());
        assertEquals("f9a17b19d3f01f6211415ca101145686", user1.getVoicemailPintoken());
        assertEquals("1234", user1.getPintoken());
        assertEquals("Ringo", user1.getFirstName());
        assertEquals("Star", user1.getLastName());
        assertEquals("abcdef", user1.getSipPassword());
        assertEquals("im_id", user1.getImId());
        assertEquals(4, user1.getPintoken().length());

        User user2 = impl.userFromRow(userRow2);
        assertEquals("kuku", user2.getUserName());
        assertEquals("125c43a02fc91d8229ee307e6d2e2d20", user2.getVoicemailPintoken());
        assertEquals("1234", user2.getPintoken());
        assertEquals("John", user2.getFirstName());
        assertEquals("Lennon", user2.getLastName());
        assertEquals("abcdef", user2.getSipPassword());
        assertEquals(4, user2.getPintoken().length());
        assertEquals("121212 jlennon,", user2.getAliasesString());

        verify(coreContext);
    }

    public void testUserDetailsFromRow() {
        User bongo = new User();
        bongo.setUserName("bongo");
        bongo.setFirstName("Ringo");

        CoreContext coreContext = createMock(CoreContext.class);
        coreContext.loadUserByUserName("bongo");
        expectLastCall().andReturn(bongo);

        PermissionManager pm = createMock(PermissionManager.class);
        pm.getPermissionModel();
        expectLastCall().andReturn(TestHelper.loadSettings("commserver/user-settings.xml")).anyTimes();
        bongo.setPermissionManager(pm);

        replay(coreContext, pm);

        String[] userRow1 = new String[] {
            "bongo", "1234", "123", "abcdef", "", "Star", "", "", "", "", "", "", "", " im_id", "Prof", "Manager",
            "user1122", "job title", "job dept", "company name", "assistant name", "001122", "112233", "223344",
            "33445566", "44556677", "alternate@gmail.com", "alternateImId", "location", "home street", "home city",
            "home state", "home country", "34001", "office street", "office city", "office state", "office country",
            "34342", "office designation", "Twitter", "Linkedin", "Facebook", "Xing", "outofoffice", "1", "MEDIUM",
            "true", "1", "BRIEF", "false", "true", "CallerID", "true", "", ""
        };

        CsvRowInserter impl = new CsvRowInserter();
        impl.setCoreContext(coreContext);

        User user1 = impl.userFromRow(userRow1);

        assertEquals("Prof", user1.getUserProfile().getSalutation());
        assertEquals("Manager", user1.getUserProfile().getManager());
        assertEquals("user1122", user1.getUserProfile().getEmployeeId());
        assertEquals("company name", user1.getUserProfile().getCompanyName());
        assertEquals("job title", user1.getUserProfile().getJobTitle());
        assertEquals("job dept", user1.getUserProfile().getJobDept());
        assertEquals("company name", user1.getUserProfile().getCompanyName());
        assertEquals("assistant name", user1.getUserProfile().getAssistantName());
        assertEquals("001122", user1.getUserProfile().getCellPhoneNumber());
        assertEquals("112233", user1.getUserProfile().getHomePhoneNumber());
        assertEquals("223344", user1.getUserProfile().getAssistantPhoneNumber());
        assertEquals("33445566", user1.getUserProfile().getFaxNumber());
        assertEquals("44556677", user1.getUserProfile().getDidNumber());
        assertEquals("alternate@gmail.com", user1.getUserProfile().getAlternateEmailAddress());
        assertEquals("alternateImId", user1.getUserProfile().getAlternateImId());
        assertEquals("location", user1.getUserProfile().getLocation());

        assertEquals("home street", user1.getUserProfile().getHomeAddress().getStreet());
        assertEquals("home city", user1.getUserProfile().getHomeAddress().getCity());
        assertEquals("home state", user1.getUserProfile().getHomeAddress().getState());
        assertEquals("home country", user1.getUserProfile().getHomeAddress().getCountry());
        assertEquals("34001", user1.getUserProfile().getHomeAddress().getZip());

        assertEquals("office street", user1.getUserProfile().getOfficeAddress().getStreet());
        assertEquals("office city", user1.getUserProfile().getOfficeAddress().getCity());
        assertEquals("office state", user1.getUserProfile().getOfficeAddress().getState());
        assertEquals("office country", user1.getUserProfile().getOfficeAddress().getCountry());
        assertEquals("34342", user1.getUserProfile().getOfficeAddress().getZip());
        assertEquals("office designation", user1.getUserProfile().getOfficeAddress().getOfficeDesignation());

        assertEquals("Twitter", user1.getUserProfile().getTwiterName());
        assertEquals("Linkedin", user1.getUserProfile().getLinkedinName());
        assertEquals("Facebook", user1.getUserProfile().getFacebookName());
        assertEquals("Xing", user1.getUserProfile().getXingName());

        assertEquals("outofoffice", user1.getActiveGreeting());
        assertEquals("1", user1.getPrimaryEmailNotification());
        assertEquals("MEDIUM", user1.getPrimaryEmailFormat());
        assertEquals(new Boolean(true), user1.isPrimaryEmailAttachAudio());
        assertEquals("1", user1.getAlternateEmailNotification());
        assertEquals("BRIEF", user1.getAlternateEmailFormat());
        assertEquals(new Boolean(false), user1.isAlternateEmailAttachAudio());
        assertEquals(new Boolean(true), user1.isVoicemailServer());
        assertEquals("CallerID", user1.getExternalNumber());
        assertEquals(new Boolean(true), user1.isAnonymousCallerAlias());

        verify(coreContext);
    }

    public void testCheckRowData() {
        User superadmin = new User();
        superadmin.setUserName("superadmin");
        superadmin.setPintoken("12345678901234567890123456789012");

        CsvRowInserter impl = new CsvRowInserter();

        String[] row = {
            "kuku", "123", "777", "", "", "", "", "", "", "001122334466", "polycom300", "yellow phone", ""
        };
        assertEquals(RowStatus.SUCCESS, impl.checkRowData(row).getRowStatus());

        String[] rowShort = {
            "kuku", "", "", "", "", "", "", "", "", "001122334466", "polycom300", "yellow phone"
        };
        assertEquals(RowStatus.SUCCESS, impl.checkRowData(rowShort).getRowStatus());

        String[] rowAuthRealmMatch = {
            "authMatch", "1234", "123", "", "", "", "", "", "", "001122334466",
            "polycom300", "yellow phone", ""
        };
        assertEquals(RowStatus.SUCCESS, impl.checkRowData(rowAuthRealmMatch).getRowStatus());

        String[] rowSuperadminhashpinsuccess = {
            "superadmin", "12345678901234567890123456789012", "", "", "", "", "", "", "", "001122334466",
            "polycom300", "yellow phone", ""
        };
        assertEquals(RowStatus.SUCCESS, impl.checkRowData(rowSuperadminhashpinsuccess).getRowStatus());

        superadmin.setPintoken("49b45dc98f67624e117a86ea4c9dc0da");
        String[] rowSuperadminclearpinsuccess = {
            "superadmin", "1234", "444", "", "", "", "", "", "", "001122334466", "polycom300", "yellow phone", ""
        };
        assertEquals(RowStatus.SUCCESS, impl.checkRowData(rowSuperadminclearpinsuccess).getRowStatus());

        //
        // Changes made allow either username or serialnumber to be blank, not both.
        //
        row[Index.USERNAME.getValue()] = "";
        assertEquals(RowInserter.RowStatus.SUCCESS, impl.checkRowData(row).getRowStatus());
        row[Index.SERIAL_NUMBER.getValue()] = "";
        assertEquals(RowInserter.RowStatus.FAILURE, impl.checkRowData(row).getRowStatus());
        row[Index.USERNAME.getValue()] = "kuku";
        assertEquals(RowInserter.RowStatus.SUCCESS, impl.checkRowData(row).getRowStatus());

        String[] rowWithInvalidUsername = {
            "@200", "", "", "", "", "", "", "", "001122334466", "polycom300", "yellow phone", ""
        };
        assertEquals(RowStatus.FAILURE, impl.checkRowData(rowWithInvalidUsername).getRowStatus());

        String[] rowWithVMSettingsAndCallerId = {
            "200", "", "", "", "", "", "", "", "0004f22f62ab", "polycom335", "Polycom Polycom335", "", "", "", "",
            "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
            "", "", "", "outofoffice", "1", "MEDIUM", "true", "1", "BRIEF", "false", "true", "CallerID", "true", "",
            ""
        };
        assertEquals(RowStatus.SUCCESS, impl.checkRowData(rowWithVMSettingsAndCallerId).getRowStatus());

        String[] rowWithInvalidActiveGreeting = {
            "200", "", "", "", "", "", "", "", "0004f22f62ab", "polycom335", "Polycom Polycom335", "", "", "", "",
            "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
            "", "", "", "wrongGreetings", "1", "MEDIUM", "true", "1", "BRIEF", "false", "true", "JohnCallerID",
            "true", "", ""
        };
        assertEquals(RowStatus.FAILURE, impl.checkRowData(rowWithInvalidActiveGreeting).getRowStatus());

        String[] rowWithInvalidEmailNotification = {
            "200", "", "", "", "", "", "", "", "0004f22f62ab", "polycom335", "Polycom Polycom335", "", "", "", "",
            "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
            "", "", "", "outofoffice", "3", "MEDIUM", "true", "", "", "", "", "CallerID", "true", "", ""
        };
        assertEquals(RowStatus.FAILURE, impl.checkRowData(rowWithInvalidEmailNotification).getRowStatus());

        String[] rowWithInvalidEmailFormat = {
            "200", "", "", "", "", "", "", "", "0004f22f62ab", "polycom335", "Polycom Polycom335", "", "", "", "",
            "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
            "", "", "", "outofoffice", "2", "WRONG", "true", "", "", "", "", "CallerID", "true", "", ""
        };
        assertEquals(RowStatus.FAILURE, impl.checkRowData(rowWithInvalidEmailFormat).getRowStatus());

        String[] rowWithInvalidEmailAttachAudio = {
            "200", "", "", "", "", "", "", "", "0004f22f62ab", "polycom335", "Polycom Polycom335", "", "", "", "",
            "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
            "", "", "", "outofoffice", "2", "FULL", "wrong", "", "", "", "", "CallerID", "", "", ""
        };
        assertEquals(RowStatus.FAILURE, impl.checkRowData(rowWithInvalidEmailAttachAudio).getRowStatus());

        String[] rowWithInvalidCallerID = {
            "200", "", "", "", "", "", "", "", "0004f22f62ab", "polycom335", "Polycom Polycom335", "", "", "", "",
            "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
            "", "", "", "outofoffice", "2", "FULL", "wrong", "", "", "", "", "wrong Caller ID", "", "", ""
        };
        assertEquals(RowStatus.FAILURE, impl.checkRowData(rowWithInvalidCallerID).getRowStatus());
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
            "", "", "", "", "", "", "", "", "", "001122334466", "polycom300", "yellow phone", ""
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
            "", "", "", "", "", "", "", "", "", "001122334455", "testPhoneModel", "yellow phone", "phone in John room"
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
            "", "", "", "", "", "", "", "", "", "001122334455", "testPhoneModel", "yellow phone", "phone in John room"
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

    public void testAddLine() {
        CsvRowInserter impl = new CsvRowInserter();
        Phone phone = new TestPhone();
        phone.setModel(new PhoneModel("test"));
        User user = new User();
        Line expected = impl.addLine(phone, user, "");
        Line actual = impl.addLine(phone, user, "");
        assertSame(expected, actual);

        User newuser = new TestUser(1);
        Line newline = impl.addLine(phone, newuser, "");
        assertNotSame(expected, newline);
    }

    public void testAddLineNoUser() {
        CsvRowInserter impl = new CsvRowInserter();
        Phone phone = new TestPhone();
        phone.setModel(new PhoneModel("test"));

        Line nullUserLine = impl.addLine(phone, null, "");
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
