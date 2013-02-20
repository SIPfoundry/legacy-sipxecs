/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common;

import static org.apache.commons.lang.StringUtils.EMPTY;
import static org.apache.commons.lang.StringUtils.defaultString;
import static org.apache.commons.lang.StringUtils.isNotEmpty;
import static org.apache.commons.lang.StringUtils.join;
import static org.apache.commons.lang.StringUtils.lowerCase;
import static org.apache.commons.lang.StringUtils.split;
import static org.apache.commons.lang.StringUtils.trim;
import static org.apache.commons.lang.StringUtils.trimToNull;
import static org.sipfoundry.sipxconfig.common.UserCallerAliasInfo.ANONYMOUS_CALLER_ALIAS;
import static org.sipfoundry.sipxconfig.common.UserCallerAliasInfo.EXTERNAL_NUMBER;
import static org.sipfoundry.sipxconfig.permission.PermissionName.EXCHANGE_VOICEMAIL;
import static org.sipfoundry.sipxconfig.permission.PermissionName.FREESWITH_VOICEMAIL;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.ACTIVE_GREETING;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.ALT_EMAIL_ATTACH_AUDIO;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.ALT_EMAIL_FORMAT;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.ALT_EMAIL_NOTIFICATION;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.PRIMARY_EMAIL_ATTACH_AUDIO;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.PRIMARY_EMAIL_FORMAT;
import static org.sipfoundry.sipxconfig.vm.MailboxPreferences.PRIMARY_EMAIL_NOTIFICATION;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.LinkedHashSet;
import java.util.List;
import java.util.Set;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.commons.security.Md5Encoder;
import org.sipfoundry.commons.userdb.profile.UserProfile;
import org.sipfoundry.sipxconfig.address.AddressManager;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.forwarding.ForwardingContext;
import org.sipfoundry.sipxconfig.moh.MohAddressFactory;
import org.sipfoundry.sipxconfig.moh.MusicOnHoldManager;
import org.sipfoundry.sipxconfig.permission.Permission;
import org.sipfoundry.sipxconfig.permission.PermissionManager;
import org.sipfoundry.sipxconfig.permission.PermissionName;
import org.sipfoundry.sipxconfig.proxy.ProxyManager;
import org.sipfoundry.sipxconfig.setting.BeanWithGroups;
import org.sipfoundry.sipxconfig.setting.Group;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;
import org.sipfoundry.sipxconfig.time.NtpManager;

/**
 * Can be user that logs in, can be superadmin, can be user for phone line
 */
public abstract class AbstractUser extends BeanWithGroups {
    public static final int VOICEMAIL_PIN_LEN = 4;
    public static final int PASSWORD_LEN = 8;
    public static final String GROUP_RESOURCE_ID = "user";

    // property names
    public static final String ALIASES_PROP = "aliases";
    public static final String FIRST_NAME_PROP = "firstName";
    public static final String LAST_NAME_PROP = "lastName";
    public static final String USER_NAME_PROP = "userName";

    public static final String MOH_SETTING = "moh";
    public static final String MOH_AUDIO_SOURCE_SETTING = "moh/audio-source";
    public static final String FAX_EXTENSION_PREFIX = "~~ff~";
    public static final String EMPTY_STRING = "";
    public static final String FAX_EXTENSION_SETTING = "voicemail/fax/extension";
    public static final String DID_SETTING = "voicemail/fax/did";
    public static final String DOMAIN_SETTING = "user-domain/domain";
    public static final String CALLFWD_TIMER = "callfwd/timer";
    public static final String OPERATOR_SETTING = "personal-attendant/operator";
    public static final String DEFAULT_VM_OPTION = "personal-attendant/default-vm-option";

    public static enum MohAudioSource {
        FILES_SRC, PERSONAL_FILES_SRC, SOUNDCARD_SRC, SYSTEM_DEFAULT, NONE;

        public static MohAudioSource parseSetting(String mohSetting) {
            try {
                return valueOf(mohSetting);
            } catch (IllegalArgumentException e) {
                return SYSTEM_DEFAULT;
            }
        }
    }

    // Reserved name for the special superadmin user. In principle, this name could now be
    // anything, it's just "superadmin" by current convention.
    public static final String SUPERADMIN = "superadmin";

    // "0" cannot be used as valid extension - it's hardcoded in login.vxml
    private static final Pattern PATTERN_NUMERIC = Pattern.compile("([1-9]\\d*)|(0\\d+)");

    private PermissionManager m_permissionManager;
    private DomainManager m_domainManager;
    private ProxyManager m_proxyManager;
    private ForwardingContext m_fwdContext;
    private AddressManager m_addressManager;
    private MohAddressFactory m_mohAddresses;
    private NtpManager m_timeManager;

    private String m_firstName;

    private String m_sipPassword;

