/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.domain;

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Random;
import java.util.Set;

import org.apache.commons.codec.binary.Base64;
import org.apache.commons.lang.StringUtils;
import org.sipfoundry.sipxconfig.cfgmgt.DeployConfigOnEdit;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.feature.Feature;
import org.sipfoundry.sipxconfig.systemaudit.ConfigChangeType;
import org.sipfoundry.sipxconfig.systemaudit.SystemAuditable;

/**
 * Single holder of domain name
 */
public class Domain extends BeanWithId implements DeployConfigOnEdit, Cloneable, SystemAuditable {
    private static final int SECRET_SIZE = 18;

    private String m_name;
    private String m_networkName;
    private Set<String> m_aliases;
    private String m_sharedSecret;

    private String m_sipRealm;

    public Domain() {
    }

    public Domain(String name) {
        setName(name);
    }

    public static Domain getDomain() {
        return DomainManagerImpl.getDomainInstance();
    }

    /**
     * Fully qualified host name is NOT using DNS SRV (e.g. myhost.example.com), otherwise use
     * domain name (example.com)
     */
    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
    }

    public String getSipRealm() {
        return m_sipRealm;
    }

    public void setSipRealm(String sipRealm) {
        m_sipRealm = sipRealm;
    }

    public boolean hasAliases() {
        return m_aliases != null && m_aliases.size() > 0;
    }

    /**
     * Editable clone object because domain object is shared everywhere and should be immutable
     */
    public Domain clone() throws CloneNotSupportedException {
        Domain copy = (Domain) super.clone();
        copy.m_aliases = new HashSet<String>(copy.m_aliases);
        return copy;
    }

    /**
     * Addresses (potentially invalid or inaccessable to sipx) that sipx commservers will accept
     * messages for and treat as actual domain name.
     */
    public Set<String> getAliases() {
        return m_aliases;
    }

    public void setAliases(Set<String> aliases) {
        m_aliases = aliases;
    }

    public void addAlias(String alias) {
        if (m_aliases == null) {
            m_aliases = new HashSet();
        }
        if (!StringUtils.equalsIgnoreCase(alias, m_name)) {
            m_aliases.add(alias);
        }
    }

    public void removeAlias(String alias) {
        if (m_aliases != null) {
            m_aliases.remove(alias);
        }
    }

    public String getSharedSecret() {
        return m_sharedSecret;
    }

    public void setSharedSecret(String sharedSecret) {
        m_sharedSecret = sharedSecret;
    }

    /**
     * Initialize domain secret if it's not initialized yet
     *
     * @return true if new secret has been initialized, false if secret was already there
     */
    protected boolean initSecret() {
        if (StringUtils.isNotBlank(m_sharedSecret)) {
            return false;
        }
        Random random = new Random(System.currentTimeMillis());
        byte[] secretBytes = new byte[SECRET_SIZE];
        random.nextBytes(secretBytes);
        m_sharedSecret = new String(new Base64().encode(secretBytes));
        return true;
    }

    @Override
    public Collection<Feature> getAffectedFeaturesOnChange() {
        return Collections.singleton((Feature) DomainManager.FEATURE);
    }

    public String getNetworkName() {
        return m_networkName;
    }

    public void setNetworkName(String networkName) {
        m_networkName = networkName;
    }

    @Override
    public String getEntityIdentifier() {
        return getName();
    }

    @Override
    public ConfigChangeType getConfigChangeType() {
        return ConfigChangeType.DOMAIN;
    }
}
