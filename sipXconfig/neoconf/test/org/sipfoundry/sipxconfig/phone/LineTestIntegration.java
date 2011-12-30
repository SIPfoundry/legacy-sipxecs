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


import static org.junit.Assert.assertArrayEquals;

import java.util.Collection;
import java.util.Collections;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.acme.AcmePhone;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.ResultDataGrid;
import org.sipfoundry.sipxconfig.test.TestHelper;

public class LineTestIntegration extends IntegrationTestCase {
    private PhoneContext m_phoneContext;
    private CoreContext m_coreContext;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        clear();
        sql("domain/DomainSeed.sql");
        sql("common/TestUserSeed.sql");
    }
    
    @Override
    protected void onSetUpInTransaction() throws Exception {
    }

    public void testAddingLine() throws Exception {
        loadDataSet("phone/AddLineSeed.xml");

        Phone phone = m_phoneContext.loadPhone(new Integer(1000));
        assertEquals(2, phone.getLines().size());
        User user = m_coreContext.loadUserByUserName("testuser");

        Line thirdLine = phone.createLine();
        thirdLine.setUser(user);
        phone.addLine(thirdLine);
        m_phoneContext.storePhone(phone);
        commit();

        // reload data to get updated ids
        m_phoneContext.flush();
        
        Phone reloadedPhone = m_phoneContext.loadPhone(new Integer(1000));
        Line reloadedThirdLine = reloadedPhone.getLine(2);
        ResultDataGrid actual = new ResultDataGrid();
        db().query("select line_id, position, user_id from line where phone_id = ? order by position", actual, 1000);
        Object[][] expected = new Object[][] {
                {1000, 0, 1000},
                {1001, 1, 1000},
                {reloadedThirdLine.getPrimaryKey(), 2, 1000}
        };
        assertArrayEquals(expected, actual.toArray());
    }

    public void testAddingLineByContext() throws Exception {
        loadDataSet("phone/AddLineSeed.xml");

        Integer phoneId = new Integer(1000);
        Phone phone = m_phoneContext.loadPhone(phoneId);
        assertEquals(2, phone.getLines().size());
        User user = m_coreContext.loadUserByUserName("testuser");

        m_phoneContext.addUsersToPhone(phoneId, Collections.singleton(user.getId()));
        commit();

        // reload data to get updated ids
        m_phoneContext.flush();
        Phone reloadedPhone = m_phoneContext.loadPhone(phoneId);
        Line reloadedThirdLine = reloadedPhone.getLine(2);

        ResultDataGrid actual = new ResultDataGrid();
        db().query("select line_id, position, user_id from line where phone_id = ? order by position", actual, 1000);
        Object[][] expected = new Object[][] {
                {1000, 0, 1000},
                {1001, 1, 1000},
                {reloadedThirdLine.getPrimaryKey(), 2, 1000}
        };
        assertArrayEquals(expected, actual.toArray());
    }

    public void testSave() throws Exception {
        loadDataSet("phone/EndpointSeed.xml");

        Phone phone = m_phoneContext.loadPhone(new Integer(1000));
        assertEquals(0, phone.getLines().size());
        User user = m_coreContext.loadUserByUserName("testuser");

        Line line = phone.createLine();
        line.setUser(user);
        phone.addLine(line);
        m_phoneContext.storePhone(phone);

        // reload data to get updated ids
        m_phoneContext.flush();
        Phone reloadedPhone = m_phoneContext.loadPhone(new Integer(1000));
        Line reloadedLine = reloadedPhone.getLine(0);

        ResultDataGrid actual = new ResultDataGrid();
        db().query("select line_id, position, user_id from line where phone_id = ? order by position", actual, 1000);
        Object[][] expected = new Object[][] {
                {reloadedLine.getPrimaryKey(), 0, 1000}
        };
        assertArrayEquals(expected, actual.toArray());
    }

    public void testLoadAndDelete() throws Exception {
        loadDataSet("phone/LineSeed.xml");

        Phone phone = m_phoneContext.loadPhone(new Integer(1000));
        Collection lines = phone.getLines();
        assertEquals(1, lines.size());

        Line line = (Line) lines.iterator().next();
        line.getSettings();
        m_phoneContext.storePhone(phone);

        lines.clear();
        m_phoneContext.storePhone(phone);

        Phone cleared = m_phoneContext.loadPhone(new Integer(1000));
        assertEquals(0, cleared.getLines().size());
    }

    public void testMoveLine() throws Exception {
        loadDataSet("phone/MoveLineSeed.xml");

        Phone phone = m_phoneContext.loadPhone(new Integer(1000));
        Line l1 = phone.getLine(0);
        Object[] ids = new Object[] {
            l1.getPrimaryKey()
        };
        DataCollectionUtil.moveByPrimaryKey(phone.getLines(), ids, 1);
        m_phoneContext.storePhone(phone);
        commit();
        m_phoneContext.flush();
        
        ResultDataGrid actual = new ResultDataGrid();
        db().query("select line_id, position, user_id from line where phone_id = ? order by position", actual, 1000);
        Object[][] expected = new Object[][] {
                {1001, 0, 1000},
                {1000, 1, 1000},
                {1002, 2, 1000}
        };
        assertArrayEquals(expected, actual.toArray());
    }

    /**
     * Makes sure the line's settings get deleted too
     */
    public void testDeleteLinesWithSettings() throws Exception {
        loadDataSet("phone/DeleteLineWithSettingsSeed.xml");

        Phone phone = m_phoneContext.loadPhone(new Integer(1000));
        Collection lines = phone.getLines();
        assertEquals(3, lines.size());
        DataCollectionUtil.removeByPrimaryKey(lines, new Object[] {
            new Integer(1001)
        });
        m_phoneContext.storePhone(phone);
        commit();
        ResultDataGrid actual = new ResultDataGrid();
        db().query("select line_id, position, value_storage_id from line where phone_id = ? order by position", actual, 1000);
        Object[][] expected = new Object[][] {
                {1000, 0, null},
                {1002, 1, null}
        };
        assertArrayEquals(expected, actual.toArray());
    }

    public void testNoLinesButOtherPhonesHaveLines() throws Exception {
        loadDataSet("phone/LineSeed.xml");
        Phone newPhone = m_phoneContext.newPhone(new TestPhoneModel());
        newPhone.setSerialNumber("aaaaaaaaaaaa");
        m_phoneContext.storePhone(newPhone);
        Phone loadedPhone = m_phoneContext.loadPhone(newPhone.getId());
        assertEquals(0, loadedPhone.getLines().size());
    }

    /**
     * Hits the db indirectly thru domainManager
     */
    public void testGetLineInfo() {
        Phone phone = new AcmePhone();
        phone.setModel(new PhoneModel("acmePhone"));
        PhoneContext context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        phone.setPhoneContext(context);
        phone.setModelFilesContext(TestHelper.getModelFilesContext());
        Line line = phone.createLine();
        User u = new User();
        u.setUserName("turkey");
        line.setUser(u);
        phone.addLine(line);
        assertEquals("turkey", line.getLineInfo().getUserId());
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
