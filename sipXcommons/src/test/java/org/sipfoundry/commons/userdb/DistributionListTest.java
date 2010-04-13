/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.commons.userdb;

import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Vector;

import junit.framework.TestCase;

public class DistributionListTest extends TestCase {

    HashMap<String, DistributionList> m_sysDistroList;
    protected void setUp() throws Exception {
        super.setUp();
        DistributionList emDistroList = new DistributionList("em", null);
        emDistroList.addMailboxString("me");
        emDistroList.addMailboxString("myself");
        DistributionList eyeDistroList = new DistributionList("eye", null);
        eyeDistroList.addSystemListString("em");
        eyeDistroList.addMailboxString("I");
        eyeDistroList.addMailboxString("igor");
        DistributionList allDistroList = new DistributionList("all", null);
        allDistroList.addMailboxString("me");
        allDistroList.addMailboxString("myself");
        allDistroList.addMailboxString("I");
        DistributionList recurse1DistroList = new DistributionList("recurse1", null);
        recurse1DistroList.addSystemListString("recurse2");
        recurse1DistroList.addMailboxString("r_one");
        DistributionList recurse2DistroList = new DistributionList("recurse2", null);
        recurse2DistroList.addSystemListString("recurse1");
        recurse1DistroList.addMailboxString("r_two");
        m_sysDistroList = new HashMap<String, DistributionList>();
        m_sysDistroList.put("all", allDistroList);
        m_sysDistroList.put("eye", eyeDistroList);
        m_sysDistroList.put("em", emDistroList);
        m_sysDistroList.put("recurse1", recurse1DistroList);
        m_sysDistroList.put("recurse2", recurse2DistroList);
    }

    protected void tearDown() throws Exception {
        super.tearDown();
    }

    public void testGetList() {
        DistributionList dl = new DistributionList("testGetList1", null);
        dl.addMailboxString("one");
        dl.addMailboxString("two");
        dl.addMailboxString("me");
        dl.addSystemListString("em");
        HashSet<String> list = dl.getList(m_sysDistroList);
        Vector<String> v = new Vector<String>();
        v.addAll(list);
        Collections.sort(v);
        assertEquals("[me, myself, one, two]", v.toString());

        dl = new DistributionList("testGetList2", null);
        dl.addSystemListString("recurse1");
        list = dl.getList(m_sysDistroList);
        v = new Vector<String>();
        v.addAll(list);
        Collections.sort(v);
        assertEquals("[r_one, r_two]", v.toString());

    }

}