    private String m_pintoken;

    private String m_voicemailPintoken;

    private String m_clearVoicemailPin;

    private String m_lastName;

    private String m_userName;

    private Set<String> m_aliases = new LinkedHashSet<String>(0);

    private Set m_supervisorForGroups;

    private UserProfile m_userProfile = new UserProfile();

    private boolean m_isShared;

    private Branch m_branch;

    private boolean m_notified;

    /**
     * Return the pintoken, which is the hash of the user's PIN. The PIN itself is private to the
     * user. To keep the PIN secure, we don't store it.
     */
    public String getPintoken() {
        return defaultString(m_pintoken, EMPTY);
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
    public void setPin(String pin) {
        String pin2 = defaultString(pin, EMPTY); // handle null
        setPintoken(Md5Encoder.getEncodedPassword(pin2));
    }

    public String getVoicemailPintoken() {
        return defaultString(m_voicemailPintoken, EMPTY);
    }

    public void setVoicemailPintoken(String voicemailPintoken) {
        m_voicemailPintoken = voicemailPintoken;
    }

    public void setVoicemailPin(String voicemailPin) {
        String pin2 = defaultString(voicemailPin, EMPTY); // handle null
        // pin
        m_clearVoicemailPin = voicemailPin;
        setVoicemailPintoken(Md5Encoder.digestEncryptPassword(m_userName, pin2));
    }

    public String getFirstName() {
        return m_firstName;
    }

    public void setFirstName(String firstName) {
        m_firstName = trim(firstName);
    }

    public String getSipPassword() {
        return m_sipPassword;
    }

    public String getSipPasswordHash(String realm) {
        String password = defaultString(m_sipPassword, EMPTY);
        return Md5Encoder.digestEncryptPassword(m_userName, realm, password);
    }

    public void setSipPassword(String password) {
        m_sipPassword = password;
    }

    public String getLastName() {
        return m_lastName;
    }

    public void setLastName(String lastName) {
        m_lastName = trim(lastName);
    }

    public String getUserName() {
        return defaultString(m_userName, EMPTY);
    }

    public void setUserName(String userName) {
        m_userName = trim(userName);
    }

    /**
     * Builds displayName based on the first and last names.
     *
     * Should be only used to retrieve displayName part of SIP URI: it will return null if both
     * last names and first name are empty. Use getLabel as safer alternative.
     */
    public String getDisplayName() {
        Object[] names = {
            m_firstName, m_lastName
        };
        String s = join(names, ' ');
        return trimToNull(s);
    }

    public String getLabel() {
        return defaultString(getDisplayName(), getUserName());
    }

    public Set<String> getAliases() {
        return m_aliases;
    }

    public void setAliases(Set<String> aliases) {
        m_aliases = aliases;
    }

    public String getDomain() {
        return m_domainManager.getDomainName();
    }

    public Branch getBranch() {
        return m_branch;
    }

    public void setBranch(Branch branch) {
        m_branch = branch;
    }

    public void setMusicOnHoldManager(MusicOnHoldManager musicOnHoldManager) {
        m_mohAddresses = musicOnHoldManager.getAddressFactory();
    }

    public void setMohAddresses(MohAddressFactory mohAddresses) {
        m_mohAddresses = mohAddresses;
    }

    private List<String> getNumericAliases() {
        Set<String> aliases = getAliases();
        List<String> numeric = new ArrayList<String>(aliases.size());
        for (String alias : aliases) {
            if (isNumeric(alias)) {
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
        return isNumeric(m_userName);
    }

    public static boolean isNumeric(String s) {
        Matcher m = PATTERN_NUMERIC.matcher(s);
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
        return getAliases().add(trim(alias));
    }

    public void addAliases(String[] aliases) {
        for (String alias : aliases) {
            addAlias(alias);
        }
    }

    /** Return the aliases as a space-delimited string */
    public String getAliasesString() {
        List<String> aliases = new ArrayList<String>(getAliases());
        Collections.sort(aliases);
        return join(aliases.iterator(), " ");
    }

    /** Set the aliases from a space-delimited string */
    public void setAliasesString(String aliasesString) {
        getAliases().clear();
        if (aliasesString != null) {
            String[] aliases = split(aliasesString);
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
    public void initialize() {
        addDefaultBeanSettingHandler(this);
    }

    @Override
    protected Setting loadSettings() {
        if (m_permissionManager != null) {
            return m_permissionManager.getPermissionModel();
        }
        return null;
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

    public boolean hasVoicemailPermission() {
        return hasPermission(PermissionName.VOICEMAIL);
    }

    public void setVoicemailPermission(boolean enabled) {
        setPermission(PermissionName.VOICEMAIL, enabled);
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

    public String getActiveGreeting() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(ACTIVE_GREETING);
        return null == setting ? EMPTY_STRING : (setting.getTypedValue() == null ? EMPTY_STRING : setting
                .getTypedValue().toString());
    }

    public void setActiveGreeting(String activeGreeting) {
        setSettingValue(ACTIVE_GREETING, activeGreeting);
    }

    public String getPrimaryEmailNotification() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(PRIMARY_EMAIL_NOTIFICATION);
        return null == setting ? EMPTY_STRING : (setting.getTypedValue() == null ? EMPTY_STRING : setting
                .getTypedValue().toString());
    }

    public void setPrimaryEmailNotification(String emailNotification) {
        setSettingValue(PRIMARY_EMAIL_NOTIFICATION, emailNotification);
    }

    public String getPrimaryEmailFormat() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(PRIMARY_EMAIL_FORMAT);
        return null == setting ? EMPTY_STRING : (setting.getTypedValue() == null ? EMPTY_STRING : (String) setting
                .getTypedValue());
    }

    public void setPrimaryEmailFormat(String emailFormat) {
        setSettingValue(PRIMARY_EMAIL_FORMAT, emailFormat);
    }

    public Boolean isPrimaryEmailAttachAudio() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(PRIMARY_EMAIL_ATTACH_AUDIO);
        return null == setting ? Boolean.valueOf(EMPTY_STRING) : (setting.getTypedValue() == null ? Boolean
                .valueOf(EMPTY_STRING) : (Boolean) setting.getTypedValue());
    }

    public void setPrimaryEmailAttachAudio(Boolean emailAttachAudio) {
        setSettingTypedValue(PRIMARY_EMAIL_ATTACH_AUDIO, emailAttachAudio);
    }

    public String getAlternateEmailNotification() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(ALT_EMAIL_NOTIFICATION);
        return null == setting ? EMPTY_STRING : (setting.getTypedValue() == null ? EMPTY_STRING : setting
                .getTypedValue().toString());
    }

    public void setAlternateEmailNotification(String alternateEmailNotification) {
        setSettingValue(ALT_EMAIL_NOTIFICATION, alternateEmailNotification);
    }

    public String getAlternateEmailFormat() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(ALT_EMAIL_FORMAT);
        return null == setting ? EMPTY_STRING : (setting.getTypedValue() == null ? EMPTY_STRING : (String) setting
                .getTypedValue());
    }

    public void setAlternateEmailFormat(String alternateEmailFormat) {
        setSettingValue(ALT_EMAIL_FORMAT, alternateEmailFormat);
    }

    public Boolean isAlternateEmailAttachAudio() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(ALT_EMAIL_ATTACH_AUDIO);
        return null == setting ? Boolean.valueOf(EMPTY_STRING) : (setting.getTypedValue() == null ? Boolean
                .valueOf(EMPTY_STRING) : (Boolean) setting.getTypedValue());
    }

