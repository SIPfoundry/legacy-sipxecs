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
import java.util.List;

import org.apache.commons.lang.RandomStringUtils;
import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.device.DeviceVersion;
import org.sipfoundry.sipxconfig.device.ModelSource;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.springframework.context.ApplicationContext;
import org.springframework.orm.hibernate3.HibernateObjectRetrievalFailureException;

public class PhoneTestDb extends SipxDatabaseTestCase {

    private PhoneContext context;

    private SettingDao settingDao;

    private CoreContext core;

    private ModelSource<PhoneModel> phoneModelSource;

    @Override
    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        context = (PhoneContext) app.getBean(PhoneContext.CONTEXT_BEAN_NAME);
        settingDao = (SettingDao) app.getBean(SettingDao.CONTEXT_NAME);
        core = (CoreContext) app.getBean(CoreContext.CONTEXT_BEAN_NAME);
        phoneModelSource = (ModelSource<PhoneModel>) app.getBean("nakedPhoneModelSource");

        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testSave() throws Exception {
        Phone phone = context.newPhone(new TestPhoneModel());
        phone.setSerialNumber("999123456789");
        phone.setDescription("unittest-sample phone1");
        context.storePhone(phone);

        ITable actual = TestHelper.getConnection().createDataSet().getTable("phone");

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/SaveEndpointExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[phone_id_1]", phone.getId());
        expectedRds.addReplacementObject("[null]", null);
        ITable expected = expectedRds.getTable("phone");
        Assertion.assertEquals(expected, actual);
    }

    public void testLoadAndDelete() throws Exception {
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        TestHelper.cleanInsertFlat("phone/EndpointLineSeed.xml");

        Phone p = context.loadPhone(new Integer(1000));
        assertEquals("999123456789", p.getSerialNumber());

        Integer id = p.getId();
        context.deletePhone(p);
        try {
            context.loadPhone(id);
            fail();
        } catch (HibernateObjectRetrievalFailureException x) {
            assertTrue(true);
        }

        IDataSet actual = TestHelper.getConnection().createDataSet();
        assertEquals(0, actual.getTable("phone").getRowCount());
        assertEquals(0, actual.getTable("line").getRowCount());
    }

    public void testUpdateSettings() throws Exception {
        TestHelper.cleanInsertFlat("phone/EndpointSeed.xml");

        Phone p = context.loadPhone(new Integer(1000));
        p.setSettingValue("server/outboundProxy", "bigbird");
        context.storePhone(p);
        context.flush();

        Phone reloadPhone = context.loadPhone(new Integer(1000));
        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/UpdateSettingsExpected.xml");
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);

        ValueStorage s = (ValueStorage) reloadPhone.getValueStorage();
        assertNotNull(s);
        expectedRds.addReplacementObject("[value_storage_id]", s.getId());

