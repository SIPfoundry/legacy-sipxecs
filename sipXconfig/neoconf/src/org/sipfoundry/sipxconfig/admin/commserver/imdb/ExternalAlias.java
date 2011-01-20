/*
 *
 *
 * Copyright (C) 2011 eZuce inc., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.Collection;
import java.util.Map;
import java.util.Set;

import org.apache.commons.lang.builder.HashCodeBuilder;
import org.sipfoundry.sipxconfig.common.Replicable;

public class ExternalAlias implements Replicable {

    @Override
    public String getName() {
        return "extAls";
    }

    @Override
    public void setName(String name) {
        // TODO Auto-generated method stub

    }

    @Override
    public Map<Replicable, Collection<AliasMapping>> getAliasMappings(String domain) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public Set<DataSet> getDataSets() {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public String getIdentity(String domain) {
        // TODO Auto-generated method stub
        return null;
    }

    @Override
    public int hashCode() {
        return new HashCodeBuilder().append(getName()).toHashCode();
    }

    @Override
    public boolean equals(Object obj) {
        return this.getName().equals(((ExternalAlias) obj).getName());
    }

}
