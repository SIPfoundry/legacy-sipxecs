/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.speeddial;

import java.util.Collections;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.springframework.context.ApplicationContext;

public class SpeedDialManagerTestDb extends SipxDatabaseTestCase {

    private SpeedDialManager m_sdm;
    private CoreContext m_coreContext;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_sdm = (SpeedDialManager) appContext.getBean(SpeedDialManager.CONTEXT_BEAN_NAME);
        m_coreContext = (CoreContext) appContext.getBean(CoreContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testGetSpeedDialForUser() throws Exception {
        TestHelper.insertFlat("speeddial/speeddial.db.xml");
        SpeedDial speedDial = m_sdm.getSpeedDialForUserId(1001, true);
        assertNotNull(speedDial);
        assertEquals(3, speedDial.getButtons().size());
        assertEquals("222", speedDial.getButtons().get(2).getNumber());
        assertFalse(speedDial.getButtons().get(0).isBlf());
        assertTrue(speedDial.getButtons().get(1).isBlf());
    }

    public void testGetNewSpeedDialForUser() throws Exception {
        TestHelper.insertFlat("speeddial/speeddial.db.xml");
        SpeedDial speedDial = m_sdm.getSpeedDialForUserId(1002, false);
        assertNull(speedDial);
        speedDial = m_sdm.getSpeedDialForUserId(1002, true);
        assertNotNull(speedDial);
        assertEquals(0, speedDial.getButtons().size());
    }

    public void testSaveSpeedDialForUser() throws Exception {
        TestHelper.insertFlat("speeddial/speeddial.db.xml");
        SpeedDial speedDial = m_sdm.getSpeedDialForUserId(1002, true);

        final int buttonCount = 5;
        for (int i = 0; i < buttonCount; i++) {
            Button b = new Button();
            b.setLabel("testSave");
            b.setNumber(String.valueOf(i));
            b.setBlf(i % 2 == 0);
            speedDial.getButtons().add(b);
        }

        m_sdm.saveSpeedDial(speedDial);
        assertEquals(buttonCount, getConnection().getRowCount("speeddial_button",
                "WHERE label = 'testSave'"));
    }

    public void testOnDeleteUser() throws Exception {
        TestHelper.insertFlat("speeddial/speeddial.db.xml");        
        assertEquals(3, getConnection().getRowCount("speeddial_button"));
        assertEquals(1, getConnection().getRowCount("speeddial"));
        m_coreContext.deleteUsers(Collections.singleton(1001));
        assertEquals(0, getConnection().getRowCount("speeddial_button"));
        assertEquals(0, getConnection().getRowCount("speeddial"));
    }

}
