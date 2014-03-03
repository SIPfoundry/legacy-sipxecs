/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.StringWriter;
import java.util.Arrays;
import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.commons.userdb.profile.Address;
import org.sipfoundry.commons.userdb.profile.Salutation;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.permission.PermissionManagerImpl;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.TestPhone;
import org.sipfoundry.sipxconfig.phone.TestPhoneModel;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public class ExportCsvTest extends TestCase {

    private ExportCsv m_export;

    private User m_user;

    @Override
    protected void setUp() throws Exception {
        m_export = new ExportCsv();

        PermissionManagerImpl pm = new PermissionManagerImpl();
        pm.setModelFilesContext(TestHelper.getModelFilesContext());

        m_user = new User();
        m_user.setPermissionManager(pm);
        m_user.setFirstName("John");
        m_user.setLastName("Lennon");
        m_user.setPin("1234");
        m_user.setVoicemailPintoken("f09e0cfc1dc0f8852ad731e79230225b");
        m_user.setSipPassword("sip_pass");
        m_user.setUserName("jlennon");

        m_user.setPrimaryEmailNotification(MailboxPreferences.AttachType.YES.getValue());
        m_user.setPrimaryEmailFormat(MailboxPreferences.MailFormat.MEDIUM.name());
        m_user.setPrimaryEmailAttachAudio(true);
        m_user.setAlternateEmailNotification(MailboxPreferences.AttachType.NO.getValue());
        m_user.setAlternateEmailFormat(MailboxPreferences.MailFormat.FULL.name());
        m_user.setAlternateEmailAttachAudio(false);
    }

    public void testExportPhoneNoLines() throws Exception {
        StringWriter writer = new StringWriter();
        SimpleCsvWriter csv = new SimpleCsvWriter(writer);

        TestPhone phone = new TestPhone();
        phone.setModel(new TestPhoneModel());
        phone.setSerialNumber("665544332211");
        phone.setDescription("phone description");
        Collection<String> userIds = m_export.exportPhone(csv, phone);
        assertEquals(0, userIds.size());
        assertEquals(
                "\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"665544332211\",\"testPhoneModel\",\"\",\"phone description\","
                        + "\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\","
                        + "\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"\n",
                writer.toString());
    }

    public void testExportPhone() throws Exception {
        StringWriter writer = new StringWriter();
        SimpleCsvWriter csv = new SimpleCsvWriter(writer);

        // Add 3 user Groups to test export of multiple groups
        Group[] groups = new Group[3];
        groups[0] = new Group();
        groups[0].setName("ug1");
        groups[1] = new Group();
        groups[1].setName("ug2");
        groups[2] = new Group();
        groups[2].setName("ug3");

        m_user.setEmailAddress("jlennon@gmail.com");
        m_user.setImId("imId");
        m_user.setGroupsAsList(Arrays.asList(groups));

        Line line = new Line();
        line.setUser(m_user);

        // Add 4 phone Groups to test export of multiple groups
        // Note: phonegroup3, has a comma in the name. This is to make sure the double quotes are
        // around the field
        // and the comma is not taken as a seperator for the csv file.
        Group[] phoneGroups = new Group[4];
        phoneGroups[0] = new Group();
        phoneGroups[0].setName("phonegroup1");
        phoneGroups[1] = new Group();
        phoneGroups[1].setName("phonegroup2");
        phoneGroups[2] = new Group();
        phoneGroups[2].setName("phonegroup3,");
        phoneGroups[3] = new Group();
        phoneGroups[3].setName("phonegroup4");
        TestPhone phone = new TestPhone();
        phone.setModel(new TestPhoneModel());
        phone.setSerialNumber("665544332211");
        phone.setDescription("phone description");
        phone.setGroupsAsList(Arrays.asList(phoneGroups));
        phone.addLine(line);

        Collection<String> userIds = m_export.exportPhone(csv, phone);
        assertEquals(1, userIds.size());
        assertTrue(userIds.contains("jlennon"));
        assertEquals(
                "\"jlennon\",\"1234\",\"f09e0cfc1dc0f8852ad731e79230225b\",\"sip_pass\",\"John\",\"Lennon\",\"\",\"jlennon@gmail.com\",\"ug1 ug2 ug3\",\"665544332211\",\"testPhoneModel\",\"phonegroup1 phonegroup2 phonegroup3, phonegroup4\",\"phone description\",\"imid\","
                        + "\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\","
                        + "\"standard\",\"1\",\"MEDIUM\",\"true\",\"0\",\"FULL\",\"false\",\"true\",\"\",\"false\",\"\",\"\",\"\",\"\"\n",
                writer.toString());
    }

    public void testExportPhoneExternalLine() throws Exception {
        StringWriter writer = new StringWriter();
        SimpleCsvWriter csv = new SimpleCsvWriter(writer);

        Line line = new Line();

        TestPhone phone = new TestPhone();
        phone.setModel(new TestPhoneModel());
        phone.setSerialNumber("665544332211");
        phone.setDescription("phone description");
        phone.addLine(line);

        LineInfo lineInfo = new LineInfo();
        lineInfo.setDisplayName("dp");
        lineInfo.setExtension("432");
        lineInfo.setRegistrationServer("example.com");
        line.setLineInfo(lineInfo);

        Collection<String> userIds = m_export.exportPhone(csv, phone);
        assertEquals(0, userIds.size());
        assertEquals(
                "\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"665544332211\",\"testPhoneModel\",\"\",\"phone description\",\"\""
                        + ",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\","
                        + "\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\"\n",
                writer.toString());
    }

    public void testExportUserNoEmail() throws Exception {
        StringWriter writer = new StringWriter();
        SimpleCsvWriter csv = new SimpleCsvWriter(writer);
        String[] row = Index.newRow();

        m_export.exportUser(csv, row, m_user);
        assertEquals(
                "\"jlennon\",\"1234\",\"f09e0cfc1dc0f8852ad731e79230225b\",\"sip_pass\",\"John\",\"Lennon\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\""
                        + ",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\","
                        + "\"standard\",\"1\",\"MEDIUM\",\"true\",\"0\",\"FULL\",\"false\",\"true\",\"\",\"false\",\"\",\"\",\"\",\"\"\n",
                writer.toString());
    }

    public void testExportUserDetails() throws Exception {
        StringWriter writer = new StringWriter();
        SimpleCsvWriter csv = new SimpleCsvWriter(writer);
        String[] row = Index.newRow();

        UserProfile profile = new UserProfile();
        profile.setSalutationId(Salutation.Prof);
        profile.setManager("Manager");
        profile.setEmployeeId("user112233");
        profile.setJobTitle("job title");
        profile.setJobDept("job dept");
        profile.setCompanyName("company name");
        profile.setAssistantName("assistant name");
        profile.setCellPhoneNumber("001122");
        profile.setHomePhoneNumber("112233");
        profile.setAssistantPhoneNumber("223344");
        profile.setFaxNumber("33445566");
        profile.setDidNumber("44556677");
        profile.setAlternateEmailAddress("alternate@gmail.com");
        profile.setAlternateImId("alternateImId");
        profile.setLocation("location");

        Address homeAddress = new Address();
        homeAddress.setStreet("home street");
        homeAddress.setCity("home city");
        homeAddress.setState("home state");
        homeAddress.setCountry("home country");
        homeAddress.setZip("34001");
        profile.setHomeAddress(homeAddress);

        Address officeAddress = new Address();
        officeAddress.setStreet("office street");
        officeAddress.setCity("office city");
        officeAddress.setState("office state");
        officeAddress.setCountry("office country");
        officeAddress.setZip("34342");
        officeAddress.setOfficeDesignation("office designation");
        profile.setOfficeAddress(officeAddress);

        profile.setTwiterName("Twitter Name");
        profile.setLinkedinName("Linkedin Name");
        profile.setFacebookName("Facebook Name");
        profile.setXingName("Xing Name");

        m_user.setUserProfile(profile);

        m_export.exportUser(csv, row, m_user);
        assertEquals(
                "\"jlennon\",\"1234\",\"f09e0cfc1dc0f8852ad731e79230225b\",\"sip_pass\",\"John\",\"Lennon\",\"\",\"\",\"\",\"\",\"\",\"\",\"\",\"\""
                        + ",\"Prof\",\"Manager\",\"user112233\",\"job title\",\"job dept\",\"company name\",\"assistant name\","
                        + "\"001122\",\"112233\",\"223344\",\"33445566\",\"44556677\",\"alternate@gmail.com\",\"alternateImId\",\"location\","
                        + "\"home street\",\"home city\",\"home state\",\"home country\",\"34001\","
                        + "\"office street\",\"office city\",\"office state\",\"office country\",\"34342\",\"office designation\",\"Twitter Name\",\"Linkedin Name\",\"Facebook Name\",\"Xing Name\","
                        + "\"standard\",\"1\",\"MEDIUM\",\"true\",\"0\",\"FULL\",\"false\",\"true\",\"\",\"false\",\"\",\"\",\"\",\"\"\n",
                writer.toString());
    }
}