    public void setAlternateEmailAttachAudio(Boolean alternateEmailAttachAudio) {
        setSettingTypedValue(ALT_EMAIL_ATTACH_AUDIO, alternateEmailAttachAudio);
    }

    public Boolean isVoicemailServer() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(FREESWITH_VOICEMAIL.getPath());
        return null == setting ? Boolean.valueOf(EMPTY_STRING) : (setting.getTypedValue() == null ? Boolean
                .valueOf(EMPTY_STRING) : (Boolean) setting.getTypedValue());
    }

    public void setVoicemailServer(Boolean voicemailServer) {
        setSettingTypedValue(FREESWITH_VOICEMAIL.getPath(), voicemailServer);
        setSettingTypedValue(EXCHANGE_VOICEMAIL.getPath(), !voicemailServer);
    }

    public String getExternalNumber() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(EXTERNAL_NUMBER);
        return null == setting ? EMPTY_STRING : (setting.getTypedValue() == null ? EMPTY_STRING : (String) setting
                .getTypedValue());
    }

    public void setExternalNumber(String externalNumber) {
        setSettingValue(EXTERNAL_NUMBER, externalNumber);
    }

    public Boolean isAnonymousCallerAlias() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(ANONYMOUS_CALLER_ALIAS);
        return null == setting ? Boolean.valueOf(EMPTY_STRING) : (setting.getTypedValue() == null ? Boolean
                .valueOf(EMPTY_STRING) : (Boolean) setting.getTypedValue());
    }

    public void setAnonymousCallerAlias(Boolean anonymous) {
        setSettingTypedValue(ANONYMOUS_CALLER_ALIAS, anonymous);
    }

    public String getFaxExtension() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(FAX_EXTENSION_SETTING);
        return null == setting ? EMPTY_STRING : (setting.getTypedValue() == null ? EMPTY_STRING : (String) setting
                .getTypedValue());
    }

    public void setFaxExtension(String faxExtension) {
        setSettingValue(FAX_EXTENSION_SETTING, faxExtension);
    }

    public String getFaxDid() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(DID_SETTING);
        return null == setting ? EMPTY_STRING : (setting.getTypedValue() == null ? EMPTY_STRING : (String) setting
                .getTypedValue());
    }

    public String getUserDomain() {
        Setting setting = null == getSettings() ? null : getSettings().getSetting(DOMAIN_SETTING);
        return null == setting ? EMPTY_STRING : (setting.getTypedValue() == null ? EMPTY_STRING : (String) setting
                .getTypedValue());
    }

    public void setFaxDid(String did) {
        setSettingValue(DID_SETTING, did);
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

    public Branch getSite() {
        Branch branch = getBranch();
        if (branch != null) {
            return branch;
        }
        return getInheritedBranch();
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

    /**
     * Check if the branch in question is referenced by this user
     *
     * @param Branch
     * @return true if any references have been found false otherwise
     */
    public boolean checkBranch(Branch branch) {
        return m_branch.equals(branch);
    }

    public String getName() {
        return getUserName();
    }

    public void setName(String name) {
        setUserName(name);
    }

    public String getImId() {
        return m_userProfile.getImId();
    }

    public void setImId(String id) {
        m_userProfile.setImId(lowerCase(trim(id)));
    }

    public String getImDisplayName() {
        return m_userProfile.getImDisplayName();
    }

    public void setImDisplayName(String imDisplayName) {
        m_userProfile.setImDisplayName(trim(imDisplayName));
    }

    public String getOperator() {
        return getSettingValue(OPERATOR_SETTING);
    }

    public boolean getPlayVmDefaultOptions() {
        return (Boolean) getSettingTypedValue(DEFAULT_VM_OPTION);
    }

    public void setOperator(String oper) {
        setSettingValue(OPERATOR_SETTING, oper);
    }

    public void setPermissionManager(PermissionManager permissionManager) {
        m_permissionManager = permissionManager;
    }

    public boolean getIsShared() {
        return m_isShared;
    }

    public void setIsShared(boolean isShared) {
        m_isShared = isShared;
    }

    public boolean hasImAccount() {
        return isNotEmpty(getImId());
    }

    public String getMusicOnHoldUri() {
        return m_mohAddresses.getPersonalMohFilesUri(getName());
    }

    public void setEmailAddress(String emailAddress) {
        m_userProfile.setEmailAddress(emailAddress);
    }

    public String getEmailAddress() {
        return m_userProfile.getEmailAddress();
    }

    public void setAlternateEmailAddress(String emailAddress) {
        m_userProfile.setAlternateEmailAddress(emailAddress);
    }

    public String getAlternateEmailAddress() {
        return m_userProfile.getAlternateEmailAddress();
    }

    public UserProfile getUserProfile() {
        m_userProfile.setUserId(getId().toString());
        m_userProfile.setUserName(getUserName());
        m_userProfile.setFirstName(getFirstName());
        m_userProfile.setLastName(getLastName());
        if (getBranch() != null) {
            m_userProfile.setBranchName(getBranch().getName());
        }
        return m_userProfile;
    }

    public void setUserProfile(UserProfile profile) {
        m_userProfile = profile;
    }

    /**
     * Determines if the passed group is available
     */
    @Override
    public boolean isGroupAvailable(Group group) {
        if (getBranch() != null && group.getBranch() != null
                && !StringUtils.equals(getBranch().getName(), group.getBranch().getName())) {
            return false;
        } else {
            return super.isGroupAvailable(group);
        }
    }

    @SettingEntry(path = CALLFWD_TIMER)
    public int getDefaultInitDelay() {
        return m_proxyManager.getSettings().getDefaultInitDelay();
    }

    @SettingEntry(path = "timezone/timezone")
    public String getDefaultTimeZone() {
        return m_timeManager.getSystemTimezone();
    }

    public void setDomainManager(DomainManager domainManager) {
        m_domainManager = domainManager;
    }

    public void setProxyManager(ProxyManager proxyManager) {
        m_proxyManager = proxyManager;
    }

    public void setForwardingContext(ForwardingContext fwdContext) {
        m_fwdContext = fwdContext;
    }

    protected ForwardingContext getForwardingContext() {
        return m_fwdContext;
    }

    public void setAddressManager(AddressManager addressManager) {
        m_addressManager = addressManager;
    }

    protected AddressManager getAddressManager() {
        return m_addressManager;
    }

    public String getClearVoicemailPin() {
        return m_clearVoicemailPin;
    }

    public void clearPasswords() {
        m_clearVoicemailPin = null;
    }

    public boolean isNotified() {
        return m_notified;
    }

    public void setNotified(boolean notified) {
        m_notified = notified;
    }

    public void setTimeManager(NtpManager timeManager) {
        m_timeManager = timeManager;
    }
}
