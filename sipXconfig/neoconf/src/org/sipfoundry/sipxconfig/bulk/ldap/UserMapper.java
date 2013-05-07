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
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.TreeSet;

import javax.naming.NameClassPair;
import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.Attribute;
import javax.naming.directory.Attributes;
import javax.naming.directory.SearchResult;
import javax.naming.ldap.LdapName;
import javax.naming.ldap.Rdn;

import org.apache.commons.beanutils.BeanUtils;
import org.apache.commons.lang.RandomStringUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.bulk.UserPreview;
import org.sipfoundry.sipxconfig.bulk.csv.Index;
import org.sipfoundry.sipxconfig.common.User;
import org.springframework.ldap.NameClassPairMapper;

public class UserMapper implements NameClassPairMapper {
    private LdapManager m_ldapManager;
    private AttrMap m_attrMap;

    /**
     * @return UserPreview
     */
    @Override
    public Object mapFromNameClassPair(NameClassPair nameClass) throws NamingException {
        SearchResult searchResult = (SearchResult) nameClass;
        Attributes attrs = searchResult.getAttributes();
        User user = new User();
        List<String> groupNames = new ArrayList<String>(getGroupNames(searchResult));

        setUserProperties(user, attrs);
        setAliasesSet(getAliasesSet(attrs), user);
        UserPreview preview = new UserPreview(user, groupNames);
        return preview;
    }
    /**
     * Sets user properties based on the attributes of the found LDAP entry
     */
    void setUserProperties(User user, Attributes attrs) throws NamingException {
        // in most cases userName is already set - this code is here to support retrieving user
        // preview
        String userName = user.getUserName();
        if (StringUtils.isBlank(userName)) {
            user.setUserName(getUserName(attrs));
        }
        setProperty(user, attrs, Index.FIRST_NAME);
        setProperty(user, attrs, Index.LAST_NAME);
        setProperty(user, attrs, Index.SIP_PASSWORD);
        setProperty(user, attrs, Index.IM_ID);
        setProperty(user, attrs, Index.EMAIL);
        setProperty(user, attrs, Index.JOB_TITLE);
        setProperty(user, attrs, Index.JOB_DEPT);
        setProperty(user, attrs, Index.COMPANY_NAME);
        setProperty(user, attrs, Index.ASSISTANT_NAME);
        setProperty(user, attrs, Index.CELL_PHONE_NUMBER);
        setProperty(user, attrs, Index.HOME_PHONE_NUMBER);
        setProperty(user, attrs, Index.ASSISTANT_PHONE_NUMBER);
        setProperty(user, attrs, Index.FAX_NUMBER);
        setProperty(user, attrs, Index.ALTERNATE_EMAIL);
        setProperty(user, attrs, Index.ALTERNATE_IM_ID);
        setProperty(user, attrs, Index.LOCATION);
        setProperty(user, attrs, Index.HOME_STREET);
        setProperty(user, attrs, Index.HOME_CITY);
        setProperty(user, attrs, Index.HOME_STATE);
        setProperty(user, attrs, Index.HOME_COUNTRY);
        setProperty(user, attrs, Index.HOME_ZIP);
        setProperty(user, attrs, Index.OFFICE_STREET);
        setProperty(user, attrs, Index.OFFICE_CITY);
        setProperty(user, attrs, Index.OFFICE_STATE);
        setProperty(user, attrs, Index.OFFICE_COUNTRY);
        setProperty(user, attrs, Index.OFFICE_ZIP);
    }

    public void setAliasesSet(Set<String> aliases, User user) {
        if (aliases != null) {
            user.copyAliases(deleteWhitespace(aliases));
        }
    }

    public Set<String> getAliasesSet(Attributes attrs) throws NamingException {
        return getValues(attrs, Index.ALIAS);
    }

    public void setPin(User user, Attributes attrs) throws NamingException {
        String pin = getPin(attrs, user.isNew());
        if (pin != null) {
            user.setPin(pin);
        }
    }

