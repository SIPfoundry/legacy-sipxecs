/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 *
 */
package org.sipfoundry.sipxconfig.common.event;

import static org.easymock.EasyMock.expectLastCall;
import static org.easymock.classextension.EasyMock.createStrictMock;
import static org.easymock.classextension.EasyMock.replay;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingImpl;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.vm.DistributionList;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.Voicemail;
import org.sipfoundry.sipxconfig.vm.attendant.PersonalAttendant;

public class PersonalAttendantSettingListenerTest extends TestCase {

    private static final String USER_CONSTANT = "user";
    private static final String PERSONAL_ATTENDANT_SETTING = "personal-attendant";
    private static final String OPERATOR_SETTING = "operator";
    private static final String GROUP_OPERATOR = "group-operator";

    private PersonalAttendantSettingListener m_out;
    private CoreContext m_coreContextMock;
    private MailboxManager m_mailboxManagerMock;
    private User m_firstUser;
    private User m_secondUser;
    private Group m_group;
    private Setting m_firstUserSetting;
    private Setting m_secondUserSetting;

    @Override
    public void setUp() {
        m_group = new Group();

        m_firstUserSetting = new SettingSet(USER_CONSTANT);
        Setting firstUserPaSetting = new SettingSet(PERSONAL_ATTENDANT_SETTING);
        m_firstUserSetting.addSetting(firstUserPaSetting);
        Setting firstUserOperatorSetting = new SettingImpl(OPERATOR_SETTING);
        firstUserPaSetting.addSetting(firstUserOperatorSetting);

        m_firstUser = new User();
        m_firstUser.setUniqueId();
        m_firstUser.setName("firstUser");
        m_firstUser.setSettings(m_firstUserSetting);

        m_secondUserSetting = new SettingSet(USER_CONSTANT);
        Setting secondUserPaSetting = new SettingSet(PERSONAL_ATTENDANT_SETTING);
        m_secondUserSetting.addSetting(secondUserPaSetting);
        Setting secondUserOperatorSetting = new SettingImpl(OPERATOR_SETTING);
        secondUserPaSetting.addSetting(secondUserOperatorSetting);

        m_secondUser = new User();
        m_secondUser.setUniqueId();
        m_secondUser.setName("secondUser");
        m_secondUser.setSettings(m_secondUserSetting);

        m_coreContextMock = createStrictMock(CoreContext.class);
        m_coreContextMock.getGroupMembers(m_group);
        expectLastCall().andReturn(Arrays.asList(new User[] {
            m_firstUser, m_secondUser
        }));
        replay(m_coreContextMock);

        m_mailboxManagerMock = new MockMailboxManager();

        PersonalAttendant firstUserAttendant = new PersonalAttendant();
        firstUserAttendant.setUser(m_firstUser);
        m_mailboxManagerMock.storePersonalAttendant(firstUserAttendant);

        PersonalAttendant secondUserAttendant = new PersonalAttendant();
        secondUserAttendant.setUser(m_secondUser);
        m_mailboxManagerMock.storePersonalAttendant(secondUserAttendant);

        m_out = new PersonalAttendantSettingListener();
        m_out.setCoreContext(m_coreContextMock);
        m_out.setMailboxManager(m_mailboxManagerMock);
    }

    public void testOnSaveGroup() {
        m_firstUser.addGroup(m_group);
        m_secondUser.addGroup(m_group);

        m_group.setSettingValue("personal-attendant/operator", GROUP_OPERATOR);
        m_out.onSave(m_group);

        assertEquals(GROUP_OPERATOR, m_mailboxManagerMock.loadPersonalAttendantForUser(m_firstUser).getOperator());
    }

    public void testOnSaveUserWithOverrideOperators() {
        m_firstUser.addGroup(m_group);
        m_firstUser.getSettings().getSetting("personal-attendant/operator").setValue("firstUserOperator");

        m_secondUser.addGroup(m_group);
        m_secondUser.getSettings().getSetting("personal-attendant/operator").setValue("secondUserOperator");

        m_out.onSave(m_firstUser);
        m_out.onSave(m_secondUser);
        assertEquals("firstUserOperator", m_mailboxManagerMock.loadPersonalAttendantForUser(m_firstUser)
                .getOperator());
        assertEquals("secondUserOperator", m_mailboxManagerMock.loadPersonalAttendantForUser(m_secondUser)
                .getOperator());
    }

    public void testOnSaveUserWithNoGroup() {
        m_firstUser.getSettings().getSetting("personal-attendant/operator").setValue("firstUserOperator");

        m_out.onSave(m_firstUser);
        assertEquals("firstUserOperator", m_mailboxManagerMock.loadPersonalAttendantForUser(m_firstUser)
                .getOperator());
    }

    private static class MockMailboxManager implements MailboxManager {

        private Map<User, PersonalAttendant> m_attendantMap;

        public MockMailboxManager() {
            m_attendantMap = new HashMap<User, PersonalAttendant>();
        }

        public void delete(Mailbox mailbox, Voicemail voicemail) {
        }

        public void deleteMailbox(String userId) {
        }

        public void renameMailbox(String oldUserId, String newUserId) {
        }

        public Mailbox getMailbox(String userId) {
            return null;
        }

        public String getMailstoreDirectory() {
            return null;
        }

        public String getStdpromptDirectory() {
            return null;
        }

        public List<Voicemail> getVoicemail(Mailbox mailbox, String folder) {
            return null;
        }

        public boolean isEnabled() {
            return false;
        }

        public boolean isSystemCpui() {
            return false;
        }

        public DistributionList[] loadDistributionLists(Mailbox mailbox) {
            return null;
        }

        public void markRead(Mailbox mailbox, Voicemail voicemail) {
        }

        public void move(Mailbox mailbox, Voicemail voicemail, String destinationFolderId) {
        }

        public void removePersonalAttendantForUser(User user) {
        }

        public void saveDistributionLists(Mailbox mailbox, DistributionList[] lists) {
        }

        /* The methods below this comment are the only ones relevant for this test */
        public PersonalAttendant loadPersonalAttendantForUser(User user) {
            return m_attendantMap.get(user);
        }

        public void storePersonalAttendant(PersonalAttendant pa) {
            m_attendantMap.put(pa.getUser(), pa);
        }

        public void clearPersonalAttendants() {
            m_attendantMap = new HashMap<User, PersonalAttendant>();
        }

        @Override
        public void updatePersonalAttendantForUser(User user, String operatorValue) {
            PersonalAttendant pa = loadPersonalAttendantForUser(user);
            pa.setOperator(operatorValue);
            storePersonalAttendant(pa);
        }

        public void writePreferencesFile(User user) {
        }
    }
}
