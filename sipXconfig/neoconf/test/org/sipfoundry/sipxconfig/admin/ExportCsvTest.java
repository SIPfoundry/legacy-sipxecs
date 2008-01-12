/*
 *
 *
 * Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 *
 */
package org.sipfoundry.sipxconfig.admin;

import java.io.StringWriter;
import java.util.Collection;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.bulk.csv.CsvWriter;
import org.sipfoundry.sipxconfig.bulk.csv.Index;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Line;
import org.sipfoundry.sipxconfig.phone.LineInfo;
import org.sipfoundry.sipxconfig.phone.TestPhone;
import org.sipfoundry.sipxconfig.phone.TestPhoneModel;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

import static org.easymock.EasyMock.createMock;
import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.EasyMock.replay;
import static org.easymock.EasyMock.verify;

public class ExportCsvTest extends TestCase {

    protected void setUp() throws Exception {
        super.setUp();
    }

    public void testExportPhoneNoLines() throws Exception {
        ExportCsv exportCsv = new ExportCsv();

        StringWriter writer = new StringWriter();
        CsvWriter csv = new CsvWriter(writer);

        TestPhone phone = new TestPhone();
        phone.setModel(new TestPhoneModel());
        phone.setSerialNumber("665544332211");
        phone.setDescription("phone description");

        Collection<String> userIds = exportCsv.exportPhone(csv, phone, "example.org");
        assertEquals(0, userIds.size());
        assertEquals(",,,,,,,,665544332211,testPhoneModel,,phone description\n", writer
                .toString());
    }

    public void testExportPhone() throws Exception {
        MailboxManager mm = createMock(MailboxManager.class);
        mm.isEnabled();
        expectLastCall().andReturn(false);
        replay(mm);

        ExportCsv exportCsv = new ExportCsv();
        exportCsv.setMailboxManager(mm);

        StringWriter writer = new StringWriter();
        CsvWriter csv = new CsvWriter(writer);

        User user = new User();
        user.setFirstName("John");
        user.setLastName("Lennon");
        user.setPin("1234", "example.org");
        user.setSipPassword("sip_pass");
        user.setUserName("jlennon");

        Line line = new Line();
        line.setUser(user);

        TestPhone phone = new TestPhone();
        phone.setModel(new TestPhoneModel());
        phone.setSerialNumber("665544332211");
        phone.setDescription("phone description");
        phone.addLine(line);

        Collection<String> userIds = exportCsv.exportPhone(csv, phone, "example.org");
        assertEquals(1, userIds.size());
        assertTrue(userIds.contains("jlennon"));
        assertEquals(
                "jlennon,example.org#b5032ad9a3aa310dc62bf140a6d8b36e,sip_pass,John,Lennon,,,,665544332211,testPhoneModel,,phone description\n",
                writer.toString());
        verify(mm);
    }

    public void testExportPhoneExternalLine() throws Exception {
        ExportCsv exportCsv = new ExportCsv();

        StringWriter writer = new StringWriter();
        CsvWriter csv = new CsvWriter(writer);

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

        Collection<String> userIds = exportCsv.exportPhone(csv, phone, "example.org");
        assertEquals(0, userIds.size());
        assertEquals(",,,,,,,,665544332211,testPhoneModel,,phone description\n", writer
                .toString());
    }

    public void testExportUserNoEmail() throws Exception {
        MailboxManager mm = createMock(MailboxManager.class);
        mm.isEnabled();
        expectLastCall().andReturn(false);
        replay(mm);

        ExportCsv exportCsv = new ExportCsv();
        exportCsv.setMailboxManager(mm);

        StringWriter writer = new StringWriter();
        CsvWriter csv = new CsvWriter(writer);
        String[] row = Index.newRow();

        User user = new User();
        user.setFirstName("John");
        user.setLastName("Lennon");
        user.setPin("1234", "example.org");
        user.setSipPassword("sip_pass");
        user.setUserName("jlennon");

        exportCsv.exportUser(csv, row, user, "example.org");
        assertEquals(
                "jlennon,example.org#b5032ad9a3aa310dc62bf140a6d8b36e,sip_pass,John,Lennon,,,,,,,\n",
                writer.toString());
        verify(mm);
    }
}
