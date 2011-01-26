/*
 *
 *
 * Copyright (C) 2010 Avaya, certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.authcode;

import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.common.BeanWithUserPermissions;
import org.sipfoundry.sipxconfig.common.Replicable;

public class AuthCode extends BeanWithUserPermissions implements Replicable {
    private String m_code;
    private String m_description;

    public String getCode() {
        return m_code;
    }

    public void setCode(String code) {
        m_code = code;
    }

    public String getDescription() {
        return m_description;
    }

    public void setDescription(String description) {
        m_description = description;
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
        return "~~ac~" + getId() + "@" + domainName;
    }

    @Override
    public Map<Replicable, Collection<AliasMapping>> getAliasMappings(String domainName) {
        return null;
    }

    @Override
    public String getName() {
        return null;
    }

    @Override
    public void setName(String name) {
    }

}
