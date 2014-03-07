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
import java.util.Date;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import javax.naming.directory.Attributes;
import javax.naming.directory.SearchResult;

import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.DuplicateType;
import org.sipfoundry.sipxconfig.common.NameInUseException;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.common.UserValidationUtils;
import org.sipfoundry.sipxconfig.conference.ConferenceBridgeContext;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.GroupAutoAssign;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.springframework.beans.factory.annotation.Required;

/**
 * Specialized version of row inserter for inserting users from LDAP searches LdapRowinserter
 */
public class LdapRowInserter extends RowInserter<SearchResult> {
    public static final String LDAP_SETTING = "ldap";
    private static final Log LOG = LogFactory.getLog(LDAP_SETTING);
    private static final String EMPTY = "EMPTY";
    private static final String EMPTY_NO_USERNAME_MAP = "EMPTY_NO_MAPPING";

    private ConferenceBridgeContext m_conferenceBridgeContext;
    private CoreContext m_coreContext;
    private PermissionManager m_permissionManager;
    private ForwardingContext m_forwardingContext;
    private MailboxManager m_mailboxManager;
    private UserMapper m_userMapper;
    private AttrMap m_attrMap;
    private String m_domain;
    private Set<String> m_aliases;
    private List<String> m_importedUserNames;
    private List<String> m_notImportedUserNames;

    @Override
    public void beforeInserting(Object... inputs) {
        // Make sure m_userMapper's AttrMap is set up.
        m_userMapper.setAttrMap(m_attrMap);
        // initialize imported username set -
        // this will contain usernames successfuly imported in the current import session
        // we will use it to check for duplicated usernames
        m_importedUserNames = new ArrayList<String>();
        //we need this to trigger an alarm if at least one user was not imported
        m_notImportedUserNames = new ArrayList<String>();
    }

    @Override
    public void afterInserting() {
        m_importedUserNames.clear();
        m_notImportedUserNames.clear();
    }

    @Override
    protected void insertRow(SearchResult searchResult) {
        Attributes attrs = searchResult.getAttributes();
        LOG.info("Inserting:" + attrs.toString());
        insertRow(searchResult, attrs);
    }

    void insertRow(SearchResult searchResult, Attributes attrs) {
        User user = null;
        boolean newUser = false;
        String userNameWithDefault = null;
        try {
            String userName = m_userMapper.getUserName(attrs);
            userNameWithDefault = StringUtils.defaultIfEmpty(userName, EMPTY);
            user = m_coreContext.loadUserByUserName(userName);
            newUser = (user == null);
            if (newUser) {
                user = m_coreContext.newUser();
                user.setUserName(userName);
            } else if (!user.isLdapManaged()) {
                // this user was already created but it is not supposed to be managed by LDAP
                LOG.info("STOP Inserting:" + attrs.toString()
                    + " as this is an existing user which is not LDAP managed");
                m_notImportedUserNames.add(userNameWithDefault);
                return;
            }
            user.setEnabled(true);
            user.setPermissionManager(m_permissionManager);
            // set ldap import information
            user.setLdapManaged(true);
            user.setLastImportedDate(new Date());
            // disable user email notification
            user.setNotified(true);

            m_userMapper.setUserProperties(user, attrs);
            m_userMapper.setAliasesSet(m_aliases, user);

            m_userMapper.setPin(user, attrs);
            m_userMapper.setVoicemailPin(user, attrs);
            m_userMapper.setSipPassword(user, attrs);

            Collection<String> groupNames = m_userMapper.getGroupNames(searchResult);

            // remove previous ldap groups
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
            saveRow(newUser, user);
        } catch (Exception e) {
            if (e instanceof NameInUseException
                && ((NameInUseException) e).getDuplicateEntity().getType() == DuplicateType.USER_IM
                && user != null
                && !StringUtils.isEmpty(user.getUserDomain())
                && !StringUtils.isEmpty(user.getImId())) {
                LOG.error("Duplicate IM ID value: " + user.getImId() + " username: " + user.getUserName());
                user.setImId(user.getImId() + "." + user.getUserDomain() + "_" + m_importedUserNames.size());
                try {
                    saveRow(newUser, user);
                } catch (Exception ex) {
                    LOG.error(
                        "Failed inserting row with IM ID: "
                            + StringUtils.defaultIfEmpty(user.getImId(), StringUtils.EMPTY), e);
                    m_notImportedUserNames.add(userNameWithDefault);
                    throw new UserException(e);
                }
            } else {
                m_notImportedUserNames.add(userNameWithDefault);
                LOG.error("Failed inserting row", e);
                throw new UserException(e);
            }
        }
    }

    private void saveRow(boolean newUser, User user) {
        if (newUser) {
            // Execute the automatic assignments for the user.
            GroupAutoAssign groupAutoAssign = new GroupAutoAssign(m_conferenceBridgeContext, m_coreContext,
                m_forwardingContext, m_mailboxManager);
            groupAutoAssign.assignUserData(user);
        } else {
            m_coreContext.saveUser(user);
        }
        m_importedUserNames.add(user.getUserName());
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
            m_notImportedUserNames.add(StringUtils.defaultIfEmpty(idAttrName, EMPTY_NO_USERNAME_MAP));
            LOG.error("Cannot insert user - no username mapping " + m_notImportedUserNames.size());
            return new RowResult(RowStatus.FAILURE);
        }
        RowStatus status = RowStatus.SUCCESS;
        String userName = null;
        String userNameWithDefault = null;
        try {
            userName = m_userMapper.getUserName(attrs);
            userNameWithDefault = StringUtils.defaultIfEmpty(userName, EMPTY);
            // check username
            if (!UserValidationUtils.isValidUserName(userName)
                || (m_importedUserNames != null && m_importedUserNames.contains(userName))) {
                LOG.error("Cannot insert username: " + userNameWithDefault);
                m_notImportedUserNames.add(userNameWithDefault);
                return new RowResult(RowStatus.FAILURE);
            }
            Set<String> aliases = m_userMapper.getAliasesSet(attrs);
            if (aliases != null) {
                Set<String> aliasesToRemove = new TreeSet<String>();
                for (String alias : aliases) {
                    if (StringUtils.equals(userName, alias) || m_coreContext.isAliasInUseForOthers(alias, userName)) {
                        aliasesToRemove.add(alias);
                        LOG.warn("Remove aliases for username: " + userNameWithDefault);
                        status = RowStatus.WARNING_ALIAS_COLLISION;
                    }
                }
                if (!aliasesToRemove.isEmpty()) {
                    aliases.removeAll(aliasesToRemove);
                }
            }
            m_aliases = aliases;
        } catch (Exception e) {
            LOG.error("Cannot insert user with username: " + userNameWithDefault);
            m_notImportedUserNames.add(userNameWithDefault);
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

    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }

    public void setUserMapper(UserMapper userMapper) {
        m_userMapper = userMapper;
    }

    private AttrMap getAttrMap() {
        return m_attrMap;
    }

    public void setDomain(String domain) {
        m_domain = domain;
    }

    @Required
    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    public List<String> getImportedUserNames() {
        return m_importedUserNames;
    }

    public List<String> getNotImportedUserNames() {
        return m_notImportedUserNames;
    }
}
