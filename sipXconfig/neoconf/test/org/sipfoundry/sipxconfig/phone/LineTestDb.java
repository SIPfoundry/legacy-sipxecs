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
import java.util.Collections;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.acme.AcmePhone;

public class LineTestDb extends SipxDatabaseTestCase {

    private PhoneContext m_context;

    private CoreContext m_core;

    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        m_core = (CoreContext) TestHelper.getApplicationContext().getBean(
                CoreContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
    }

    public void testAddingLine() throws Exception {
        TestHelper.cleanInsertFlat("phone/AddLineSeed.xml");

        Phone phone = m_context.loadPhone(new Integer(1000));
        assertEquals(2, phone.getLines().size());
        User user = m_core.loadUserByUserName("testuser");

        Line thirdLine = phone.createLine();
        thirdLine.setUser(user);
        phone.addLine(thirdLine);
        m_context.storePhone(phone);

        // reload data to get updated ids
        m_context.flush();
        Phone reloadedPhone = m_context.loadPhone(new Integer(1000));
        Line reloadedThirdLine = reloadedPhone.getLine(2);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/AddLineExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[line_id]", reloadedThirdLine.getPrimaryKey());
        expectedRds.addReplacementObject("[null]", null);

        ITable expected = expectedRds.getTable("line");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");

        Assertion.assertEquals(expected, actual);
    }

    public void testAddingLineByContext() throws Exception {
        TestHelper.cleanInsertFlat("phone/AddLineSeed.xml");

        Integer phoneId = new Integer(1000);
        Phone phone = m_context.loadPhone(phoneId);
        assertEquals(2, phone.getLines().size());
        User user = m_core.loadUserByUserName("testuser");

        m_context.addUsersToPhone(phoneId, Collections.singleton(user.getId()));

        // reload data to get updated ids
        m_context.flush();
        Phone reloadedPhone = m_context.loadPhone(phoneId);
        Line reloadedThirdLine = reloadedPhone.getLine(2);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/AddLineExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[line_id]", reloadedThirdLine.getPrimaryKey());
        expectedRds.addReplacementObject("[null]", null);

        ITable expected = expectedRds.getTable("line");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");

        Assertion.assertEquals(expected, actual);
    }

    public void testSave() throws Exception {
        TestHelper.cleanInsertFlat("phone/EndpointSeed.xml");

        Phone phone = m_context.loadPhone(new Integer(1000));
        assertEquals(0, phone.getLines().size());
        User user = m_core.loadUserByUserName("testuser");

        Line line = phone.createLine();
        line.setUser(user);
        phone.addLine(line);
        m_context.storePhone(phone);

        // reload data to get updated ids
        m_context.flush();
        Phone reloadedPhone = m_context.loadPhone(new Integer(1000));
        Line reloadedLine = reloadedPhone.getLine(0);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/SaveLineExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);
        expectedRds.addReplacementObject("[line_id]", reloadedLine.getPrimaryKey());
        ITable expected = expectedRds.getTable("line");

        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");

        Assertion.assertEquals(expected, actual);
    }

    public void testLoadAndDelete() throws Exception {
        TestHelper.cleanInsertFlat("phone/LineSeed.xml");

        Phone phone = m_context.loadPhone(new Integer(1000));
        Collection lines = phone.getLines();
        assertEquals(1, lines.size());

        Line line = (Line) lines.iterator().next();
        line.getSettings();
        m_context.storePhone(phone);

        lines.clear();
        m_context.storePhone(phone);

        Phone cleared = m_context.loadPhone(new Integer(1000));
        assertEquals(0, cleared.getLines().size());
    }

    public void testMoveLine() throws Exception {
        TestHelper.cleanInsertFlat("phone/MoveLineSeed.xml");

        Phone phone = m_context.loadPhone(new Integer(1000));
        Line l1 = phone.getLine(0);
        Object[] ids = new Object[] {
            l1.getPrimaryKey()
        };
        DataCollectionUtil.moveByPrimaryKey(phone.getLines(), ids, 1);
        m_context.storePhone(phone);
        m_context.flush();

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/MoveLineExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);

        ITable expected = expectedRds.getTable("line");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");

        Assertion.assertEquals(expected, actual);
    }

    /**
     * Makes sure the line's settings get deleted too
     */
    public void testDeleteLinesWithSettings() throws Exception {
        TestHelper.cleanInsertFlat("phone/DeleteLineWithSettingsSeed.xml");

        Phone phone = m_context.loadPhone(new Integer(1000));
        Collection lines = phone.getLines();
        assertEquals(3, lines.size());
        DataCollectionUtil.removeByPrimaryKey(lines, new Object[] {
            new Integer(1001)
        });
        m_context.storePhone(phone);

        IDataSet expectedDs = TestHelper
                .loadDataSetFlat("phone/DeleteLineWithSettingsExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);

        ITable expected = expectedRds.getTable("line");
        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");

        Assertion.assertEquals(expected, actual);
    }

    public void testNoLinesButOtherPhonesHaveLines() throws Exception {
        TestHelper.cleanInsertFlat("phone/LineSeed.xml");
        Phone newPhone = m_context.newPhone(new TestPhoneModel());
        newPhone.setSerialNumber("aaaaaaaaaaaa");
        m_context.storePhone(newPhone);
        Phone loadedPhone = m_context.loadPhone(newPhone.getId());
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
}
