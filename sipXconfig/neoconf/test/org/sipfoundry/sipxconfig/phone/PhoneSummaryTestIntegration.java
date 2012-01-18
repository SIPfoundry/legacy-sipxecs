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

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import org.sipfoundry.sipxconfig.test.IntegrationTestCase;


public class PhoneSummaryTestIntegration extends IntegrationTestCase {
    private PhoneContext m_phoneContext;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
    }

    public void testLoad() throws Exception {
        sql("common/TestUserSeed.sql");
        loadDataSet("phone/PhoneSummarySeed.xml");

        List<Phone> summaries = m_phoneContext.loadPhones();

        Comparator<Phone> idSort = new Comparator<Phone>() {
            public int compare(Phone phone1, Phone phone2){
                return phone1.getId() - phone2.getId();
            }
        };

        Collections.sort(summaries, idSort);

        assertEquals(3, summaries.size());
        Phone[] summariesArray = summaries.toArray(new Phone[0]);

        assertEquals("unittest-sample phone1", summariesArray[0].getDescription());
        assertEquals(1, summariesArray[0].getLines().size());

        assertEquals("unittest-sample phone2", summariesArray[1].getDescription());
        assertEquals(0, summariesArray[1].getLines().size());

        assertEquals("unittest-sample phone3", summariesArray[2].getDescription());
        assertEquals(2, summariesArray[2].getLines().size());
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }
}
