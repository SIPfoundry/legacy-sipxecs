/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common;

import static org.sipfoundry.commons.mongo.MongoConstants.CONTACT;
import static org.sipfoundry.commons.mongo.MongoConstants.GROUPS;
import static org.sipfoundry.commons.mongo.MongoConstants.HOTELING;
import static org.sipfoundry.commons.mongo.MongoConstants.TIMESTAMP;
import static org.sipfoundry.commons.mongo.MongoConstants.TIMEZONE;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;
import static org.sipfoundry.commons.mongo.MongoConstants.VOICEMAIL_ENABLED;
import static org.sipfoundry.sipxconfig.common.AbstractUser.IM_ACCOUNT;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TimeZone;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.callgroup.AbstractRing;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.permission.PermissionName;

/**
 * Can be user that logs in, can be superadmin, can be user for phone line
 */
public class User extends AbstractUser implements Replicable {
    private static final Log LOG = LogFactory.getLog(User.class);
    private static final String VM_ENABLED_SETTING_PATH = "voicemail/vacation/vmEnabled";
    private static final String ALIAS_RELATION = "alias";
    private static final String ALIAS_RELATION_FAX = "fax";
    private static final String TZ = "timezone/timezone";
    private static final String E911_SETTING_PATH = "e911/location";
    private static final String PHANTOM_USER = "phantom/enabled";
    private static final String FORCE_PIN_CHANGE = "voicemail/security/force-pin-change";
    private String m_identity;
    private boolean m_validUser = true;

