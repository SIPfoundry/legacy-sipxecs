/*
 * 
 * 
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.api;

import java.rmi.RemoteException;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DataCollectionUtil;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

public class UserServiceImpl implements UserService {
    
    /** TODO: Remove this when user loader uses lucene */
    private static final int PAGE_SIZE = 1000;
    
    private static final String GROUP_RESOURCE_ID = 
        org.sipfoundry.sipxconfig.common.User.GROUP_RESOURCE_ID;
    
    private static final String SORT_ORDER = "userName";
    
    private static final Log LOG = LogFactory.getLog(UserServiceImpl.class);
    
    private CoreContext m_coreContext;
    
    private SettingDao m_settingDao;
    
    private UserBuilder m_userBuilder;
    
    private MailboxManager m_mailboxManager;
    
    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void setUserBuilder(UserBuilder userTranslator) {
        m_userBuilder = userTranslator;
    }
    
    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    public void addUser(AddUser addUser) throws RemoteException {
        org.sipfoundry.sipxconfig.common.User myUser = new org.sipfoundry.sipxconfig.common.User();
        User apiUser = addUser.getUser();        
        ApiBeanUtil.toMyObject(m_userBuilder, myUser, apiUser);
        String[] groups = apiUser.getGroups();
        for (int i = 0; groups != null && i < groups.length; i++) {
            Group g = m_settingDao.getGroupCreateIfNotFound(GROUP_RESOURCE_ID, groups[i]);
            myUser.addGroup(g);
        }
        String emailAddress = apiUser.getEmailAddress();
        if (!StringUtils.isBlank(emailAddress)) {
            if (!m_mailboxManager.isEnabled()) {
                throw new IllegalArgumentException("Voicemail is not configured on this system");
            }
            
            MailboxPreferences mailboxPreferences = new MailboxPreferences();
            Mailbox mailbox = m_mailboxManager.getMailbox(myUser.getUserName());
            mailboxPreferences.setEmailAddress(emailAddress);
            m_mailboxManager.saveMailboxPreferences(mailbox, mailboxPreferences);
        }
        myUser.setPin(addUser.getPin(), m_coreContext.getAuthorizationRealm());
        m_coreContext.saveUser(myUser);
    }
    
    public FindUserResponse findUser(FindUser findUser) throws RemoteException {
        FindUserResponse response = new FindUserResponse();      
        UserSearch search = (findUser == null ? null : findUser.getSearch());
        org.sipfoundry.sipxconfig.common.User[] users = search(search);
        User[] arrayOfUsers = (User[]) ApiBeanUtil.toApiArray(m_userBuilder, users, User.class);
        loadEmailAddresses(arrayOfUsers);
        
        response.setUsers(arrayOfUsers);
        
        return response;
    }
    
    void loadEmailAddresses(User[] users) {
        if (!m_mailboxManager.isEnabled()) {
            return;
        }
        for (User user : users) {
            Mailbox mailbox = m_mailboxManager.getMailbox(user.getUserName());
            MailboxPreferences preferences = m_mailboxManager.loadMailboxPreferences(mailbox);
            user.setEmailAddress(preferences.getEmailAddress());
        }
    }

    org.sipfoundry.sipxconfig.common.User[] search(UserSearch search) {
        List users = Collections.EMPTY_LIST;
        if (search == null) {
            users = m_coreContext.loadUsers();
        } else if (search.getByUserName() != null) {
            org.sipfoundry.sipxconfig.common.User user = m_coreContext.loadUserByUserName(search.getByUserName());
            if (user != null) {
                users = Collections.singletonList(user);
            }
        } else if (search.getByFuzzyUserNameOrAlias() != null) {
            users = m_coreContext.loadUsersByPage(search.getByFuzzyUserNameOrAlias(),
                    null, 0, PAGE_SIZE, SORT_ORDER, true);                
            warnIfOverflow(users, PAGE_SIZE);
        } else if (search.getByGroup() != null) {
            Group g = m_settingDao.getGroupByName(GROUP_RESOURCE_ID, search.getByGroup());
            users = m_coreContext.loadUsersByPage(null, g.getId(), 0, PAGE_SIZE, SORT_ORDER, true);
            warnIfOverflow(users, PAGE_SIZE);
        }
        
        return (org.sipfoundry.sipxconfig.common.User[])
            users.toArray(new org.sipfoundry.sipxconfig.common.User[users.size()]);
    }
    
    void warnIfOverflow(List list, int size) {
        if (list.size() >= size) {
            LOG.warn("Search results exceeded maximum size " + PAGE_SIZE);
        }        
    }

    public void manageUser(ManageUser manageUser) throws RemoteException {
        org.sipfoundry.sipxconfig.common.User[] myUsers = search(manageUser.getSearch());
        for (int i = 0; i < myUsers.length; i++) {
            if (Boolean.TRUE.equals(manageUser.getDeleteUser())) {
                m_coreContext.deleteUser(myUsers[i]);
                continue; // no other edits make sense
            }
            if (manageUser.getEdit() != null) {
                User apiUser = new User();
                Set properties  = ApiBeanUtil.getSpecifiedProperties(manageUser.getEdit());
                ApiBeanUtil.setProperties(apiUser, manageUser.getEdit());
                m_userBuilder.toMyObject(myUsers[i], apiUser, properties);
                Property emailProperty = ApiBeanUtil.findProperty(manageUser.getEdit(), MailboxPreferences.EMAIL_PROP);
                if (emailProperty != null) {
                    Mailbox mailbox = m_mailboxManager.getMailbox(myUsers[i].getUserName());                    
                    MailboxPreferences preferences = m_mailboxManager.loadMailboxPreferences(mailbox);
                    preferences.setEmailAddress(emailProperty.getValue());
                    m_mailboxManager.saveMailboxPreferences(mailbox, preferences);
                }
            }

            if (manageUser.getAddGroup() != null) {                
                Group g = m_settingDao.getGroupCreateIfNotFound(GROUP_RESOURCE_ID, manageUser
                        .getAddGroup());
                myUsers[i].addGroup(g);
            }

            if (manageUser.getRemoveGroup() != null) {
                Group g = m_settingDao.getGroupByName(GROUP_RESOURCE_ID, manageUser.getRemoveGroup());
                if (g != null) {
                    DataCollectionUtil.removeByPrimaryKey(myUsers[i].getGroups(), g.getPrimaryKey());
                }
            }
            
            m_coreContext.saveUser(myUsers[i]);
        }
    }
}
