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
import static org.sipfoundry.commons.mongo.MongoConstants.TIMESTAMP;
import static org.sipfoundry.commons.mongo.MongoConstants.TIMEZONE;
import static org.sipfoundry.commons.mongo.MongoConstants.UID;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TimeZone;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.address.Address;
import org.sipfoundry.sipxconfig.branch.Branch;
import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.forwarding.CallSequence;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.ivr.Ivr;
import org.sipfoundry.sipxconfig.permission.PermissionName;

//import static org.sipfoundry.commons.mongo.MongoConstants.TIMEZONE;

/**
 * Can be user that logs in, can be superadmin, can be user for phone line
 */
public class User extends AbstractUser implements Replicable {
    private static final String ALIAS_RELATION = "alias";
    private static final String ALIAS_RELATION_FAX = "fax";
    private static final String TZ = "timezone/timezone";
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
        return dataSets;
    }

    public Collection<String> getPermissions() {
        return getUserPermissionNames();
    }

    public String getContactUri(String domain) {
        return SipUri.format(getDisplayName(), getUserName(), domain);
    }

    public String getIdentity(String domain) {
        if (m_identity == null) {
            m_identity = SipUri.stripSipPrefix(SipUri.format(null, getUserName(), domain));
        }
        return m_identity;
    }

    public void setIdentity(String identity) {
        m_identity = identity;
    }

    public Collection<AliasMapping> getAliasMappings(String domainName) {
        List<AliasMapping> mappings = new ArrayList<AliasMapping>();
        String contact = getUri(domainName);
        for (String alias : getAliases()) {
            AliasMapping mapping = new AliasMapping(alias, contact, ALIAS_RELATION);
            mappings.add(mapping);
        }

        ImAccount imAccount = new ImAccount(this);
        if (imAccount.isEnabled() && !StringUtils.isEmpty(imAccount.getImId())
                && !imAccount.getImId().equals(getUserName())) {
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
        props.put(TIMESTAMP, System.currentTimeMillis());
        return props;
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
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
}
