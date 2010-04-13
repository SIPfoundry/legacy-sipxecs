/*
 *
 *
 * Copyright (C) 2007 Pingtel Corp., certain elements licensed under a Contributor Agreement.
 * Contributors retain copyright to elements licensed under a Contributor Agreement.
 * Licensed to the User under the LGPL license.
 *
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;

public class Aliases extends DataSetGenerator {

    private AliasProvider m_aliasProvider;

    public Aliases() {
    }

    protected DataSet getType() {
        return DataSet.ALIAS;
    }

    protected void addItems(List<Map<String, String>> items) {
        addAliases(items, m_aliasProvider.getAliasMappings());
    }

    void addAliases(List<Map<String, String>> items, Collection<AliasMapping> aliases) {
        for (AliasMapping alias : aliases) {
            Map<String, String> aliasItem = addItem(items);
            aliasItem.put("identity", alias.getIdentity());
            aliasItem.put("contact", alias.getContact());
        }
    }

    public void setAliasProvider(AliasProvider aliasProvider) {
        m_aliasProvider = aliasProvider;
    }
}
