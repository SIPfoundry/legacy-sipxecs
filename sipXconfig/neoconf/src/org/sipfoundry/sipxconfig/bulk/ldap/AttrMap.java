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

import static org.apache.commons.lang.StringUtils.isEmpty;
import static org.apache.commons.lang.StringUtils.join;
import static org.apache.commons.lang.StringUtils.split;
import static org.apache.commons.lang.StringUtils.trimToNull;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.functors.NotNullPredicate;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.bulk.csv.Index;
import org.sipfoundry.sipxconfig.common.BeanWithId;

/**
 * Information related to mapping LDAP attributes to User properties
 */
public class AttrMap extends BeanWithId {
    private static final String SEPARATOR = "|";

    private Map<String, String> m_user2ldap = new TreeMap<String, String>();

    /**
     * name of the group that will keep all users imported from LDAP
     */
    private String m_defaultGroupName;

    /**
     * PIN to be used for Users that do not have PIN mapped
     */
    private String m_defaultPin;

    /**
     * Starting point of the search
     */
    private String m_searchBase;

    private String m_searchBaseDefault;

    /**
     * Additional filter expression
     */
    private String m_filter;

    /**
     * Object class containing user attributes.
     *
     * This is used in a filter that selects user related entries. It's perfectly OK to use
     * attributes from other object class in the mapping
     *
     * @see m_selectedObjectClasses
     *
     */
    private String m_objectClass;

    /**
     * Selected object classes - we will only consider attributes from this list for mapping.
     */
    private Collection<String> m_selectedObjectClasses;

    /**
     * Path to LDAP schema - usually set by call to verify
     */
    private String m_subschemaSubentry;

    /**
     * Returns non null LDAP attributes. Used to limit search results.
     */
    public Collection<String> getLdapAttributes() {
        Collection<String> attrs = new ArrayList<String>(m_user2ldap.values());
        CollectionUtils.filter(attrs, NotNullPredicate.INSTANCE);
        //we can have multiple attributes values assigned for a user field (alias for example)
        //In DB, table ldap_user_property_to_ldap_attr  these values are saved like this: audio|mobile|ipphone
        List<String> attrsList = new ArrayList<String>();
        for (String attr : attrs) {
            if (!isEmpty(attr)) {
                attrsList.addAll(Arrays.asList(split(attr, SEPARATOR)));
            }
        }
        return attrsList;
    }

    public String[] getLdapAttributesArray() {
        Collection<String> attrs = getLdapAttributes();
        return attrs.toArray(new String[attrs.size()]);
    }

    public String getIdentityAttributeName() {
        return m_user2ldap.get(Index.USERNAME.getName());
    }

    public String getImAttributeName() {
        return m_user2ldap.get(Index.IM_ID.getName());
    }

    public void setDefaultPin(String defaultPin) {
        m_defaultPin = defaultPin;
    }

    public String getDefaultPin() {
        return m_defaultPin;
    }

    public void setDefaultGroupName(String defaultGroupName) {
        m_defaultGroupName = defaultGroupName;
    }

    public String getDefaultGroupName() {
        return m_defaultGroupName;
    }

    public void setSearchBase(String searchBase) {
        m_searchBase = searchBase;
    }

    public String getSearchBase() {
        return m_searchBase;
    }

    public void setObjectClass(String objectClass) {
        m_objectClass = objectClass;
    }

    public String getObjectClass() {
        return m_objectClass;
    }

    public void setSelectedObjectClasses(Collection<String> selectedObjectClasses) {
        m_selectedObjectClasses = selectedObjectClasses;
    }

    public Collection<String> getSelectedObjectClasses() {
        return m_selectedObjectClasses;
    }

    /**
     * @return filter string based on object class selected by the user
     */
    public String getSearchFilter() {
        String objectClass = StringUtils.defaultIfEmpty(m_objectClass, "*");
        // strip - it's added later
        if (m_filter == null) {
            return String.format("(objectclass=%s)", objectClass);
        }
        String filter = m_filter;
        if (m_filter.startsWith("(") && m_filter.endsWith(")")) {
            filter = m_filter.substring(1, m_filter.length() - 1);
        }
        return String.format("(&(objectclass=%s)(%s))", objectClass, filter);
    }

    public void setUserToLdap(Map<String, String> user2ldap) {
        m_user2ldap = user2ldap;
    }

    public Map<String, String> getUserToLdap() {
        return m_user2ldap;
    }

    public void setAttribute(String field, String attribute) {
        m_user2ldap.put(field, attribute);
    }

    public String getAttribute(String field) {
        return m_user2ldap.get(field);
    }
    /**
     * This method should be used for getting multiple attributes mapping (example: alias)
     * @param field
     * @return
     */
    public List<String> getAttributes(String field) {
        String joinedAttrs = m_user2ldap.get(field);
        return isEmpty(joinedAttrs) ? new ArrayList<String>()
               : new ArrayList<String>(Arrays.asList(split(joinedAttrs, SEPARATOR)));
    }

    /**
     * This method should be used for setting multiple attributes mapping (example: alias)
     * @param field
     * @return
     */
    public void setAttributes(String field, List<String> attributes) {
        String joinedAttrs = trimToNull(join(attributes, SEPARATOR));
        m_user2ldap.put(field, joinedAttrs);
    }

    public String getFilter() {
        return m_filter;
    }

    public void setFilter(String filter) {
        m_filter = filter;
    }

    public void setSubschemaSubentry(String subschemaSubentry) {
        m_subschemaSubentry = subschemaSubentry;
    }

    public String getSubschemaSubentry() {
        return m_subschemaSubentry;
    }

    public boolean verified() {
        return StringUtils.isNotEmpty(m_searchBase);
    }

    public String getSearchBaseDefault() {
        return m_searchBaseDefault;
    }

    public void setSearchBaseDefault(String searchBaseDefault) {
        m_searchBaseDefault = searchBaseDefault;
    }
}
