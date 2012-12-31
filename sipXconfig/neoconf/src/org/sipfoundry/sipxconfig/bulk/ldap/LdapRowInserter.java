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

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import javax.naming.directory.Attributes;
import javax.naming.directory.SearchResult;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.UserValidationUtils;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.GroupAutoAssign;
import org.sipfoundry.sipxconfig.vm.MailboxManager;

/**
 * Specialized version of row inserter for inserting users from LDAP searches LdapRowinserter
 */
public class LdapRowInserter extends RowInserter<SearchResult> {
    public static final String LDAP_SETTING = "ldap";

    private LdapManager m_ldapManager;
    private ConferenceBridgeContext m_conferenceBridgeContext;
    private CoreContext m_coreContext;
    private ForwardingContext m_forwardingContext;
    private MailboxManager m_mailboxManager;
    private Set<String> m_existingUserNames;
    private UserMapper m_userMapper;
    private AttrMap m_attrMap;
    private String m_domain;
    private Set<String> m_aliases;
    private Set<String> m_importedUserNames;

    private boolean m_preserveMissingUsers;

    @Override
    public void beforeInserting(Object... inputs) {
        // Make sure m_userMapper's AttrMap is set up.
        m_userMapper.setAttrMap(m_attrMap);
        // get all the users from LDAP group
        m_existingUserNames = new HashSet<String>();
        //initialize imported username set -
        //this will contain usernames successfuly imported in the current import session
        //we will use it to check for duplicated usernames
        m_importedUserNames = new HashSet<String>();
        Group defaultGroup = m_coreContext.getGroupByName(m_attrMap.getDefaultGroupName(),
                false);
        if (defaultGroup != null) {
            Collection<String> userNames = m_coreContext.getGroupMembersNames(defaultGroup);
            m_existingUserNames.addAll(userNames);
        }
    }

    @Override
    public void afterInserting() {
        if (m_preserveMissingUsers) {
            return;
        }
        // remove all the users that were not re-imported from LDAP
        m_coreContext.deleteUsersByUserName(m_existingUserNames);
        m_existingUserNames.clear();
        m_importedUserNames.clear();
    }

    @Override
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

            // disable user email notification
            user.setNotified(true);

            m_existingUserNames.remove(userName);

            m_userMapper.setUserProperties(user, attrs);
            m_userMapper.setAliasesSet(m_aliases, user);

            m_userMapper.setPin(user, attrs);
            m_userMapper.setVoicemailPin(user, attrs);
            m_userMapper.setSipPassword(user, attrs);

            Collection<String> groupNames = m_userMapper.getGroupNames(searchResult);

            //remove previous ldap groups
            Set<Group> groups = user.getGroups();
            List<Group> groupsToDelete = new ArrayList<Group>();
            for (Group group : groups) {
                if (new Boolean(group.getSettingValue(LDAP_SETTING))) {
                    groupsToDelete.add(group);
                }
            }
            user.getGroups().removeAll(groupsToDelete);
            // add all found groups
            for (String groupName : groupNames) {
                Group userGroup = m_coreContext.getGroupByName(groupName, true);
                userGroup.setSettingValue(LDAP_SETTING, "true");
                user.addGroup(userGroup);
            }

            user.setSettingValue(User.DOMAIN_SETTING, m_domain);

            if (newUser) {
                // Execute the automatic assignments for the user.
                GroupAutoAssign groupAutoAssign = new GroupAutoAssign(m_conferenceBridgeContext, m_coreContext,
                                                                      m_forwardingContext, m_mailboxManager);
                groupAutoAssign.assignUserData(user);
            } else {
                m_coreContext.saveUser(user);
            }
            m_importedUserNames.add(userName);
        } catch (Exception e) {
            LOG.error("Failed inserting row", e);
            throw new UserException(e);
        }
    }

    /**
     * Initial implementation will just print all attributes...
     */
    @Override
    protected String dataToString(SearchResult sr) {
        return sr.getName();
    }

    @Override
    protected RowResult checkRowData(SearchResult sr) {
        Attributes attrs = sr.getAttributes();
        String idAttrName = m_attrMap.getIdentityAttributeName();
        if (attrs.get(idAttrName) == null) {
            return new RowResult(RowStatus.FAILURE);
        }
        RowStatus status = RowStatus.SUCCESS;
        try {
            String userName = m_userMapper.getUserName(attrs);
            // check username
            if (!UserValidationUtils.isValidUserName(userName)
                || (m_importedUserNames != null && m_importedUserNames.contains(userName))) {
                return new RowResult(RowStatus.FAILURE);
            }
            Set<String> aliases = m_userMapper.getAliasesSet(attrs);
            if (aliases != null) {
                Set<String> aliasesToRemove = new TreeSet<String>();
                for (String alias : aliases) {
                    if (StringUtils.equals(userName, alias) || m_coreContext.isAliasInUseForOthers(alias, userName)) {
                        aliasesToRemove.add(alias);
                        status = RowStatus.WARNING_ALIAS_COLLISION;
                    }
                }
                if (!aliasesToRemove.isEmpty()) {
                    aliases.removeAll(aliasesToRemove);
                }
            }
            m_aliases = aliases;
        } catch (Exception e) {
            return new RowResult(RowStatus.FAILURE);
        }
        return new RowResult(status);
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
        return m_attrMap;
    }

    public void setDomain(String domain) {
        m_domain = domain;
    }
}