    public void setVoicemailPin(User user, Attributes attrs) throws NamingException {
        String voicemailPin = getVoicemailPin(attrs, user.isNew());
        OverwritePinBean overwritePinBean = m_ldapManager.retriveOverwritePin();
        boolean overwritePin = (overwritePinBean == null || overwritePinBean.isValue());
        if (voicemailPin != null && (user.isNew() || overwritePin)) {
            user.setVoicemailPin(voicemailPin);
        }
    }

    public void setSipPassword(User user, Attributes attrs) throws NamingException {
        String sipPassword = getSipPassword(attrs);
        if (sipPassword == null && user.isNew()) {
            user.setSipPassword(RandomStringUtils.randomAlphanumeric(12));
        }
    }

    public Collection<String> getGroupNames(SearchResult sr) throws NamingException {
        Set<String> groupNames = new HashSet<String>();
        // group names in the current entry
        Attributes attrs = sr.getAttributes();
        Set<String> entryGroups = replaceWhitespace(getValues(attrs, Index.USER_GROUP));
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
                Set<String> rdnsGroups = replaceWhitespace(getValues(rdnsAttributes, Index.USER_GROUP));
                if (rdnsGroups != null) {
                    groupNames.addAll(rdnsGroups);
                }

            }
        }
        //only if there is no already defined group, add the default user group
        if (groupNames.isEmpty()) {
            String defaultGroupName = getAttrMap().getDefaultGroupName();
            if (defaultGroupName != null) {
                groupNames.add(defaultGroupName);
            }
        }
        return groupNames;
    }

    private Set<String> deleteWhitespace(Set<String> values) {
        if (values != null) {
            Set<String> withoutWhitespaces = new HashSet<String>();
            for (String value : values) {
                withoutWhitespaces.add(StringUtils.deleteWhitespace(value));
            }

            return withoutWhitespaces;
        }

        return null;
    }

    private Set<String> replaceWhitespace(Set<String> values) {
        if (values != null) {
            Set<String> userGroupNames = new HashSet<String>();
            for (String value : values) {
                userGroupNames.add(StringUtils.replace(value.trim(), " ", "_"));
            }

            return userGroupNames;
        }

        return null;
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
        String attrName = getAttrMap().userProperty2ldapAttribute(index.getName());
        if (attrName == null) {
            // no attribute for this property - nothing to do
            return null;
        }
        return getValue(attrs, attrName);
    }

    private Set<String> getValues(Attributes attrs, Index index) throws NamingException {
        String attrName = getAttrMap().userProperty2ldapAttribute(index.getName());
        if (attrName == null) {
            // no attribute for this property - nothing to do
            return null;
        }
        return getValues(attrs, attrName);
    }

    private AttrMap getAttrMap() {
        return m_attrMap;
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
        // some values like userPassword are returned as byte[], see XX-9328
        if (value instanceof byte[]) {
            return new String((byte[]) value);
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

    public String getPin(Attributes attrs, boolean newUser) throws NamingException {
        String pin = getValue(attrs, Index.PIN);
        if (pin == null && newUser) {
            // for new users consider default pin
            pin = getAttrMap().getDefaultPin();
        }
        return pin;
    }

    public String getVoicemailPin(Attributes attrs, boolean newUser) throws NamingException {
        String pin = getValue(attrs, Index.VOICEMAIL_PIN);
        if (pin == null && newUser) {
            // for new users consider default pin
            pin = getAttrMap().getDefaultPin();
        }
        return pin;
    }

    public String getAliases(Attributes attrs) throws NamingException {
        return getValue(attrs, Index.ALIAS);
    }

    public String getImId(Attributes attrs) throws NamingException {
        return getValue(attrs, Index.IM_ID);
    }

    public String getUserName(Attributes attrs) throws NamingException {
        String attrName = getAttrMap().getIdentityAttributeName();
        String userName = getValue(attrs, attrName);
        return userName;
    }

    public String getSipPassword(Attributes attrs) throws NamingException {
        return getValue(attrs, Index.SIP_PASSWORD);
    }

    /**
     * Not meant to be set from spring, but rather pulled from LdapManager after spring instantiated.
     */
    public void setAttrMap(AttrMap attrMap) {
        m_attrMap = attrMap;
    }

    public void setLdapManager(LdapManager ldapManager) {
        m_ldapManager = ldapManager;
    }
}
