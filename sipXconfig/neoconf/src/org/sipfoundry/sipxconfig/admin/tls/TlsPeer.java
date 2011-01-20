/*
 *
 *
 * Copyright (C) 2009 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.tls;

import java.util.Collection;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.common.BeanWithUserPermissions;
import org.sipfoundry.sipxconfig.common.Replicable;

public class TlsPeer extends BeanWithUserPermissions implements Replicable {
    private String m_name;

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name;
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
        return "~~tp~" + getName() + "@" + domainName;
    }

    @Override
    public Map<Replicable, Collection<AliasMapping>> getAliasMappings(String domainName) {
        return null;
    }

}
