package org.sipfoundry.sipxconfig.phonebook;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.context.ApplicationContext;

public class AddressBookEntryTestDb extends SipxDatabaseTestCase {

    private CoreContext m_core;

    @Override
    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_core = (CoreContext) app.getBean(CoreContext.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");

    }

    public void testGetBranchAddress() throws Exception {
        TestHelper.insertFlat("phonebook/AddressBookEntrySeed.xml");

        User user = m_core.loadUserByUserName("userseed1");
        assertNotNull(user);
        assertEquals(1, user.getAddressBookEntry().getId().intValue());

        Address officeAddress = user.getAddressBookEntry().getOfficeAddress();
        assertEquals(1, officeAddress.getId().intValue());
        assertNotNull(user.getBranch().getAddress());
        user.getAddressBookEntry().setBranchAddress(user.getBranch().getAddress());
        user.getAddressBookEntry().setUseBranchAddress(true);
        m_core.saveUser(user);

        assertEquals("branchAddress", user.getAddressBookEntry().getOfficeAddress().getStreet());

        officeAddress.setStreet("testAddress");
        m_core.saveUser(user);

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
        TestHelper.insertFlat("phonebook/AddressBookEntrySeed.xml");

        User user = m_core.loadUserByUserName("userseed1");
        assertNotNull(user);
        m_core.deleteUser(user);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phonebook/DeleteUserAddressBookEntry.db.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[null]", null);

        ITable expected = expectedRds.getTable("address");
        ITable actual = TestHelper.getConnection().createQueryTable("address",
                "select * from address");

        Assertion.assertEquals(expected, actual);
    }
}
