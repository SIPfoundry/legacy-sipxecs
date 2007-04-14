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

import java.lang.reflect.InvocationTargetException;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.Attribute;
import javax.naming.directory.Attributes;
import javax.naming.directory.SearchResult;
import javax.naming.ldap.LdapName;
import javax.naming.ldap.Rdn;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.bulk.RowInserter;
import org.sipfoundry.sipxconfig.bulk.csv.Index;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.vm.Mailbox;
import org.sipfoundry.sipxconfig.vm.MailboxManager;
import org.sipfoundry.sipxconfig.vm.MailboxPreferences;

/**
 * Specialized version of row inserter for inserting users from LDAP searches LdapRowinserter
 */
public class LdapRowInserter extends RowInserter<SearchResult> {

    private AttrMap m_attrMap;

    private CoreContext m_coreContext;
    
    private MailboxManager m_mailboxManager;

    private Set<String> m_existingUserNames;

    private boolean m_preserveMissingUsers;

    public void beforeInserting() {
        // get all the users from LDAP group
        m_existingUserNames = new HashSet<String>();
        Group defaultGroup = m_coreContext.getGroupByName(m_attrMap.getDefaultGroupName(), false);
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

    /**
     * Initial implementation will just print all attributes...
     */
    protected String dataToString(SearchResult sr) {
        return sr.getName();
    }

    protected void insertRow(SearchResult sr) {
        try {
            Attributes attrs = sr.getAttributes();
            LOG.info("Inserting:" + attrs.toString());
            String userName = getUserName(attrs);
            User user = m_coreContext.loadUserByUserName(userName);
            boolean newUser = user == null;
            if (newUser) {
                user = m_coreContext.newUser();
                user.setUserName(userName);
            }
            m_existingUserNames.remove(userName);

            setUserProperties(user, attrs);
            String pin = getPin(attrs, newUser);
            if (pin != null) {
                user.setPin(pin, m_coreContext.getAuthorizationRealm());
            }

            Collection<String> groupNames = getGroupNames(sr);

            // add all found groups
            for (String groupName : groupNames) {
                Group userGroup = m_coreContext.getGroupByName(groupName, true);
                user.addGroup(userGroup);
            }
            m_coreContext.saveUser(user);
            
            MailboxPreferences mboxPrefs = getMailboxPreferences(attrs);
            if (mboxPrefs != null) {
                Mailbox mailbox = m_mailboxManager.getMailbox(user.getUserName());               
                m_mailboxManager.saveMailboxPreferences(mailbox, mboxPrefs);
            }
        } catch (NamingException e) {
            throw new RuntimeException(e);
        }
    }

    protected boolean checkRowData(SearchResult sr) {
        Attributes attrs = sr.getAttributes();
        String idAttrName = m_attrMap.getIdentityAttributeName();
        return (attrs.get(idAttrName) != null);
    }

    /**
     * Sets user properties based on the attributes of the found LDAP entry
     */
    void setUserProperties(User user, Attributes attrs) throws NamingException {
        // in most cases userName is already set - this code is here to support retrieving user
        // previe
        String userName = user.getUserName();
        if (StringUtils.isBlank(userName)) {
            user.setUserName(getUserName(attrs));
        }
        setProperty(user, attrs, Index.FIRST_NAME);
        setProperty(user, attrs, Index.LAST_NAME);
        setProperty(user, attrs, Index.SIP_PASSWORD);

        Set<String> aliases = getValues(attrs, Index.ALIAS);
        if (aliases != null) {
            user.copyAliases(aliases);
        }
    }
    
    MailboxPreferences getMailboxPreferences(Attributes attrs) throws NamingException {
        String emailAddress = getValue(attrs, Index.EMAIL);
        if (!m_mailboxManager.isEnabled() || StringUtils.isBlank(emailAddress)) {
            return null;
        }
        
        String userId = getValue(attrs, Index.USERNAME);
        Mailbox mailbox = m_mailboxManager.getMailbox(userId);
        MailboxPreferences mboxPrefs = m_mailboxManager.loadMailboxPreferences(mailbox);
        mboxPrefs.setEmailAddress(emailAddress);
        return mboxPrefs;
    }

    Collection<String> getGroupNames(SearchResult sr) throws NamingException {
        Set<String> groupNames = new HashSet<String>();
        String defaultGroupName = m_attrMap.getDefaultGroupName();
        if (defaultGroupName != null) {
            groupNames.add(defaultGroupName);
        }

        // group names in the current entry
        Attributes attrs = sr.getAttributes();
        Set<String> entryGroups = getValues(attrs, Index.USER_GROUP);
        if (entryGroups != null) {
            groupNames.addAll(entryGroups);
        }

        // group names found in distinguished name
        if (sr.isRelative()) {
            String name = sr.getName();
            LdapName ldapName = new LdapName(name);
            List<Rdn> rdns = ldapName.getRdns();
            for (Rdn rdn : rdns) {
                Attributes rdnsAttributes = rdn.toAttributes();
                Set<String> rdnsGroups = getValues(rdnsAttributes, Index.USER_GROUP);
                if (rdnsGroups != null) {
                    groupNames.addAll(rdnsGroups);
                }

            }
        }
        return groupNames;
    }

    private void setProperty(User user, Attributes attrs, Index index) throws NamingException {
        try {
            String value = getValue(attrs, index);
            if (value != null) {
                BeanUtils.setProperty(user, index.getName(), value);
            }
        } catch (IllegalAccessException e) {
            throw new RuntimeException(e);
        } catch (InvocationTargetException e) {
            throw new RuntimeException(e.getCause());
        }
    }

    private String getValue(Attributes attrs, Index index) throws NamingException {
        String attrName = m_attrMap.userProperty2ldapAttribute(index.getName());
        if (attrName == null) {
            // no attribute for this property - nothing to do
            return null;
        }
        return getValue(attrs, attrName);
    }

    private Set<String> getValues(Attributes attrs, Index index) throws NamingException {
        String attrName = m_attrMap.userProperty2ldapAttribute(index.getName());
        if (attrName == null) {
            // no attribute for this property - nothing to do
            return null;
        }
        return getValues(attrs, attrName);
    }

    /**
     * Returns single value for an attribute, even if attribute has more values...
     * 
     * @param attrs collection of attributes
     * @param attr attribute name
     */
    private String getValue(Attributes attrs, String attrName) throws NamingException {
        Attribute attribute = attrs.get(attrName);
        if (attribute == null) {
            return null;
        }
        Object value = attribute.get();
        if (value == null) {
            return null;
        }
        return value.toString();
    }

    /**
     * Returns all string values for an attribute with a given name, ignores the values that are
     * not string values
     * 
     * @param attrs collection of attributes
     * @param attr attribute name
     */
    private Set<String> getValues(Attributes attrs, String attrName) throws NamingException {
        Attribute attribute = attrs.get(attrName);
        if (attribute == null) {
            return null;
        }
        Set<String> values = new TreeSet<String>();
        NamingEnumeration< ? > allValues = attribute.getAll();
        while (allValues.hasMore()) {
            Object object = allValues.nextElement();
            if (object instanceof String) {
                values.add((String) object);
            }
        }
        return values;
    }

    private String getPin(Attributes attrs, boolean newUser) throws NamingException {
        String pin = getValue(attrs, Index.PIN);
        if (pin == null && newUser) {
            // for new users consider default pin
            pin = m_attrMap.getDefaultPin();
        }
        return pin;
    }

    private String getUserName(Attributes attrs) throws NamingException {
        String attrName = m_attrMap.getIdentityAttributeName();
        String userName = getValue(attrs, attrName);
        return userName;
    }

    public void setAttrMap(AttrMap attrMap) {
        m_attrMap = attrMap;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setPreserveMissingUsers(boolean removeMissingUsers) {
        m_preserveMissingUsers = removeMissingUsers;
    }

    public void setMailboxManager(MailboxManager mailboxManager) {
        m_mailboxManager = mailboxManager;
    }
}
