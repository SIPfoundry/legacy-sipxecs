/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.bulk.ldap;

import java.util.Collection;
import java.util.HashSet;
import java.util.Set;

import javax.naming.NamingException;
import javax.naming.directory.Attributes;
import javax.naming.directory.SearchResult;

import org.apache.commons.lang.RandomStringUtils;
import org.sipfoundry.sipxconfig.admin.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.GroupAutoAssign;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

/**
 * Specialized version of row inserter for inserting users from LDAP searches LdapRowinserter
 */
public class LdapRowInserter extends RowInserter<SearchResult> {
    private LdapManager m_ldapManager;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private CoreContext m_coreContext;
    private ForwardingContext m_forwardingContext;
    private MailboxManager m_mailboxManager;
    private Set<String> m_existingUserNames;
    private UserMapper m_userMapper;
    private AttrMap m_attrMap;

    private boolean m_preserveMissingUsers;

    public void beforeInserting() {
        // Make sure m_userMapper's AttrMap is set up.
        m_userMapper.setAttrMap(m_ldapManager.getAttrMap());
        // get all the users from LDAP group
        m_existingUserNames = new HashSet<String>();
        Group defaultGroup = m_coreContext.getGroupByName(getAttrMap().getDefaultGroupName(),
                false);
        if (defaultGroup != null) {
            Collection<String> userNames = m_coreContext.getGroupMembersNames(defaultGroup);
            m_existingUserNames.addAll(userNames);
        }
    }

    public void afterInserting() {
        if (m_preserveMissingUsers) {
            return;
        }
        // remove all the users that were not re-imported from LDAP
        m_coreContext.deleteUsersByUserName(m_existingUserNames);
        m_existingUserNames.clear();
    }

    protected void insertRow(SearchResult searchResult) {
        Attributes attrs = searchResult.getAttributes();
        LOG.info("Inserting:" + attrs.toString());
        insertRow(searchResult, attrs);
    }

    void insertRow(SearchResult searchResult, Attributes attrs) {
        try {
            String userName = m_userMapper.getUserName(attrs);
            User user = m_coreContext.loadUserByUserName(userName);
            boolean newUser = user == null;
            if (newUser) {
                user = m_coreContext.newUser();
                user.setUserName(userName);
            }
            m_existingUserNames.remove(userName);

            m_userMapper.setUserProperties(user, attrs);
            String pin = m_userMapper.getPin(attrs, newUser);
            if (pin != null) {
                user.setPin(pin, m_coreContext.getAuthorizationRealm());
            }

            String sipPassword = m_userMapper.getSipPassword(attrs);
            if (sipPassword == null && newUser) {
                user.setSipPassword(RandomStringUtils.randomAlphanumeric(8));
            }

            Collection<String> groupNames = m_userMapper.getGroupNames(searchResult);

            // add all found groups
            for (String groupName : groupNames) {
                Group userGroup = m_coreContext.getGroupByName(groupName, true);
                user.addGroup(userGroup);
            }
            m_coreContext.saveUser(user);

            if (newUser) {
                // Execute the automatic assignments for the user.
                GroupAutoAssign groupAutoAssign = new GroupAutoAssign(m_conferenceBridgeContext, m_coreContext,
                                                                      m_forwardingContext, m_mailboxManager);
                groupAutoAssign.assignUserData(user);
            }
        } catch (NamingException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Initial implementation will just print all attributes...
     */
    protected String dataToString(SearchResult sr) {
        return sr.getName();
    }

    protected RowStatus checkRowData(SearchResult sr) {
        Attributes attrs = sr.getAttributes();
        String idAttrName = m_attrMap.getIdentityAttributeName();
        return attrs.get(idAttrName) != null ? RowStatus.SUCCESS
                : RowStatus.FAILURE;
    }

    public void setAttrMap(AttrMap attrMap) {
        m_attrMap = attrMap;
    }

    public void setConferenceBridgeContext(ConferenceBridgeContext conferenceBridgeContext) {
        m_conferenceBridgeContext = conferenceBridgeContext;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setForwardingContext(ForwardingContext forwardingContext) {
        m_forwardingContext = forwardingContext;
    }

    public void setPreserveMissingUsers(boolean removeMissingUsers) {
        m_preserveMissingUsers = removeMissingUsers;
    }

    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    public void setUserMapper(UserMapper userMapper) {
        m_userMapper = userMapper;
    }

    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }

    private AttrMap getAttrMap() {
        if (m_attrMap != null) {
            return m_attrMap;
        }

        m_attrMap = m_ldapManager.getAttrMap();
        return m_attrMap;
    }
}
