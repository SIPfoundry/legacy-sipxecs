/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.vm;

import static org.apache.commons.lang.StringUtils.join;

import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringWriter;
import java.util.Collection;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.TestHelper;

public class DistributionListsTest extends TestCase {

    private DistributionListsReader m_reader;
    private DistributionListsWriter m_writer;

    protected void setUp() {
        m_reader = new DistributionListsReader();
        m_writer = new DistributionListsWriter();
        m_writer.setVelocityEngine(TestHelper.getVelocityEngine());
    }

    public void testReadPreferences() {
        InputStream in = getClass().getResourceAsStream("200/distribution.xml");
        DistributionList[] lists = m_reader.readObject(new InputStreamReader(in));
        IOUtils.closeQuietly(in);
        assertEquals(10, lists.length);
        assertEquals(join(new String[] {
            "200", "202"
        }, ' '), join(lists[1].getExtensions(), ' '));
        assertNull(lists[0].getExtensions());
    }

    public void testWritePreferences() throws IOException {
        DistributionList[] lists = DistributionList.createBlankList();
        StringWriter actual = new StringWriter();
        lists[1].setExtensions(new String[] {
            "300", "900"
        });
        lists[3].setExtensions(new String[0]);
        m_writer.writeObject(lists, actual);
        StringWriter expected = new StringWriter();
        InputStream expectedIn = getClass().getResourceAsStream("expected-distribution.xml");
        IOUtils.copy(expectedIn, expected);
        IOUtils.closeQuietly(expectedIn);
        assertEquals(expected.toString(), actual.toString());
    }

    public void testGetUniqueExtensions() throws Exception {
        DistributionList[] dls = DistributionList.createBlankList();
        assertEquals(0, DistributionList.getUniqueExtensions(dls).size());

        DistributionList dl1 = new DistributionList();
        dl1.setExtensions(StringUtils.split("123 234 345 456"));
        Collection<String> uniqueExtensions = DistributionList.getUniqueExtensions(dl1);
        assertEquals(4, uniqueExtensions.size());
        assertTrue(uniqueExtensions.contains("123"));
        assertFalse(uniqueExtensions.contains("567"));

        DistributionList dl2 = new DistributionList();
        dl2.setExtensions(StringUtils.split("234 345 456 567"));
        Collection<String> uniqueExtensions2 = DistributionList.getUniqueExtensions(dl1, dl2);
        assertEquals(5, uniqueExtensions2.size());
        assertTrue(uniqueExtensions2.contains("123"));
        assertTrue(uniqueExtensions2.contains("567"));
    }
}