        IDataSet actual = TestHelper.getConnection().createDataSet();
        Assertion.assertEquals(expectedRds.getTable("setting_value"), actual.getTable("setting_value"));
    }

    public void testAddGroup() throws Exception {
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        TestHelper.cleanInsertFlat("phone/EndpointSeed.xml");
        TestHelper.cleanInsertFlat("phone/SeedPhoneGroup.xml");

        Phone p = context.loadPhone(new Integer(1000));
        List groups = context.getGroups();
        p.addGroup((Group) groups.get(0));
        context.storePhone(p);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/AddGroupExpected.xml");
        IDataSet actual = TestHelper.getConnection().createDataSet();
        Assertion.assertEquals(expectedDs.getTable("phone_group"), actual.getTable("phone_group"));
    }

    public void testRemoveGroupThenAddBackThenAddAnotherGroup() throws Exception {
        TestHelper.cleanInsertFlat("phone/EndpointSeed.xml");
        TestHelper.cleanInsertFlat("phone/SeedPhoneGroup.xml");

        Phone p = context.loadPhone(new Integer(1000));
        List groups = context.getGroups();
        p.addGroup((Group) groups.get(0));
        context.storePhone(p);
        p = null;

        Phone reloaded = context.loadPhone(new Integer(1000));
        reloaded.getGroups().clear();
        reloaded.addGroup((Group) groups.get(0));
        reloaded.addGroup((Group) groups.get(1));
        context.storePhone(reloaded);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/AddSecondGroupExpected.xml");
        IDataSet actual = TestHelper.getConnection().createDataSet();
        Assertion.assertEquals(expectedDs.getTable("phone_group"), actual.getTable("phone_group"));
    }

    public void testPhoneSubclassSave() throws Exception {
        PhoneModel model = new PhoneModel("acmePhone", "acmePhoneStandard");
        Phone subclass = context.newPhone(model);
        subclass.setSerialNumber("000000000000");
        context.storePhone(subclass);

        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/PhoneSubclassSaveExpected.xml");
        IDataSet actual = TestHelper.getConnection().createDataSet();
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[phone_id]", subclass.getId());
        expectedRds.addReplacementObject("[null]", null);

        Assertion.assertEquals(expectedRds.getTable("phone"), actual.getTable("phone"));
    }

    public void testPhoneSubclassDelete() throws Exception {
        TestHelper.cleanInsertFlat("phone/PhoneSubclassSeed.xml");
        Phone subclass = context.loadPhone(new Integer(1000));
        subclass.setSerialNumber("000000000000");
        context.deletePhone(subclass);

        IDataSet actual = TestHelper.getConnection().createDataSet();
        assertEquals(0, actual.getTable("phone").getRowCount());
    }

    public void testClear() throws Exception {
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        TestHelper.cleanInsertFlat("phone/EndpointLineSeed.xml");
        context.clear();
    }

    public void testDeletePhoneGroups() throws Exception {
        TestHelper.cleanInsertFlat("phone/GroupMemberCountSeed.xml");
        settingDao.deleteGroups(Collections.singletonList(new Integer(1001)));
        // link table references removed
        ITable actual = TestHelper.getConnection().createDataSet().getTable("phone_group");
        // just phone 2 and group 2
        assertEquals(1, actual.getRowCount());
    }

    public void testPhonesByUserId() throws Exception {
        TestHelper.cleanInsertFlat("common/TestUserSeed.db.xml");
        TestHelper.cleanInsertFlat("phone/EndpointLineSeed.xml");
        Collection phones = context.getPhonesByUserId(new Integer(1000));
        assertEquals(1, phones.size());
        Phone p = (Phone) phones.iterator().next();
        assertEquals("unittest-sample phone1", p.getDescription());
    }

    public void testPhonesByUserIdAndPhoneModel() throws Exception {
        TestHelper.cleanInsertFlat("common/TestUserSeed.db.xml");
        TestHelper.cleanInsertFlat("phone/EndpointLineSeed.xml");
        Collection phones = context.getPhonesByUserIdAndPhoneModel(new Integer(1000), "testPhoneModel");
        assertEquals(1, phones.size());
        Phone p = (Phone) phones.iterator().next();
        assertEquals("unittest-sample phone1", p.getDescription());
        phones = context.getPhonesByUserIdAndPhoneModel(new Integer(1000), "unknownModel");
        assertEquals(0, phones.size());
    }

    public void testDeleteUserRemoveLines() throws Exception {
        TestHelper.cleanInsertFlat("common/TestUserSeed.db.xml");
        TestHelper.cleanInsertFlat("phone/EndpointLineSeed.xml");
        User testUser = core.loadUser(new Integer(1000));
        core.deleteUser(testUser);

        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");
        assertEquals(0, actual.getRowCount());
    }

    public void testDeleteUserOnPhoneWithExternalLines() throws Exception {
        TestHelper.cleanInsertFlat("common/TestUserSeed.db.xml");
        TestHelper.cleanInsertFlat("phone/ExternalLineSeed.xml");
        User testUser = core.loadUser(new Integer(1000));
        core.deleteUser(testUser);

        ITable actual = TestHelper.getConnection().createDataSet().getTable("line");
        assertEquals(1, actual.getRowCount());

        // no primary user after user is deleted
        Phone phone = context.loadPhone(1000);
        assertNull(phone.getPrimaryUser());
    }

    public void testDeviceVersion() throws Exception {
        TestHelper.cleanInsertFlat("phone/PhoneVersionSeed.db.xml");
        Phone phone = context.loadPhone(1000);
        DeviceVersion deviceVersion = phone.getDeviceVersion();
        assertNotNull(deviceVersion);
        assertEquals("acmePhone", deviceVersion.getVendorId());
        assertEquals("1", deviceVersion.getVersionId());

        context.storePhone(phone);
    }

    public void testGetPrimaryUser() throws Exception {
        TestHelper.insertFlat("common/TestUserSeed.db.xml");
        TestHelper.cleanInsertFlat("phone/EndpointLineSeed.xml");
        Phone phone = context.loadPhone(1000);
        User user = phone.getPrimaryUser();
        assertNotNull(user);
        assertEquals(1000, user.getId().intValue());
    }

    public void testGetPrimaryUserNoUser() throws Exception {
        TestHelper.cleanInsertFlat("phone/PhoneVersionSeed.db.xml");
        Phone phone = context.loadPhone(1000);
        assertNull(phone.getPrimaryUser());
    }

    public void testPopulatePhones() throws Exception {
        for (PhoneModel model : phoneModelSource.getModels()) {
            Phone phone = context.newPhone(model);
            phone.setSerialNumber(RandomStringUtils.randomNumeric(12));
            context.storePhone(phone);
        }
    }
}
