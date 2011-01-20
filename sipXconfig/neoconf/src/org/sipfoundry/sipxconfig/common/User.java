/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.im.ImAccount;
import org.sipfoundry.sipxconfig.permission.PermissionName;

/**
 * Can be user that logs in, can be superadmin, can be user for phone line
 */
public class User extends AbstractUser {
    private String m_identity;

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
            m_identity = SipUri.format(null, getUserName(), domain);
        }
        return m_identity;
    }

    public void setIdentity(String identity) {
        m_identity = identity;
    }

    public Map<Replicable, Collection<AliasMapping>> getAliasMappings(String domainName) {
        List<AliasMapping> mappings = new ArrayList<AliasMapping>();
        for (String alias : getAliases()) {
            AliasMapping mapping = new AliasMapping(alias);
            mappings.add(mapping);
        }

        ImAccount imAccount = new ImAccount(this);
        if (imAccount.isEnabled() && !StringUtils.isEmpty(imAccount.getImId())
                && !imAccount.getImId().equals(getUserName())) {
            AliasMapping mapping = new AliasMapping(imAccount.getImId());
            mappings.add(mapping);
        }

        // add fax extension aliases
        String faxExtension = getFaxExtension();
        String faxDid = getFaxDid();
        if (!faxExtension.isEmpty()) {
            String faxContactUri = SipUri.format(getDisplayName(), FAX_EXTENSION_PREFIX + getUserName(), domainName);
            AliasMapping mapping = new AliasMapping(faxExtension, faxContactUri);
            mappings.add(mapping);
            if (!faxDid.isEmpty()) {
                mapping = new AliasMapping(faxDid, faxContactUri);
                mappings.add(mapping);
            }
        }

        if (this.hasPermission(PermissionName.EXCHANGE_VOICEMAIL)
                || this.hasPermission(PermissionName.FREESWITH_VOICEMAIL)) {
            AliasMapping mapping = new AliasMapping("~~vm~");
            mappings.add(mapping);
        }

        Map<Replicable, Collection<AliasMapping>> aliases = new HashMap<Replicable, Collection<AliasMapping>>();
        aliases.put(this, mappings);
        return aliases;
    }
}
