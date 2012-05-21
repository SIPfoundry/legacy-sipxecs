/**
 * Copyright (c) 2012 eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 */
package org.sipfoundry.sipxconfig.alarm;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import java.io.IOException;
import java.util.List;
import java.util.Map;

import org.junit.Test;

public class AlarmLogParserTest {
    
    @Test
    public void decode() {
        AlarmLogParser parser = new AlarmLogParser();
        assertEquals("lion", parser.decodeValue("= STRING: lion"));
        assertEquals("lion sleeps tonight", parser.decodeValue("= STRING: \"lion sleeps tonight\""));
    }    
    
    @Test
    public void parseFields() throws IOException {
        AlarmLogParser parser = new AlarmLogParser();
        Map<String, String> actual = parser.parseFields("bird", "cat::species = STRING: lion bird::species = STRING: goose bird::color = STRING: gray");
        assertEquals(2, actual.size());
        assertEquals("goose", actual.get("species"));
        assertEquals("gray", actual.get("color"));
    }
    
    @Test
    public void parse() throws IOException {
        AlarmLogParser parser = new AlarmLogParser();
        List<AlarmEvent> events = parser.parse(null, null, 0, 10, getClass().getResourceAsStream("alarm.test.log"));
        assertEquals(2, events.size());
        AlarmEvent[] actual = events.toArray(new AlarmEvent[0]);
        assertNotNull(actual[0].getDate());       
    }
}