    /**
     * get all the data sets that are replicable for this entity
     *
     * @return
     */
    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> dataSets = new HashSet<DataSet>();
        dataSets.add(DataSet.PERMISSION);
        dataSets.add(DataSet.ALIAS);
        dataSets.add(DataSet.USER_STATIC);
        dataSets.add(DataSet.CALLER_ALIAS);
        dataSets.add(DataSet.CREDENTIAL);
        dataSets.add(DataSet.USER_FORWARD);
        dataSets.add(DataSet.USER_LOCATION);
        dataSets.add(DataSet.SPEED_DIAL);
        dataSets.add(DataSet.MAILSTORE);
        dataSets.add(DataSet.E911);
        return dataSets;
    }

    @SuppressWarnings("unchecked")
    public Collection<String> getPermissions() {
        return isEnabled() ? getUserPermissionNames() : CollectionUtils.EMPTY_COLLECTION;
    }

    public String getContactUri(String domain) {
        return SipUri.format(getDisplayName(), getUserName(), domain);
    }

    @Override
    public String getIdentity(String domain) {
        if (m_identity == null) {
            m_identity = SipUri.stripSipPrefix(SipUri.format(null, getUserName(), domain));
        }
        return m_identity;
    }

    public void setIdentity(String identity) {
        m_identity = identity;
    }

    public boolean isEnabled() {
        return getUserProfile().isEnabled();
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        List<AliasMapping> mappings = new ArrayList<AliasMapping>();
        String contact = getUri(domainName);
        for (String alias : getAliases()) {
            AliasMapping mapping = new AliasMapping(alias, contact, ALIAS_RELATION);
            mappings.add(mapping);
        }

        ImAccount imAccount = new ImAccount(this);
        if (imAccount.isEnabled() && !StringUtils.isEmpty(imAccount.getImId())
                && !imAccount.getImId().equals(getUserName()) && !getAliases().contains(imAccount.getImId())) {
            AliasMapping mapping = new AliasMapping(imAccount.getImId(), contact, ALIAS_RELATION);
            mappings.add(mapping);
        }

        // add fax extension aliases
        String faxExtension = getFaxExtension();
        String faxDid = getFaxDid();
        if (!faxExtension.isEmpty()) {
            String faxContactUri = SipUri.format(getDisplayName(), FAX_EXTENSION_PREFIX + getUserName(), domainName);
            AliasMapping mapping = new AliasMapping(faxExtension, faxContactUri, ALIAS_RELATION_FAX);
            mappings.add(mapping);
            if (!faxDid.isEmpty()) {
                mapping = new AliasMapping(faxDid, faxContactUri, ALIAS_RELATION_FAX);
                mappings.add(mapping);
            }
        }

        // add call sequence
        CallSequence sequence = getForwardingContext().getCallSequenceForUserId(getId());
        if (sequence != null) {
            mappings.addAll(sequence.getAliasMappings(domainName));
        }

        if (this.hasPermission(PermissionName.FREESWITH_VOICEMAIL)) {
            // NOTE: Missing explaination why exchange needs direction connection to FS
            Address address = getAddressManager().getSingleAddress(Ivr.SIP_ADDRESS);
            if (address != null) {
                String sipUri = SipUri.formatDepositVm(getUserName(), address.getAddress());
                AliasMapping mapping = new AliasMapping("~~vm~" + getUserName(), sipUri, "vmprm");
                mappings.add(mapping);
            }
        }

        return mappings;
    }

    protected CallSequence getCallSequence() {
        return getForwardingContext().getCallSequenceForUserId(getId());
    }

    public int getEffectiveExpire() {
        int expire = 0;
        CallSequence sequence = getCallSequence();
        if (sequence != null) {
            int timeToSum = sequence.getCfwdTime();
            List<AbstractRing> rings = sequence.getRings();

            // if no ring defined then return only initial expire
            if (rings.isEmpty()) {
                return timeToSum;
            }

            for (AbstractRing ring : rings) {
                if (AbstractRing.Type.IMMEDIATE == ring.getType()) {
                    if (ring.getExpiration() > timeToSum) {
                        timeToSum = ring.getExpiration();
                    }
                } else {
                    // end "at the same time" cycle
                    expire = expire + timeToSum;
                    timeToSum = ring.getExpiration();
                }
                // if last ring then sum it up
                if (rings.lastIndexOf(ring) == rings.size() - 1) {
                    expire = expire + timeToSum;
                }
            }
        }
        return expire;
    }

    public void setValidUser(boolean vld) {
        m_validUser = vld;
    }

    @Override
    public boolean isValidUser() {
        return m_validUser;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        Map<String, Object> props = new HashMap<String, Object>();
        props.put(UID, getUserName());
        props.put(CONTACT, getContactUri(domain));
        props.put(GROUPS, getGroupsNames().split(" "));
        props.put(TIMEZONE, getTimezone().getID());
        props.put(VOICEMAIL_ENABLED, isDepositVoicemail());
        props.put(TIMESTAMP, System.currentTimeMillis());
        props.put(HOTELING, getSettingValue("hotelling/enable"));
        return props;
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }

    /**
     * User entity must be replicated only when UserProfile.m_enabled is set to true
     */
    @Override
    public boolean isReplicationEnabled() {
        return isEnabled();
    }

    /**
     * Return the effective branch a user is in.
     *
     * @return
     */
    public Branch getUserBranch() {
        return (getInheritedBranch() != null) ? (getInheritedBranch()) : (getBranch());
    }

    public TimeZone getTimezone() {
        if (getUserBranch() != null && (Boolean) getSettingTypedValue("timezone/useBranchTimezone")
                && getUserBranch().getTimeZone() != null) {
            return TimeZone.getTimeZone((getUserBranch().getTimeZone()));
        }

        if (getSettingValue(TZ) != null) {
            return TimeZone.getTimeZone(getSettingValue(TZ));
        }

        return TimeZone.getDefault();
    }

    public String getDid() {
        return getUserProfile().getDidNumber();
    }

    public boolean isPhantom() {
        return (Boolean) getSettingTypedValue(PHANTOM_USER);
    }

    public void setPhantom(boolean phantom) {
        getSettings().getSetting(PHANTOM_USER).setTypedValue(phantom);
    }

    public boolean isForcePinChange() {
        return (Boolean) getSettingTypedValue(FORCE_PIN_CHANGE);
    }

    public void setForcePinChange(boolean force) {
        getSettings().getSetting(FORCE_PIN_CHANGE).setTypedValue(force);
    }

    public Integer getE911LocationId() {
        if (getSettingTypedValue(E911_SETTING_PATH) == null) {
            return null;
        }
        Integer id = (Integer) getSettingTypedValue(E911_SETTING_PATH);
        if (id < 0) {
            LOG.error("Database is in bad state, E911 location defined is wrong! Please review!");
        }
        return id;
    }

    public boolean isImEnabled() {
        return (Boolean) getSettingTypedValue(IM_ACCOUNT);
    }

    public void setE911LocationId(Integer id) {
        if (id != null && id < 0) {
            return;
        }
        setSettingTypedValue(E911_SETTING_PATH, id);
    }

    public boolean isDepositVoicemail() {
        return ((Boolean) getSettingTypedValue(VM_ENABLED_SETTING_PATH)).booleanValue();
    }

    public void setDepositVoicemail(boolean value) {
        setSettingTypedValue(VM_ENABLED_SETTING_PATH, value);
    }
}
