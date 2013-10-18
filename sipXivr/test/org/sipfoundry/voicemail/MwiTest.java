/**
 *
 *
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
package org.sipfoundry.voicemail;

import java.io.File;
import java.io.IOException;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

public class MwiTest extends TestCase {
    File m_testdir;

    public void testRegions() throws IOException {
       Mwi mwi = new Mwi();
       mwi.setMwiAddresses("192.168.0.1@one,192.168.0.2@one,192.168.0.114,192.168.0.1@two,192.168.0.2@three,192.168.0.1");
       Map<String, List<String>> addrs = mwi.getSortedAddresses();
       assertEquals(4, addrs.keySet().size());
       List<String> regionOne = addrs.get("one");
       assertEquals(2, regionOne.size());
       assertEquals("192.168.0.1", regionOne.get(0));
       assertEquals("192.168.0.2", regionOne.get(1));
       List<String> regionTwo = addrs.get("two");
       assertEquals(1, regionTwo.size());
       assertEquals("192.168.0.1", regionTwo.get(0));
       List<String> regionThree = addrs.get("three");
       assertEquals(1, regionThree.size());
       assertEquals("192.168.0.2", regionThree.get(0));
       List<String> globalRegion = addrs.get("global");
       assertEquals(2, globalRegion.size());
       assertEquals("192.168.0.114", globalRegion.get(0));
       assertEquals("192.168.0.1", globalRegion.get(1));
    }
}
