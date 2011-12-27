/*
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 */
package org.sipfoundry.sipxconfig.phonebook;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.test.IntegrationTestCase;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.springframework.context.ApplicationContext;

public class AddressBookEntryTestDb extends IntegrationTestCase {
    private CoreContext m_coreContext;

    @Override
    protected void onSetUpBeforeTransaction() throws Exception {
        super.onSetUpBeforeTransaction();
        db().execute("select truncate_all()");
    }
    
    @Override
    protected void onSetUpInTransaction() throws Exception {
        super.onSetUpInTransaction();
        sql("commserver/SeedLocations.sql");
    }

    public void testGetBranchAddress() throws Exception {
        sql("phonebook/AddressBookEntrySeed.sql");

        User user = m_coreContext.loadUserByUserName("userseed1");
        assertNotNull(user);
        assertEquals(1, user.getAddressBookEntry().getId().intValue());

        Address officeAddress = user.getAddressBookEntry().getOfficeAddress();
        assertEquals(1, officeAddress.getId().intValue());
        assertNotNull(user.getBranch().getAddress());
        user.getAddressBookEntry().setBranchAddress(user.getBranch().getAddress());
        user.getAddressBookEntry().setUseBranchAddress(true);
        m_coreContext.saveUser(user);

        assertEquals("branchAddress", user.getAddressBookEntry().getOfficeAddress().getStreet());

        officeAddress.setStreet("testAddress");
        m_coreContext.saveUser(user);

        assertEquals("branchAddress", user.getAddressBookEntry().getOfficeAddress().getStreet());

        user.getAddressBookEntry().setUseBranchAddress(false);
        user.getAddressBookEntry().getOfficeAddress().setStreet("personalAddress");
        assertEquals("personalAddress", user.getAddressBookEntry().getOfficeAddress().getStreet());

        user.getAddressBookEntry().setUseBranchAddress(true);
        assertEquals("branchAddress", user.getAddressBookEntry().getOfficeAddress().getStreet());

        user.getAddressBookEntry().setBranchAddress(null);
        user.getAddressBookEntry().getOfficeAddress().setStreet("personalAddress");
        assertEquals("personalAddress", user.getAddressBookEntry().getOfficeAddress().getStreet());
    }

    public void testDeleteUser() throws Exception {
        sql("phonebook/AddressBookEntrySeed.sql");

        User user = m_coreContext.loadUserByUserName("userseed1");
        assertNotNull(user);
        m_coreContext.deleteUser(user);
        flush();
        assertEquals(1, db().queryForInt("select count(*) from address"));
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
