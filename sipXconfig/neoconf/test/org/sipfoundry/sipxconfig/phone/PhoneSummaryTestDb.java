/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.Collection;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;


public class PhoneSummaryTestDb extends SipxDatabaseTestCase {

    private PhoneContext m_context;
    
    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);        
    }
    
    public void testLoad() throws Exception {        
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        TestHelper.cleanInsertFlat("phone/PhoneSummarySeed.xml");
        
        Collection summaries = m_context.loadPhones();
        
        assertEquals(3, summaries.size());
        Phone[] summariesArray = (Phone[]) summaries.toArray(new Phone[0]);

        assertEquals("unittest-sample phone1", summariesArray[0].getDescription());
        assertEquals(1, summariesArray[0].getLines().size());

        assertEquals("unittest-sample phone2", summariesArray[1].getDescription());
        assertEquals(0, summariesArray[1].getLines().size());

        assertEquals("unittest-sample phone3", summariesArray[2].getDescription());
        assertEquals(2, summariesArray[2].getLines().size());
    }
}
