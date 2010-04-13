/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class BulkManagerImplTestDb extends SipxDatabaseTestCase {
    private BulkManager m_bulkManager;

    protected void setUp() throws Exception {
        super.setUp();
        ApplicationContext context = TestHelper.getApplicationContext();
        m_bulkManager = (BulkManager) context.getBean("bulkManagerDao");
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testInsertFromCsvEmpty() throws Exception {
        m_bulkManager.insertFromCsv(new StringReader(""));
        assertEquals(0, getConnection().getRowCount("users"));
        assertEquals(0, getConnection().getRowCount("phone"));
        assertEquals(0, getConnection().getRowCount("line"));
        assertEquals(0, getConnection().getRowCount("user_group"));
        assertEquals(0, getConnection().getRowCount("phone_group"));
    }

    public void testInsertFromCsvNameDuplication() throws Exception {
        // users with duplicated names should be overwritten
        InputStream cutsheet = getClass().getResourceAsStream("dup_names.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(2, getConnection().getRowCount("users"));
        assertEquals(3, getConnection().getRowCount("phone"));
        assertEquals(3, getConnection().getRowCount("line"));
        assertEquals(2, getConnection().getRowCount("user_group"));
        assertEquals(3, getConnection().getRowCount("phone_group"));
        assertEquals(1, getConnection().getRowCount("group_storage", "where resource = 'phone'"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'user'"));
    }

    public void testInsertFromCsvAliasDuplication() throws Exception {
        // second user has a duplicated alias - it should be ignored, but remaining users have to
        // be imported
        InputStream cutsheet = getClass().getResourceAsStream("errors.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(2, getConnection().getRowCount("users"));
        assertEquals(2, getConnection().getRowCount("phone"));
        assertEquals(2, getConnection().getRowCount("line"));
        assertEquals(2, getConnection().getRowCount("user_group"));
        assertEquals(2, getConnection().getRowCount("phone_group"));
        assertEquals(1, getConnection().getRowCount("group_storage", "where resource = 'phone'"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'user'"));
    }

    public void testInsertFromCsvPhoneDuplication() throws Exception {
        // users with duplicated names should be overwritten
        InputStream cutsheet = getClass().getResourceAsStream("dup_phones.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(5, getConnection().getRowCount("users"));
        assertEquals(4, getConnection().getRowCount("phone"));
        assertEquals(5, getConnection().getRowCount("line"));
        assertEquals(5, getConnection().getRowCount("user_group"));
        assertEquals(4, getConnection().getRowCount("phone_group"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'phone'"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'user'"));
    }

    public void testInsertFromCsv() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("cutsheet.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(5, getConnection().getRowCount("users"));
        assertEquals(5, getConnection().getRowCount("phone"));
        assertEquals(5, getConnection().getRowCount("line"));
        assertEquals(5, getConnection().getRowCount("user_group"));
        assertEquals(5, getConnection().getRowCount("phone_group"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'phone'"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'user'"));
    }

    public void testInsertFromCsvDuplicate() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("cutsheet.csv");
        cutsheet.mark(-1);
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        // and try again
        cutsheet.reset();
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(5, getConnection().getRowCount("users"));
        assertEquals(5, getConnection().getRowCount("phone"));
        // lines are updated, if this value is 10, lines are erroneously being duplicated
        assertEquals(5, getConnection().getRowCount("line"));
        assertEquals(5, getConnection().getRowCount("user_group"));
        assertEquals(5, getConnection().getRowCount("phone_group"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'phone'"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'user'"));
    }

    public void testInsertFromCsvUserNameAliasConflict() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("user_alias_conflict.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(1, getConnection().getRowCount("users"));
    }

    public void testInsertFromCsvBlankPhoneGroup() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("blank_phonegroup.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(1, getConnection().getRowCount("users"));
        assertEquals(1, getConnection().getRowCount("phone"));
        assertEquals(0, getConnection().getRowCount("phone_group"));
        assertEquals(1, getConnection().getRowCount("user_group"));
    }

    public void testInsertFromCsvBlankUserGroup() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("blank_usergroup.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(1, getConnection().getRowCount("users"));
        assertEquals(1, getConnection().getRowCount("phone"));
        assertEquals(1, getConnection().getRowCount("phone_group"));
        assertEquals(0, getConnection().getRowCount("user_group"));
    }
}
