/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.time;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.List;

import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.test.TestUtil;

import junit.framework.TestCase;

public class TimeManagerImplTest extends TestCase {
    private TimeManagerImpl m_manager;
    @Override
    protected void setUp() throws Exception {
        m_manager = new TimeManagerImpl();
        m_manager.setBinDirectory(TestUtil.getTestSourceDirectory(TimeManagerImplTest.class));
        m_manager.setLibExecDirectory(TestUtil.getTestSourceDirectory(TimeManagerImplTest.class));
        m_manager.setNtpConfigFile("tempNtpConfigFile");
    }

    public void testGetStatus() throws Exception {
        int returnValue = m_manager.getSystemTimeSettingType();
        assertEquals(1, returnValue);
    }

    public void testGetServers() throws Exception {
        List<String> returnValue = m_manager.getNtpServers();
        ArrayList<String> expectedServers = new ArrayList<String>();
        expectedServers.add("1.ntp.server");
        expectedServers.add("2.ntp.server");
        expectedServers.add("3.ntp.server");

        assertEquals(expectedServers, returnValue);
    }

    public void testGetConfiguration() throws Exception {
        String returnValue = m_manager.getNtpConfiguration();
        String expectedConfiguration = "\nNTP CONFIGURATION LINE 1\nNTP CONFIGURATION LINE 2\nNTP CONFIGURATION LINE 3\n";

        assertEquals(expectedConfiguration, returnValue);
    }

    public void testNtpServers() throws Exception {
        ArrayList<String> servers = new ArrayList<String>();
        servers.add("1.ntp.server");
        servers.add("2.ntp.server");
        servers.add("3.ntp.server");

        try {
            m_manager.setNtpServers(servers);
        } catch (UserException ex) {
            // if we get an exeption, it means the script didn't return right ...
            assertTrue(false);
        }
        // the script did return right
        assertTrue(true);
    }

    public void testSetSystemDate() throws Exception {
         GregorianCalendar gc = new GregorianCalendar();
         // YEAR + MONTH + DAY_OF_MONTH
         gc.set(Calendar.YEAR, 2002);
         gc.set(Calendar.MONTH, Calendar.FEBRUARY);
         gc.set(Calendar.DAY_OF_MONTH, 02);
         gc.set(Calendar.HOUR_OF_DAY, 02);
         gc.set(Calendar.MINUTE, 02);


        try {
            m_manager.setSystemDate(new SimpleDateFormat("MMddHHmmyyyy").format(gc.getTime()));
        } catch (UserException ex) {
            // if we get an exeption, it means the script didn't return right ...
            assertTrue(false);
        }
        // the script did return right
        assertTrue(true);
    }
}
