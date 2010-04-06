/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.admin.time;

import java.io.Reader;
import java.io.StringReader;
import java.util.List;

import junit.framework.TestCase;

public class TimezoneTest extends TestCase {

    StringReader sr = null;

    private class TimezoneMock extends Timezone {

        @Override
        protected Reader getReaderForClockFile() {
            return sr;
        }
    }

    public void testInitalizeTimezoneFromClockFilewithQuotes() {
        // double quotes around ZONE="test"
        sr = new StringReader("Here is my test data\nZONE=\"test1\"\n");
        TimezoneMock tzm = new TimezoneMock();
        assertEquals("test1", tzm.getTimezone());
    }

    public void testInitalizeTimezoneFromClockFilewithNoQuotes() {
        // no quotes around ZONE=test
        sr = new StringReader("Here is my test data\nZONE=test2\n");
        TimezoneMock tzm = new TimezoneMock();
        assertEquals("test2", tzm.getTimezone());

    }

    public void testInitalizeTimezoneFromClockFilewithNoZone() {
        // no ZONE= in the file
        sr = new StringReader("Here is my test data\n\n");
        TimezoneMock tzm = new TimezoneMock();
        assertEquals("", tzm.getTimezone());
    }

    public void testListAllTimezonesWithCurrentTZInList() {
        sr = new StringReader("Here is my test data\nZONE=Europe/Dublin\n");
        TimezoneMock tzm = new TimezoneMock();

        List<String> timezonesList = tzm.getAllTimezones();
        assertEquals("Europe/Dublin", tzm.getTimezone());
        assertTrue(timezonesList.contains("Africa/Timbuktu"));
        assertTrue(timezonesList.contains("America/Los_Angeles"));

        // Brazil/Acres should not be in list.
        assertFalse(timezonesList.contains("Brazil/Acres"));
        assertTrue(timezonesList.contains(tzm.getTimezone()));
    }

    public void testListAllTimezonesWithCurrentTZNotInList() {
        // no ZONE= in the file
        sr = new StringReader("Here is my test data\nZONE=Continent/City\n");
        TimezoneMock tzm = new TimezoneMock();

        List<String> timezonesList = tzm.getAllTimezones();
        assertEquals("Continent/City", tzm.getTimezone());

        //
        // The Timezone files will not contain Continent/City.
        // So the code adds the Continent/City to the list.
        //
        assertTrue(timezonesList.contains(tzm.getTimezone()));
    }
}
