/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.phonebook;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.Collections;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.site.phonebook.PhonebookResource.PhonebookCsv;

public class PhonebookResourceTest extends TestCase {
    
    public void testCsv() throws IOException {
        IMocksControl entryControl = EasyMock.createControl();
        PhonebookEntry entry = entryControl.createMock(PhonebookEntry.class);
        entry.getFirstName();
        entryControl.andReturn("Penguin");
        entry.getLastName();
        entryControl.andReturn("Emporer");
        entry.getNumber();        
        entryControl.andReturn("happyfeet@southpole.org");
        entryControl.replay();

        ByteArrayOutputStream actual = new ByteArrayOutputStream();
        PhonebookCsv csv = new PhonebookCsv(Collections.singleton(entry));
        csv.write(actual);        
        
        assertEquals("\"First name\",\"Last name\",\"Number\"\n" +
                "\"Penguin\",\"Emporer\",\"happyfeet@southpole.org\"\n", actual.toString());
        
        entryControl.verify();
        
    }

}
