/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.commserver.imdb;

import static org.sipfoundry.commons.mongo.MongoConstants.BUTTONS;
import static org.sipfoundry.commons.mongo.MongoConstants.DIALPAD;
import static org.sipfoundry.commons.mongo.MongoConstants.DISTRIB_LISTS;
import static org.sipfoundry.commons.mongo.MongoConstants.ITEM;
import static org.sipfoundry.commons.mongo.MongoConstants.LANGUAGE;
import static org.sipfoundry.commons.mongo.MongoConstants.OPERATOR;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.commons.mongo.MongoConstants;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DialPad;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.dialplan.AttendantMenu;
import org.sipfoundry.sipxconfig.dialplan.AttendantMenuAction;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.test.TestHelper;
import org.sipfoundry.sipxconfig.vm.AbstractMailboxManager;
import org.sipfoundry.sipxconfig.vm.DistributionList;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences.ActiveGreeting;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;

import com.mongodb.BasicDBObject;
import com.mongodb.DBObject;

public class MailstoreTestIntegration extends ImdbTestCase {
    private MailboxManager m_mailboxManager;
    private User m_user;
    private SettingDao m_settingDao;

    @Override
    protected void onSetUpBeforeTransaction() {
        m_user = getCoreContext().newUser();
        m_user.setUserName("200");
    }

    public void testGenerate() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        loadDataSetXml("commserver/seedLocations.xml");
        getCoreContext().saveUser(m_user);
        PersonalAttendant pa = m_mailboxManager.loadPersonalAttendantForUser(m_user);
        pa.setLanguage("ro");
        pa.setOverrideLanguage(true);
        AttendantMenu menu = new AttendantMenu();
        menu.addMenuItem(DialPad.NUM_0, AttendantMenuAction.TRANSFER_OUT, "222");
        pa.setMenu(menu);
        m_user.setSettingValue(MailboxPreferences.ACTIVE_GREETING, ActiveGreeting.EXTENDED_ABSENCE.toString());
        m_user.setSettingValue("personal-attendant/operator", "111");
        m_mailboxManager.storePersonalAttendant(pa);
        getCoreContext().saveUser(m_user);

        DBObject search = new BasicDBObject();
        search.put(OPERATOR, "sip:111@" + getDomainManager().getDomainName());
        search.put(LANGUAGE, "ro");
        List<DBObject> buttonsList = new ArrayList<DBObject>();
        DBObject menuItem = new BasicDBObject();
        menuItem.put(DIALPAD, "0");
        menuItem.put(ITEM, "sip:222@" + getDomainManager().getDomainName());
        buttonsList.add(menuItem);
        search.put(BUTTONS, buttonsList);

        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), new BasicDBObject(
                MongoConstants.PERSONAL_ATT, search));

        DistributionList dl1 = new DistributionList();

        dl1.setExtensions(new String[] {
            "202"
        });

        DistributionList dl2 = new DistributionList();
        dl2.setExtensions(new String[] {
            "203", "204"
        });

        DistributionList[] dls = DistributionList.createBlankList();
        dls[1] = dl1;
        dls[3] = dl2;
        try {
            m_mailboxManager.saveDistributionLists(m_user.getId(), dls);
            fail();
        } catch (UserException e) {

        }

        User u202 = getCoreContext().newUser();
        u202.setUserName("202");
        u202.setPermission(PermissionName.VOICEMAIL, false);

        User u203 = getCoreContext().newUser();
        u203.setUserName("203");

        User u204 = getCoreContext().newUser();
        u204.setUserName("204");

        getCoreContext().saveUser(u202);
        getCoreContext().saveUser(u203);
        getCoreContext().saveUser(u204);

        try {
            m_mailboxManager.saveDistributionLists(m_user.getId(), dls);
            fail();
        } catch (UserException e) {

        }

        u202.setPermission(PermissionName.VOICEMAIL, true);
        getCoreContext().saveUser(u202);
        m_mailboxManager.saveDistributionLists(u202.getId(), dls);
        List<DBObject> dLists = new ArrayList<DBObject>();

        DBObject dlist1 = new BasicDBObject();
        dlist1.put(DIALPAD, 1);
        dlist1.put(ITEM, "202");
        dLists.add(dlist1);

        DBObject dlist2 = new BasicDBObject();
        dlist2.put(DIALPAD, 3);
        dlist2.put(ITEM, "203 204");
        dLists.add(dlist2);
        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), new BasicDBObject(DISTRIB_LISTS, dLists).append("uid", "202"));

        //TODO: test for other stuff in Mailstore dataset, like email, etc
    }

    public void testPAforGroup() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        loadDataSetXml("commserver/seedLocations.xml");
        Group group = new Group();
        group.setSettingValue("personal-attendant/operator", "123");
        group.setResource(CoreContext.USER_GROUP_RESOURCE_ID);
        group.setName("group");
        m_settingDao.saveGroup(group);


        User user = getCoreContext().newUser();
        user.setUserName("200");
        user.addGroup(m_settingDao.getGroupByName(CoreContext.USER_GROUP_RESOURCE_ID, "group"));
        getCoreContext().saveUser(user);

        DBObject user200 = new BasicDBObject().append(UID, "200");
        DBObject search = new BasicDBObject()
            .append(OPERATOR, "sip:123@" + getDomainManager().getDomainName())
            .append("lng", "en");
        user200.put(MongoConstants.PERSONAL_ATT, search);

        MongoTestCaseHelper.assertObjectPresent(getEntityCollection(), user200);
    }

    public void setMailboxManager(MailboxManager localMailboxManager) {
        m_mailboxManager = localMailboxManager;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }
}
