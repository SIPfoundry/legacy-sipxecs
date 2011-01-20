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

import org.sipfoundry.sipxconfig.admin.commserver.imdb.AliasMapping;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;

/**
 * Internal user, can be a TLS Peer
 */
public class InternalUser extends AbstractUser {

    @Override
    public Set<DataSet> getDataSets() {
        Set<DataSet> ds = new HashSet<DataSet>();
        ds.add(DataSet.PERMISSION);
        ds.add(DataSet.CREDENTIAL);
        return ds;
    }

    @Override
    public String getIdentity(String domain) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public Map<Replicable, Collection<AliasMapping>> getAliasMappings(String domain) {
        return Collections.EMPTY_MAP;
    }

}
