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
import java.util.Map;

import com.mongodb.DBObject;

import org.sipfoundry.sipxconfig.admin.commserver.AliasProvider;
import org.sipfoundry.sipxconfig.common.Replicable;

public class Aliases extends DataSetGenerator {
    public static final String FAX_EXTENSION_PREFIX = "~~ff~";
    public static final String ALIASES = "als";
    private AliasProvider m_aliasProvider;

    public Aliases() {
    }

    protected DataSet getType() {
        return DataSet.ALIAS;
    }

    public void setAliasProvider(AliasProvider aliasProvider) {
        m_aliasProvider = aliasProvider;
    }

    @Override
    public void generate() {

        Map<Replicable, Collection<AliasMapping>> mappings = m_aliasProvider.getAliasMappings();
        for (Replicable entity : mappings.keySet()) {
            insertAliases(entity, mappings.get(entity));
        }
    }

    public void generate(Replicable entity) {
        insertAliases(entity, entity.getAliasMappings(getCoreContext().getDomainName()).get(entity));
    }

    private void insertAliases(Replicable entity, Collection<AliasMapping> mappings) {
        DBObject top = findOrCreate(entity);
        top.put(ALIASES, mappings);
        getDbCollection().save(top);
    }
}
