/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.ObjectUtils;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;

/**
 * Can be user that logs in, can be superadmin, can be user for phone line
 */
public class User extends BeanWithGroups implements NamedObject {
    public static final String GROUP_RESOURCE_ID = "user";

    // security roles
    public static final String ROLE_USER = "ROLE_USER";
    public static final String ROLE_ADMIN = "ROLE_ADMIN";

    // property names
    public static final String ALIASES_PROP = "aliases";
    public static final String FIRST_NAME_PROP = "firstName";
    public static final String LAST_NAME_PROP = "lastName";
    public static final String USER_NAME_PROP = "userName";

    // Reserved name for the special superadmin user. In principle, this name could now be
    // anything, it's just "superadmin" by current convention.
    public static final String SUPERADMIN = "superadmin";

    // "0" cannot be used as valid extension - it's hardcoded in login.vxml
    private static final Pattern PATTERN_NUMERIC = Pattern.compile("([1-9]\\d*)|(0\\d+)");

    private PermissionManager m_permissionManager;

    private String m_firstName;

    private String m_sipPassword;

    private String m_pintoken;

    private String m_lastName;

    private String m_userName;

    private Set<String> m_aliases = new LinkedHashSet<String>(0);

    private Set m_supervisorForGroups;

    /**
     * Return the pintoken, which is the hash of the user's PIN. The PIN itself is private to the
     * user. To keep the PIN secure, we don't store it.
     */
    public String getPintoken() {
        return (String) ObjectUtils.defaultIfNull(m_pintoken, StringUtils.EMPTY);
    }

    /**
     * Set the pintoken, which is the hash of the user's PIN. This method is only for prevent the
     * pintoken from being nulluse by Hibernate. Call setPin to change the PIN.
     */
    public void setPintoken(String pintoken) {
        m_pintoken = pintoken;
    }

    /**
     * Set the PIN, protecting it under a security realm. The PIN is private to the user. To keep
     * the PIN secure, we don't store it. Instead we store the "pintoken", which is a hash of the
     * PIN.
     *
     * @param pin PIN
     * @param realm security realm
     */
    public void setPin(String pin, String realm) {
        String pin2 = (String) ObjectUtils.defaultIfNull(pin, StringUtils.EMPTY); // handle null
        // pin
        setPintoken(Md5Encoder.digestPassword(m_userName, realm, pin2));
    }

    public String getFirstName() {
        return m_firstName;
    }

    public void setFirstName(String firstName) {
        m_firstName = firstName;
    }

    public String getSipPassword() {
        return m_sipPassword;
    }

    public String getSipPasswordHash(String realm) {
        String password = (String) ObjectUtils.defaultIfNull(m_sipPassword, StringUtils.EMPTY);
        return Md5Encoder.digestPassword(m_userName, realm, password);
    }

    public void setSipPassword(String password) {
        m_sipPassword = password;
    }

    public String getLastName() {
        return m_lastName;
    }

    public void setLastName(String lastName) {
        m_lastName = lastName;
    }

    public String getUserName() {
        return (String) ObjectUtils.defaultIfNull(m_userName, StringUtils.EMPTY);
    }

    public void setUserName(String userName) {
        m_userName = userName;
    }

    public String getDisplayName() {
        Object[] names = {
            m_firstName, m_lastName
        };
        String s = StringUtils.join(names, ' ');
        return StringUtils.trimToNull(s);
    }

    public Set<String> getAliases() {
        return m_aliases;
    }

    public void setAliases(Set<String> aliases) {
        m_aliases = aliases;
    }

    private List<String> getNumericAliases() {
        Set<String> aliases = getAliases();
        List<String> numeric = new ArrayList<String>(aliases.size());
        for (String alias : aliases) {
            Matcher m = PATTERN_NUMERIC.matcher(alias);
            if (m.matches()) {
                numeric.add(alias);
            }
        }
        return numeric;
    }

    /**
     * Finds the shorted numeric alias for this user.
     *
     * @return null if no numeric aliases, shortest numeric alias (if there more than one that
     *         have equal lenght we can return any of them)
     */
    private String getShortestNumericAlias() {
        List<String> numericAliases = getNumericAliases();
        String shortestAlias = null;
        int len = 0;
        for (String alias : numericAliases) {
            if (shortestAlias == null || alias.length() < len) {
                shortestAlias = alias;
                len = alias.length();
            }
        }
        return shortestAlias;
    }

    public boolean hasNumericUsername() {
        Matcher m = PATTERN_NUMERIC.matcher(m_userName);
        return m.matches();
    }

    /**
     * Get numeric extension for this user. Since we are trying to support many possible options
     * we are going to try user name and then list of aliases. If user has more than a single
     * numeric alias it's not going to work reliably.
     *
     * Note: since "0" is hardcoded in login.vxml it cannot be used as user extension
     *
     * @return String representing numeric extension for this user
     */
    public String getExtension(boolean considerUserName) {
        if (considerUserName && hasNumericUsername()) {
            return m_userName;
        }
        return getShortestNumericAlias();
    }

    /**
     * Copy the input aliases to become the aliases of this user, without replacing the Set
     * object. For a user read from the DB, Hibernate creates the Set and we don't want to mess
     * with it. Also, by copying the aliases, subsequent changes to the input Set won't affect the
     * user's Set, since it is a separate object.
     */
    public void copyAliases(Collection<String> aliases) {
        getAliases().clear();
        getAliases().addAll(aliases);
    }

