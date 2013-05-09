/**
 * Copyright (c) 2013 eZuce, Inc. All rights reserved.
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
package org.sipfoundry.sipxconfig.common;

import static org.junit.Assert.assertEquals;

import java.util.Map;

import org.junit.Before;
import org.junit.Test;

public class DartExampleApiTest {
    private DartExampleApi m_api;
    
    @Before
    public void setUp() {
        m_api = new DartExampleApi();
        DartExampleApi.reset();
    }
    
    @Test
    public void delete() {
        int before = getSpottings().size();
        m_api.parseForm("{action:'DELETE',day:'01/03/2013'}");
        assertEquals(before - 1, getSpottings().size());
    }
    
    @Test
    public void add() {
        int before = getSpottings().size();
        m_api.parseForm("{action:'ADD',day:'01/09/2013',bird:'jay'}");
        assertEquals(before + 1, getSpottings().size());
        assertEquals(1, getSpottings().get("01/09/2013").length);
        m_api.parseForm("{action:'ADD',day:'01/09/2013',bird:'grackle'}");
        assertEquals(before + 1, getSpottings().size());
        assertEquals(2, getSpottings().get("01/09/2013").length);
    }

    @SuppressWarnings("unchecked")
    Map<String, String[]> getSpottings() {
        return (Map<String, String[]>) m_api.getReport().get("spottings");
    }
}
