/*
 *
 *
 * Copyright (C) 2010 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 */
package org.sipfoundry.sipxconfig.common;

import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.commserver.imdb.DataSet;

/**
 * Bean With Internal user
 */
public class BeanWithUserPermissions extends BeanWithId implements Replicable {
    private InternalUser m_internalUser;

    public InternalUser getInternalUser() {
        return m_internalUser;
    }

    public void setInternalUser(InternalUser user) {
        m_internalUser = user;
    }

    @Override
    public String getName() {
        return null;
    }

    @Override
    public void setName(String name) {
    }

    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> ds = new HashSet<DataSet>();
        ds.add(DataSet.CREDENTIAL);
        ds.add(DataSet.PERMISSION);
        return ds;
    }

    @Override
    public String getIdentity(String domainName) {
        return getInternalUser().getUserName() + "@" + domainName;
    }

    @Override
    public Collection<AliasMapping> getAliasMappings(String domainName) {
        return null;
    }

    @Override
    public boolean isValidUser() {
        return false;
    }

    @Override
    public Map<String, Object> getMongoProperties(String domain) {
        return Collections.EMPTY_MAP;
    }

    @Override
    public String getEntityName() {
        return getClass().getSimpleName();
    }
}