    /**
     * Add the alias to the set of aliases. Return true if the alias was added, false if the alias
     * was already in the set.
     */
    public boolean addAlias(String alias) {
        return getAliases().add(alias);
    }

    public void addAliases(String[] aliases) {
        getAliases().addAll(Arrays.asList(aliases));
    }

    /** Return the aliases as a space-delimited string */
    public String getAliasesString() {
        List<String> aliases = new ArrayList<String>(getAliases());
        Collections.sort(aliases);
        return StringUtils.join(aliases.iterator(), " ");
    }

    /** Set the aliases from a space-delimited string */
    public void setAliasesString(String aliasesString) {
        getAliases().clear();
        if (aliasesString != null) {
            String[] aliases = StringUtils.split(aliasesString);
            addAliases(aliases);
        }
    }

    /**
     * Creates long version of user SIP URI (with display name)
     */
    public String getUri(String domainName) {
        return SipUri.format(this, domainName);
    }

    /**
     * Creates short version of user SIP URI (without display name)
     *
     * sip:user@example.com
     */
    public String getAddrSpec(String domainName) {
        return SipUri.format(m_userName, domainName, false);
    }

    @Override
    protected Setting loadSettings() {
        if (m_permissionManager != null) {
            return m_permissionManager.getPermissionModel();
        }
        return null;
    }

    public List getAliasMappings(String domainName) {
        final String contact = getUri(domainName);
        List mappings = new ArrayList(getAliases().size());
        for (Iterator iter = getAliases().iterator(); iter.hasNext();) {
            String alias = (String) iter.next();
            if (StringUtils.isBlank(alias)) {
                throw new RuntimeException("Found an empty alias for user " + m_userName);
            }
            final String identity = AliasMapping.createUri(alias, domainName);
            AliasMapping mapping = new AliasMapping(identity, contact);
            mappings.add(mapping);
        }

        return mappings;
    }

    /**
     * Returns the names of all permissions assigned to the user
     */
    public Collection<String> getUserPermissionNames() {
        Collection<Permission> perms = m_permissionManager.getPermissions();
        Collection<String> names = new ArrayList<String>(perms.size());
        for (Permission perm : perms) {
            if (hasPermission(perm)) {
                names.add(perm.getName());
            }
        }
        return names;
    }

    /**
     * Check if a user has a specific permission
     */
    public boolean hasPermission(PermissionName permissionName) {
        Setting setting = retrieveSettingForPermission(permissionName);
        return Permission.isEnabled(setting.getValue());
    }

    /**
     * Check if a user has a specific permission
     */
    public boolean hasPermission(Permission permission) {
        Setting setting = retrieveSettingForPermission(permission);
        return Permission.isEnabled(setting.getValue());
    }

    /**
     * Set specific permission for the user
     *
     * @param permissionName - permission to set
     * @param enabled - true for enabled, false for disabled
     */
    public void setPermission(PermissionName permissionName, boolean enabled) {
        Setting setting = retrieveSettingForPermission(permissionName);
        setting.setTypedValue(enabled);
    }

    /**
     * Set specific permission for the user
     *
     * @param permission - permission to set
     * @param enabled - true for enabled, false for disabled
     */
    public void setPermission(Permission permission, boolean enabled) {
        Setting setting = retrieveSettingForPermission(permission);
        setting.setTypedValue(enabled);
    }

    public boolean isAdmin() {
        return hasPermission(PermissionName.SUPERADMIN);
    }

    private Setting retrieveSettingForSettingPath(String path, String name) {
        Setting setting = getSettings().getSetting(path);
        if (setting == null) {
            throw new IllegalArgumentException("Setting " + name + " does not exist in user setting model");
        }
        return setting;
    }

    private Setting retrieveSettingForPermission(PermissionName permissionName) {
        return retrieveSettingForSettingPath(permissionName.getPath(), permissionName.getName());
    }

    private Setting retrieveSettingForPermission(Permission permission) {
        return retrieveSettingForSettingPath(permission.getSettingPath(), permission.getName());
    }

    public boolean isSupervisor() {
        return m_supervisorForGroups != null && m_supervisorForGroups.size() > 0;
    }

    public Set getSupervisorForGroups() {
        return m_supervisorForGroups;
    }

    public void setSupervisorForGroups(Set<Group> supervisorForGroups) {
        m_supervisorForGroups = supervisorForGroups;
    }

    public void clearSupervisorForGroups() {
        if (m_supervisorForGroups != null) {
            m_supervisorForGroups.clear();
        }
    }

    public Group getSite() {
        Set<Group> groups = getGroups();
        if (groups.isEmpty()) {
            return null;
        }
        // first group represents user location
        return groups.iterator().next();
    }

    public void addSupervisorForGroup(Group group) {
        if (group.isNew()) {
            throw new RuntimeException("Group needs to be saved before it can be added to the set.");
        }
        if (m_supervisorForGroups == null) {
            m_supervisorForGroups = new HashSet();
        }
        m_supervisorForGroups.add(group);
    }

    public String getName() {
        return getUserName();
    }

    public void setName(String name) {
        setUserName(name);
    }

    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }
}
