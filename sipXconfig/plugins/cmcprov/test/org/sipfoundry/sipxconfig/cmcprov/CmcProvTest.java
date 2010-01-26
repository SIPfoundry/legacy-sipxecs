/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.cmcprov;

import java.io.File;
import java.net.URI;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.sipfoundry.sipxconfig.cmcprov.ContactSynchronizer;

public class CmcProvTest extends TestCase {
    private ContactSynchronizer m_handler;

    protected void setUp()
    {
       String src = getClass().getResource("phonebook.xml").getFile();

       m_handler =  ContactSynchronizer.getInstance(src, src + ".contacts.xml");
    }

    public void testSynchronize() throws Exception
    {
        m_handler.synChronize();
        String expected = IOUtils.toString(getClass().getResourceAsStream("expectedResult.xml"));
        String result = IOUtils.toString(getClass().getResourceAsStream("phonebook.xml.contacts.xml"));

        assertEquals(expected, result);
    }

    protected void tearDown()
    {
        File mergedFile = new File(getClass().getResource("phonebook.xml.contacts.xml").getFile());
        mergedFile.delete();
    }

}
