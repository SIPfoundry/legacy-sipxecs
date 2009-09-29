/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.intercom;

import java.util.Collections;
import java.util.List;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.dialplan.DialingRule;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.context.ApplicationContext;

public class IntercomManagerImplTestDb extends SipxDatabaseTestCase {
    private static final String PREFIX_DEFAULT = "*76"; // keep in sync with intercom.beans.xml
    private static final int TIMEOUT_DEFAULT = 5000;

    private IntercomManager m_intercomManager;
    private PhoneContext m_phoneContext;
    private SettingDao m_settingsDao;

    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_intercomManager = (IntercomManagerImpl) app
                .getBean(IntercomManagerImpl.CONTEXT_BEAN_NAME);
        m_phoneContext = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        m_settingsDao = (SettingDao) TestHelper.getApplicationContext().getBean(
                SettingDao.CONTEXT_NAME);

        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testNewIntercom() {
        Intercom intercom = m_intercomManager.newIntercom();
        assertNotNull(intercom);
    }

    public void testGetIntercom() {
        assertEquals(0, m_intercomManager.loadIntercoms().size());
        Intercom intercom = m_intercomManager.getIntercom();
        assertNotNull(intercom); // new Intercom must be created automatically

        // check the defaults
        assertEquals(PREFIX_DEFAULT, intercom.getPrefix());
        assertEquals(TIMEOUT_DEFAULT, intercom.getTimeout());
        assertEquals(null, intercom.getCode());

        // save the new Intercom, then we should be able to load it
        intercom.setCode(""); // can't save the Intercom with a null code
        m_intercomManager.saveIntercom(intercom);
        Intercom intercom2 = m_intercomManager.getIntercom();
        assertEquals(intercom.getId(), intercom2.getId());
    }

    public void testSaveIntercom() throws Exception {
        TestHelper.insertFlat("phone/SeedPhoneGroup.xml");

        // create the intercom
        Intercom intercom = m_intercomManager.newIntercom();
        final String PREFIX = "77";
        intercom.setPrefix(PREFIX);
        final int TIMEOUT = 123;
        intercom.setTimeout(TIMEOUT);
        final String CODE = "whatever";
        intercom.setCode(CODE);

        List groups = m_phoneContext.getGroups();
        intercom.addGroup((Group) groups.get(0));
        intercom.addGroup((Group) groups.get(1));

        // save the intercom
        m_intercomManager.saveIntercom(intercom);

        // load it back up and check it
        List intercoms = m_intercomManager.loadIntercoms();
        assertEquals(1, intercoms.size());
        intercom = (Intercom) intercoms.get(0);
        assertEquals(PREFIX, intercom.getPrefix());
        assertEquals(TIMEOUT, intercom.getTimeout());
        assertEquals(CODE, intercom.getCode());
        groups = intercom.getGroupsAsList();
        assertEquals(2, groups.size());
    }

    public void testDeleteGroup() throws Exception {
        TestHelper.insertFlat("phone/SeedPhoneGroup.xml");

        // create the intercom
        Intercom intercom = m_intercomManager.newIntercom();
        final String PREFIX = "77";
        intercom.setPrefix(PREFIX);
        final int TIMEOUT = 123;
        intercom.setTimeout(TIMEOUT);
        final String CODE = "whatever";
        intercom.setCode(CODE);

        List<Group> groups = m_phoneContext.getGroups();
        intercom.addGroup(groups.get(0));
        intercom.addGroup(groups.get(1));

        // save the intercom
        m_intercomManager.saveIntercom(intercom);

        m_settingsDao.deleteGroups(Collections.singleton(groups.get(0).getId()));

        // load it back up and check it
        List intercoms = m_intercomManager.loadIntercoms();
        assertEquals(1, intercoms.size());
        intercom = (Intercom) intercoms.get(0);
        assertEquals(PREFIX, intercom.getPrefix());
        assertEquals(TIMEOUT, intercom.getTimeout());
        assertEquals(CODE, intercom.getCode());
        groups = intercom.getGroupsAsList();
        assertEquals(1, groups.size());
    }


    public void testLoadIntercoms() throws Exception {
        // verify loading the sample data
        TestHelper.insertFlat("admin/intercom/SampleIntercoms.xml");
        List intercoms = m_intercomManager.loadIntercoms();
        assertEquals(2, intercoms.size());
        Intercom i1 = (Intercom) intercoms.get(0);
        assertEquals("66", i1.getPrefix());
        Intercom i2 = (Intercom) intercoms.get(1);
        assertEquals(4000, i2.getTimeout());
    }

    public void testGetNumIntercoms() throws Exception {
        assertEquals(0, m_intercomManager.loadIntercoms().size());
        TestHelper.insertFlat("admin/intercom/SampleIntercoms.xml");
        assertEquals(2, m_intercomManager.loadIntercoms().size());
    }

    public void testGetIntercomForPhone() throws Exception {
        // load some sample intercoms and phones
        TestHelper.insertFlat("admin/intercom/SampleIntercoms.xml");
        TestHelper.insertFlat("phone/SamplePhoneSeed.xml");

        // play with linking phones to intercoms

        // load a phone and verify that it has no intercom
        Phone phone1 = m_phoneContext.loadPhone(1000);
        assertNotNull(phone1);
        assertNull(m_intercomManager.getIntercomForPhone(phone1));

        // give it an intercom
        List phone1Groups = phone1.getGroupsAsList();
        Group group1 = (Group) phone1Groups.get(0);
        Intercom intercom = m_intercomManager.loadIntercoms().get(0);
        intercom.addGroup(group1);
        m_intercomManager.saveIntercom(intercom);
        assertEquals(intercom, m_intercomManager.getIntercomForPhone(phone1));

        // check a couple of other phones, one in the same group and one not in the same group
        Phone phone2 = m_phoneContext.loadPhone(1001);
        assertNull(m_intercomManager.getIntercomForPhone(phone2));
        Phone phone3 = m_phoneContext.loadPhone(1002);
        assertEquals(intercom, m_intercomManager.getIntercomForPhone(phone3));
    }

    public void testGetRules() throws Exception {
        TestHelper.insertFlat("admin/intercom/SampleIntercoms.xml");
        List< ? extends DialingRule> rules = m_intercomManager.getDialingRules();
        assertEquals(2, rules.size());
    }

    public void testClear() throws Exception {
        // load some sample intercoms
        TestHelper.insertFlat("admin/intercom/SampleIntercoms.xml");

        // blow them all away
        m_intercomManager.clear();

        // they should be gone
        List intercoms = m_intercomManager.loadIntercoms();
        assertEquals(0, intercoms.size());
    }
}
